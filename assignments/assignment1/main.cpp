#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>
#include <ew/texture.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();
void resetCamera(ew::Camera* camera, ew::CameraController* controller);

ew::Camera camera;
ew::CameraController cameraController;

const glm::vec3 BACKGROUND_COLOR = glm::vec3(0.6f, 0.8f, 0.92f);

struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;


//Global state
glm::vec2 screenSize = { 1080, 720 };
float prevFrameTime;
float deltaTime;

ew::Transform monkeyTransform;

//Framebuffer
unsigned int pingpongFBO[2];
unsigned int pingpongColorBuffer[2];

//Screen Quad
float quad[] = {
  //-X- -Y-  -U- -V- 
	//Tri 1
	-1,  1,   0,  1,
	-1, -1,   0,  0,
	 1,  1,   1,  1,
	 
	 //Tri 2
	  1, -1,   1,  0,
	  1,  1,   1,  1,
	 -1, -1,   0,  0,
};
unsigned int screenVBO;
unsigned int screenVAO;

//Post-Processing effectSelection
const int NUM_POSTPROCESSING_EFFECTS = 2;
int numSelectedPostProcessingEffects = 0;
struct PostProcessingEffectSelection {
	ew::Shader* shader;
	bool enabled = false;
};
PostProcessingEffectSelection effectSelection[NUM_POSTPROCESSING_EFFECTS];
enum PostProcessingEffect
{
	VIGNETTE,
	INVERT,
};
float vignetteStrength = 1;


