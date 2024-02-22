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
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;

struct {
	GLuint fbo;
	GLuint world_position;
	GLuint world_normal;
	GLuint albedo;
	GLuint depth;
} deferred;

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

ew::Transform planeTransform;

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

	GLuint marbleTexture = ew::loadTexture("assets/marble_color.jpg");
	GLuint marbleRoughness = ew::loadTexture("assets/marble_roughness.jpg");

	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");

	ew::Mesh planeMesh = ew::createPlane(64, 64, 8);
	planeTransform.position.y = -2;

	create_deferred_pass();

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;


		//RENDER
		glBindFramebuffer(GL_FRAMEBUFFER, deferred.fbo);
		glClearColor(BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cameraController.move(window, &camera, deltaTime);

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

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
	if (ImGui::Button("Reset Camera"))
		resetCamera(&camera, &cameraController);
	if (ImGui::CollapsingHeader("Material")) {
		ImGui::SliderFloat("Ambient Coefficient", &material.Ka, 0.0f, 2.0f);
		ImGui::SliderFloat("Diffuse Coefficient", &material.Kd, 0.0f, 2.0f);
		ImGui::SliderFloat("Specular Coefficient", &material.Ks, 0.0f, 2.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
	}
	
	ImGui::Text("Rotate");
	ImGui::SameLine();
	// Arrow buttons with Repeater
	float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
	ImGui::PushButtonRepeat(true);
	if (ImGui::ArrowButton("##left", ImGuiDir_Left))
		planeTransform.rotation = glm::rotate(planeTransform.rotation, -5 * deltaTime, glm::vec3(0.0, 1.0, 0.0));
	ImGui::SameLine(0.0f, spacing);
	if (ImGui::ArrowButton("##right", ImGuiDir_Right))
		planeTransform.rotation = glm::rotate(planeTransform.rotation, 5 * deltaTime, glm::vec3(0.0, 1.0, 0.0));
	ImGui::PopButtonRepeat();
	ImGui::Separator();
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
	ImGui::Image((ImTextureID)deferred.depth, windowSize, ImVec2(0, 1), ImVec2(1, 0));
	
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

