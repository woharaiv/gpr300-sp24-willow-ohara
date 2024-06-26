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
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

ew::Transform monkeyTransform;

struct Water {
	glm::vec3 color = glm::vec3(0.0f, 0.31f, 0.85f);
	float sample1Mod = 0.9f;
	float sample2Mod = 0.03f;
	float scale = 100;
	float waveStrength = 1.0f;
	float waveScale = 1.0f;
} water;

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
	
	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader waterShader = ew::Shader("assets/water.vert", "assets/water.frag");

	GLuint waterTexture = ew::loadTexture("assets/water.png");

	ew::Mesh waterPlane = ew::createPlane(128, 128, 64);
	monkeyTransform.position.y = -1;

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//RENDER
		glClearColor(BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cameraController.move(window, &camera, deltaTime);

		//Bind water texture to texture unit 0
		glBindTextureUnit(0, waterTexture); //glBindTextureUnit() is a new function to OpenGL 4.5

		//Use lit shader
		waterShader.use();

		waterShader.setVec3("_CameraPos", camera.position);
		waterShader.setVec3("_AmbientColor", BACKGROUND_COLOR/2.0f); //Ambient color is realtive to background color, making it feel natural

		//Set up shader to draw plane
		waterShader.setMat4("_Model", monkeyTransform.modelMatrix());
		waterShader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		waterShader.setInt("_MainTex", 0);

		waterShader.setFloat("_Time", time);
		waterShader.setFloat("_WaveStrength", water.waveStrength);
		waterShader.setFloat("_WaveScale", water.waveScale);
		waterShader.setVec3("_WaterColor", water.color);
		
		waterPlane.draw(); //Draw plane with current shader

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
	ImGui::DragFloat("Wave Scale", &water.waveScale, 0.01f, 0.0f);
	ImGui::DragFloat("Wave Strength", &water.waveStrength, 0.01f, 0.0f);
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