int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenSize.x, screenSize.y);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //Back-face culling
	glEnable(GL_DEPTH_TEST); //Depth testing

	//create framebuffer
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongColorBuffer);

	for (int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongColorBuffer[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, screenSize.x, screenSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorBuffer[i], 0);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
	}

	//Unbind this color buffer from the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); //Camera looks at center of scren
	camera.aspectRatio = (float)screenSize.x / screenSize.y;
	camera.fov = 60.0f; //Field of view in degrees

	//Set up VAO and VBO for screen quad
	glGenVertexArrays(1, &screenVAO);
	glGenBuffers(1, &screenVBO);
	
	glBindVertexArray(screenVAO);
	glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), &quad, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0);
	
	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader post_none = ew::Shader("assets/post_none.vert", "assets/post_none.frag");

	ew::Shader post_vignette = ew::Shader("assets/post_none.vert", "assets/post_vignette.frag");
	effectSelection[VIGNETTE] = {&post_vignette, false};

	ew::Shader post_invert = ew::Shader("assets/post_none.vert", "assets/post_invert.frag");
	effectSelection[INVERT] = { &post_invert, false };

	GLuint marbleTexture = ew::loadTexture("assets/marble_color.jpg");
	GLuint marbleRoughness = ew::loadTexture("assets/marble_roughness.jpg");

	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//RENDER
		
		//Bind color buffer
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongColorBuffer[0]);
		
		//Clear framebuffer
		glClearColor(BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cameraController.move(window, &camera, deltaTime);

		
		//Bind marble texture to texture unit 0
		glBindTextureUnit(0, marbleTexture); //glBindTextureUnit() is a new function to OpenGL 4.5
		//Bind marble roughness map to texture unit 1
		glBindTextureUnit(1, marbleRoughness);

		//Use lit shader
		shader.use();

		shader.setVec3("_CameraPos", camera.position);
		shader.setVec3("_AmbientColor", BACKGROUND_COLOR/2.0f); //Ambient color is realtive to background color, making it feel natural

		//Set up shader to draw monkey
		shader.setMat4("_Model", monkeyTransform.modelMatrix());
		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		shader.setInt("_MainTex", 0);
		shader.setInt("_RoughnessTex", 1);
		shader.setFloat("_Material.Ka", material.Ka);
		shader.setFloat("_Material.Kd", material.Kd);
		shader.setFloat("_Material.Ks", material.Ks);
		shader.setFloat("_Material.Shininess", material.Shininess);
		monkeyModel.draw(); //Draw monkey model with current shader
		
	  //=====POST-RPOCESSING=====
		int i = 0;
		int pass = 0;
		
		if (numSelectedPostProcessingEffects > 0)
		{
			//Create a loop that draws each selected effect to the buffer until the last one, which draws to the screen
			for each (PostProcessingEffectSelection effect in effectSelection)
			{
				if (effect.enabled)
				{
					effect.shader->use();
					
					//Draw framebuffer contents onto buffer opposite of last time
					glBindFramebuffer(GL_FRAMEBUFFER, pingpongColorBuffer[(pass + 1) % 2]);
					//Clear buffer for draw
					glClearColor(BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, 1.0);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					glBindVertexArray(screenVAO);
					glBindTexture(GL_TEXTURE_2D, pingpongColorBuffer[pass % 2]);
					
					//Set variables for shader
					switch (i)
					{
					case VIGNETTE:
						effect.shader->setVec2("screenSize", screenSize);
						effect.shader->setFloat("vignetteStrength", vignetteStrength);
						break;	
					}
					effect.shader->setInt("screenTexture", 0);
					glDrawArrays(GL_TRIANGLES, 0, 6);
					pass++;

					//Don't bother looping anymore once we know we've hit all the chosen effects
					if (pass >= numSelectedPostProcessingEffects)
						break;
				}
				i++;
			}
		}
		
	    //Unbind to draw to screen rather than buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//Clear buffer for final draw
		glClearColor(BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindVertexArray(screenVAO);
		glBindTexture(GL_TEXTURE_2D, pingpongColorBuffer[(pass) % 2]);
		post_none.use();
		post_none.setInt("screenTexture", 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		
	  //=====AFTER POST-PROCESSING=====
		drawUI();
		
		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Settings");

	if (ImGui::Button("Reset Camera"))
		resetCamera(&camera, &cameraController);
	
	if (ImGui::CollapsingHeader("Material")) {
		ImGui::SliderFloat("Ambient Coefficient", &material.Ka, 0.0f, 2.0f);
		ImGui::SliderFloat("Diffuse Coefficient", &material.Kd, 0.0f, 2.0f);
		ImGui::SliderFloat("Specular Coefficient", &material.Ks, 0.0f, 2.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
	}
	
	if (ImGui::CollapsingHeader("Post-Processing Effects"))
	{
		numSelectedPostProcessingEffects = 0;
		ImGui::Checkbox("Vignette", &effectSelection[VIGNETTE].enabled);
		if(effectSelection[VIGNETTE].enabled)
		{
			ImGui::SliderFloat("Vignette Strength", &vignetteStrength, 0.0f, 10);
			numSelectedPostProcessingEffects++;
		}
		ImGui::Checkbox("Invert Colors", &effectSelection[INVERT].enabled);
		if (effectSelection[INVERT].enabled)
		{
			numSelectedPostProcessingEffects++;
		}
	}
	ImGui::Text("Selected effects: %d", numSelectedPostProcessingEffects);
	
	ImGui::Text("Rotate");
	ImGui::SameLine();
	// Arrow buttons with Repeater
	float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
	ImGui::PushButtonRepeat(true);
	if (ImGui::ArrowButton("##left", ImGuiDir_Left))
		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, -5 * deltaTime, glm::vec3(0.0, 1.0, 0.0));
	ImGui::SameLine(0.0f, spacing);
	if (ImGui::ArrowButton("##right", ImGuiDir_Right))
		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, 5 * deltaTime, glm::vec3(0.0, 1.0, 0.0));
	ImGui::PopButtonRepeat();
	ImGui::Separator();
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenSize.x = width;
	screenSize.y = height;
	camera.aspectRatio = (float)screenSize.x / screenSize.y;
}

void resetCamera(ew::Camera* camera, ew::CameraController* controller) 
{
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}

