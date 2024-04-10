#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>
#include <ew/texture.h>
#include <ew/procGen.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();
void resetCamera(ew::Camera* camera, ew::CameraController* controller);
void drawAtPos(ew::Model*, glm::vec3, ew::Shader*);
void drawAtPos(ew::Mesh*, glm::vec3, ew::Shader*);

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

ew::Camera camera;
ew::CameraController cameraController;

const glm::vec3 BACKGROUND_COLOR = glm::vec3(0.6f, 0.8f, 0.92f);

struct Material {
	float Ka = 0;
	float Kd = 1;
	float Ks = 1;
	float Shininess = 128;
}material;

struct {
	GLuint fbo;
	GLuint world_position;
	GLuint world_normal;
	GLuint albedo;
	GLuint depth;
} deferred;

struct {
	GLuint vao;
	GLuint vbo;
}display;

struct PointLight {
	glm::vec3 position;
	float radius;
	glm::vec4 color;
};
const int MAX_POINT_LIGHTS = 256;
PointLight pointLights[MAX_POINT_LIGHTS];
void initialize_point_lights()
{
	float hoirzontalOffset = 50 / (sqrt(MAX_POINT_LIGHTS) - 1);
	int i = 0;
	srand(glfwGetTime() * 1000);
	for (int x = -1; x < sqrt(MAX_POINT_LIGHTS) - 1; x++)
	{
		for (int z = -1; z < sqrt(MAX_POINT_LIGHTS) - 1; z++)
		{
			pointLights[i].position = glm::vec3(hoirzontalOffset * x, -1, hoirzontalOffset * -z);
			pointLights[i].radius = (rand() % 46 + 5) / 10.0f; //Random Radius between 0.5 and 5
			pointLights[i].color = glm::vec4((rand() % 100 + 1) / 100.0f, (rand() % 100 + 1) / 100.0f, (rand() % 100 + 1) / 100.0f, 1.0f); //Random color
			i++;
		}
	}
}

void create_deferred_pass(void)
{
	//create framebuffer
	glCreateFramebuffers(1, &deferred.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, deferred.fbo);

	//generate world_position
	glGenTextures(1, &deferred.world_position);
	glBindTexture(GL_TEXTURE_2D, deferred.world_position);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//generate world_normal
	glGenTextures(1, &deferred.world_normal);
	glBindTexture(GL_TEXTURE_2D, deferred.world_normal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//generate albedo
	glGenTextures(1, &deferred.albedo);
	glBindTexture(GL_TEXTURE_2D, deferred.albedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//generate depth
	glGenTextures(1, &deferred.depth);
	glBindTexture(GL_TEXTURE_2D, deferred.depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	//Attach buffers to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, deferred.world_position, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, deferred.world_normal, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, deferred.albedo, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, deferred.depth, 0);
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
	}

	unsigned int buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, buffers);

	//Unbind the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void create_display_pass()
{
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
	//Set up VAO and VBO for screen quad
	glGenVertexArrays(1, &display.vao);
	glGenBuffers(1, &display.vbo);

	glBindVertexArray(display.vao);
	glBindBuffer(GL_ARRAY_BUFFER, display.vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), &quad, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0);
}

//Shadow struct
struct {
	GLuint rbo;
	GLuint fbo;
	GLuint map;
	GLuint vao;
	float minBias = 0.005;
	float maxBias = 0.03;
}shadow;

float main_light_pos[3] = { 25.01f, 2.5f, -25.01f };

const int SHADOW_RESOLUTION = 2048;

static void create_shadow_pass()
{
	//Shadow Framebuffer
	glGenFramebuffers(1, &shadow.fbo);
	//Shadow map
	glGenTextures(1, &shadow.map);

	//Bind to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadow.fbo);
	//Bind to shadow map
	glBindTexture(GL_TEXTURE_2D, shadow.map);
	//Initialize shadow map texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SHADOW_RESOLUTION, SHADOW_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	//Change buffer texture's filtering mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//Change buffer texture's wrapping mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f,1.0f,1.0f,1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	//Change buffer texture's compare mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

	//Attach depth buffer to FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow.map, 0);
	//Tell glCheckFramebufferStatus that we don't need a color buffer here
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
	}

	//Unbind the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ew::Transform monkeyTransform;

int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //Back-face culling
	glEnable(GL_DEPTH_TEST); //Depth testing

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); //Camera looks at center of scren
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f; //Field of view in degrees
	
	ew::Shader shader = ew::Shader("assets/geometryPass.vert", "assets/geometryPass.frag");
	ew::Shader shadow_shader = ew::Shader("assets/shadow.vert", "assets/shadow.frag");
	ew::Shader display_shader = ew::Shader("assets/lightingPass.vert", "assets/lightingPass.frag");
	ew::Shader light_orb = ew::Shader("assets/lightOrb.vert", "assets/lightOrb.frag");

	GLuint marbleTexture = ew::loadTexture("assets/marble_color.jpg");
	GLuint marbleRoughness = ew::loadTexture("assets/marble_roughness.jpg");

	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");
	ew::Mesh planeMesh = ew::createPlane(64, 64, 8);
	monkeyTransform.position.y = -2;

	ew::Mesh lightOrbMesh = ew::createSphere(1.0f, 8);
	ew::Mesh directionalLightMesh = ew::createCylinder(0.75, 2, 8);

	create_deferred_pass();
	create_display_pass();
	create_shadow_pass();

	initialize_point_lights();

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		float near_plane = 1.0f, far_plane = 7.5f;
		glm::mat4 lightProjection = glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, near_plane, far_plane);
		glm::mat4 lightView = glm::lookAt(
			glm::vec3(main_light_pos[0], main_light_pos[1], main_light_pos[2]),
			glm::vec3(25, 0, -25),
			glm::vec3(0, 1, 0)
		);
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		//Enable depth testing
		glEnable(GL_DEPTH_TEST);

		cameraController.move(window, &camera, deltaTime);
		
		//=====SHADOW PASS=====
		shadow_shader.use();

		shadow_shader.setMat4("_LightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
		glBindFramebuffer(GL_FRAMEBUFFER, shadow.fbo);
		//Clear framebuffer
		glClearColor(BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);
		shadow_shader.setMat4("_Model", monkeyTransform.modelMatrix());
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				drawAtPos(&monkeyModel, glm::vec3(i * 5, 0, j * -5), &shadow_shader);
			}
		}
		glCullFace(GL_BACK);
		//shadowShader.setMat4("_Model", planeTransform.modelMatrix());
		//planeMesh.draw();
		//Unbind
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//Reset viewport
		glViewport(0, 0, screenWidth, screenHeight);

		//===Geometry Pass===
		glBindFramebuffer(GL_FRAMEBUFFER, deferred.fbo);
		glClearColor(BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Bind marble texture to texture unit 0
		glBindTextureUnit(0, marbleTexture); //glBindTextureUnit() is a new function to OpenGL 4.5

		//Use deferred shader
		shader.use();

		//Set up shader to draw monkey
		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		shader.setInt("_MainTex", 0);
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				drawAtPos(&monkeyModel, glm::vec3(i*5, 0, j*-5), &shader);
			}
		}
		drawAtPos(&planeMesh, glm::vec3(20, -2, -20), &shader);



		//===Lighting Pass===
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindVertexArray(display.vao);
		glBindTextureUnit(0, deferred.world_position);
		glBindTextureUnit(1, deferred.world_normal);
		glBindTextureUnit(2, deferred.albedo);
		glBindTextureUnit(3, shadow.map);
		
		//Use plain shader for final draw
		display_shader.use();
		
		//Maps from geometry pass
		display_shader.setInt("gPosition", 0);
		display_shader.setInt("gNormals", 1);
		display_shader.setInt("gAlbedo", 2);
		display_shader.setInt("shadowMap", 3);
		
		//Lighting variables
		display_shader.setVec3("viewPos", camera.position);
		display_shader.setVec3("ambientColor", BACKGROUND_COLOR / 2.0f); //Ambient color is realtive to background color, making it feel natural
		display_shader.setVec3("directionalLightPosition", glm::vec3(main_light_pos[0], main_light_pos[1], main_light_pos[2]));
		display_shader.setMat4("_LightSpaceMatrix", lightSpaceMatrix);

		display_shader.setFloat("material.Ka", material.Ka);
		display_shader.setFloat("material.Kd", material.Kd);
		display_shader.setFloat("material.Ks", material.Ks);
		display_shader.setFloat("material.Shininess", material.Shininess);
		for (int i = 0; i < MAX_POINT_LIGHTS; i++)
		{
			display_shader.setVec3(("pointLights[" + std::to_string(i) + "].position"), pointLights[i].position);
			display_shader.setVec4(("pointLights[" + std::to_string(i) + "].color"), pointLights[i].color);
			display_shader.setFloat(("pointLights[" + std::to_string(i) + "].radius"), pointLights[i].radius);
		}

		//Disable detph testing
		glDisable(GL_DEPTH_TEST);
		//Draw screen triangle
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//===Light Orbs===
		glEnable(GL_DEPTH_TEST);
		//Copy lighting pass depth buffer to the current fbo
		glBindFramebuffer(GL_READ_FRAMEBUFFER, deferred.fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		light_orb.use();
		light_orb.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		for (int i = 0; i < MAX_POINT_LIGHTS; i++)
		{
			glm::mat4 m = glm::mat4(1.0f);
			m = glm::translate(m, pointLights[i].position);
			m = glm::scale(m, glm::vec3(0.2f));

			light_orb.setMat4("_Model", m);
			light_orb.setVec3("_Color", pointLights[i].color * (pointLights[i].radius / 4));
			lightOrbMesh.draw();
		}
		glm::mat4 m = glm::mat4(1.0f);
		m = glm::translate(m, glm::vec3(main_light_pos[0], main_light_pos[1] + 15, main_light_pos[2]));
		m = glm::scale(m, glm::vec3(2.5f));

		light_orb.setMat4("_Model", m);
		light_orb.setVec3("_Color", glm::vec3(0.0));
		directionalLightMesh.draw();

		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

void drawAtPos(ew::Model* model, glm::vec3 position, ew::Shader* shader)
{
	ew::Transform transform;
	transform.position = position;
	shader->setMat4("_Model", transform.modelMatrix());
	model->draw();
}
void drawAtPos(ew::Mesh* mesh, glm::vec3 position, ew::Shader* shader)
{
	ew::Transform transform;
	transform.position = position;
	shader->setMat4("_Model", transform.modelMatrix());
	mesh->draw();
}

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");
	ImGui::Text("%.1fms %.0fFPS | AVG: %.2fms %.1fFPS", ImGui::GetIO().DeltaTime * 1000, 1.0f / ImGui::GetIO().DeltaTime, 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	if (ImGui::Button("Reset Camera"))
		resetCamera(&camera, &cameraController);
	if (ImGui::CollapsingHeader("Material")) {
		ImGui::SliderFloat("Ambient Coefficient", &material.Ka, 0.0f, 2.0f);
		ImGui::SliderFloat("Diffuse Coefficient", &material.Kd, 0.0f, 2.0f);
		ImGui::SliderFloat("Specular Coefficient", &material.Ks, 0.0f, 2.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
	}
	
	ImGui::End();

	ImGui::Begin("Maps");
	ImVec2 windowSize = ImGui::GetWindowSize();
	windowSize.x /= 2;
	windowSize.y = windowSize.x;

	ImGui::Image((ImTextureID)deferred.world_position, windowSize, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::SameLine();
	ImGui::Image((ImTextureID)deferred.world_normal, windowSize, ImVec2(0, 1), ImVec2(1, 0));
	
	ImGui::Image((ImTextureID)deferred.albedo, windowSize, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::SameLine();
	ImGui::Image((ImTextureID)shadow.map, windowSize, ImVec2(0, 1), ImVec2(1, 0));
	
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
	camera.aspectRatio = (float)screenWidth / screenHeight;
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

