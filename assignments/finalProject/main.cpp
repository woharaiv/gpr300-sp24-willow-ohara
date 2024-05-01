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

#include <willowLib/portals.h>

#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "ObjectTravel.h"
#include "PortalContact.h"

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();
void resetCamera(ew::Camera* camera, ew::CameraController* controller);
void drawAtPos(ew::Model*, glm::vec3, ew::Shader*);
void drawAtPos(ew::Mesh*, glm::vec3, ew::Shader*);
void runLightingPass(ew::Shader* displayShader, willowLib::DisplayPass* disp, willowLib::DeferredPass* def, ew::Camera* cam);
void runLightingPass(ew::Shader* displayShader, willowLib::DisplayPassToTexture* disp, willowLib::DeferredPass* def, ew::Camera* cam);
void ResolveCollisions();
//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

//Cameras
ew::Camera camera;
ew::CameraController cameraController;

const glm::vec3 BACKGROUND_COLOR = glm::vec3(0.6f, 0.8f, 0.92f);
//Base Material
struct Material {
	float Ka = 0;
	float Kd = 1;
	float Ks = 1;
	float Shininess = 128;
}material;

//Point Lights
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

//Main Light
float main_light_pos[3] = { 25.01f, 2.5f, -25.01f };

glm::mat4 lightSpaceMatrix;

//Deferred
willowLib::DeferredPass basePass;

//Display
willowLib::DisplayPass display;

//Shadow
willowLib::ShadowPass shadow;

ew::Transform planeTransform;

//Portals
willowLib::Portal bluePortal(glm::vec3(-2, 3, -5));
willowLib::Portal orangePortal(glm::vec3(-2, 3, -5));

//Portal FX
glm::vec2 directions = glm::vec2(1.0, 2.0);
float squash = 5.0;
float intensity = 0.02;
glm::vec3 blue = glm::vec3(0.0, 0.0, 0.9);
glm::vec3 orange = glm::vec3(1.0, 0.5, 0.0);

//Cube Location
glm::vec3 CubePos = glm::vec3(1.0, 0.5, 2.0);



//TODO: Change the size values for all of the contacts and the travel

//Portal contacts (portal transform, attachedPortalTransform, portalDimensions)
PortalContact bluePortalContact(glm::vec3(-1, 3, -5), glm::vec3(-3.5, 3, -2.5) ,glm::vec3(10, 10, 10)); //Temporary values
PortalContact orangePortalContact(glm::vec3(-3.5, 3, -2.5), glm::vec3(-1, 3, -5), glm::vec3(10, 10, 10)); //Temporary values

//Object Travel (objectPos, portalOnePos, portalTwoPos, objectDimensions)
ObjectTravel objectTravel(glm::vec3(0, 0, 0), glm::vec3(-1, 3, -5), glm::vec3(-3.5, 3, -2.5), glm::vec3(5, 5, 5)); //Temporary values

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
	ew::Shader portal_scene_shader = ew::Shader("assets/portalLightingPass.vert", "assets/portalLightingPass.frag");
	ew::Shader light_orb = ew::Shader("assets/lightOrb.vert", "assets/lightOrb.frag");
	ew::Shader portal_shader = ew::Shader("assets/portalShader.vert", "assets/portalShader.frag");

	GLuint marbleTexture = ew::loadTexture("assets/marble_color.jpg");
	GLuint marbleRoughness = ew::loadTexture("assets/marble_roughness.jpg");
	
	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");

	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");
	
	ew::Mesh planeMesh = ew::createPlane(64, 64, 8);
	planeTransform.position.y = -2;

	ew::Mesh cubeMesh = ew::createCube(1);
	ew::Transform cubeTransform;
	cubeTransform.position = CubePos;
	
	//Initialize portals
	ew::Mesh portalMesh = ew::createVerticalPlane(2, 4.5, 200);
	
	bluePortal.setYaw(3.14159f);
	bluePortal.linkedPortal = &orangePortal;
	orangePortal.linkedPortal = &bluePortal;

	ew::Mesh lightOrbMesh = ew::createSphere(1.0f, 8);
	ew::Mesh directionalLightMesh = ew::createCylinder(0.75, 2, 8);

	willowLib::createDeferredPass(&basePass, screenWidth, screenHeight);
	willowLib::createDisplayPass(&display);
	willowLib::createShadowPass(&shadow);

	willowLib::createDeferredPass(&bluePortal.portalPerspective);
	willowLib::createDisplayToTexturePass(&bluePortal.display);
	willowLib::createDeferredPass(&orangePortal.portalPerspective);
	willowLib::createDisplayToTexturePass(&orangePortal.display);

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
		lightSpaceMatrix = lightProjection * lightView;

		//Enable depth testing
		glEnable(GL_DEPTH_TEST);

		cameraController.move(window, &camera, deltaTime);

		ResolveCollisions();
		
		//=====SHADOW PASS=====
		shadow_shader.use();

		shadow_shader.setMat4("_LightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, willowLib::SHADOW_RESOLUTION, willowLib::SHADOW_RESOLUTION);
		glBindFramebuffer(GL_FRAMEBUFFER, shadow.fbo);
		//Clear framebuffer
		glClearColor(BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);
		shadow_shader.setMat4("_Model", planeTransform.modelMatrix());
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
		glBindFramebuffer(GL_FRAMEBUFFER, basePass.fbo);
		glClearColor(BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Bind marble texture to texture unit 0
		glBindTextureUnit(0, marbleTexture); //glBindTextureUnit() is a new function to OpenGL 4.5

		//Use deferred shader
		shader.use();

		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		shader.setInt("_MainTex", 0);
		//Draw Monkey
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				drawAtPos(&monkeyModel, glm::vec3(i*5, 0, j*-5), &shader);
			}
		}
		//Draw Ground
		glBindTextureUnit(0, marbleTexture);
		drawAtPos(&planeMesh, glm::vec3(20, -2, -20), &shader);
		drawAtPos(&cubeMesh, CubePos, &shader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//===Blue Portal Geometry Pass===
		glBindFramebuffer(GL_FRAMEBUFFER, bluePortal.portalPerspective.fbo);
		glClearColor(BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Bind marble texture to texture unit 0
		glBindTextureUnit(0, marbleTexture); //glBindTextureUnit() is a new function to OpenGL 4.5

		//Use deferred shader
		shader.use();

		shader.setMat4("_ViewProjection", bluePortal.portalCamera.projectionMatrix() * bluePortal.portalCamera.viewMatrix());
		shader.setInt("_MainTex", 0);
		//Draw Monkey
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				drawAtPos(&monkeyModel, glm::vec3(i * 5, 0, j * -5), &shader);
			}
		}
		

		//Draw Ground
		glBindTextureUnit(0, marbleTexture);
		drawAtPos(&planeMesh, glm::vec3(20, -2, -20), &shader);
		drawAtPos(&cubeMesh, CubePos, &shader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//===Orange Portal Geometry Pass===
		glBindFramebuffer(GL_FRAMEBUFFER, orangePortal.portalPerspective.fbo);
		glClearColor(BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Bind marble texture to texture unit 0
		glBindTextureUnit(0, marbleTexture); //glBindTextureUnit() is a new function to OpenGL 4.5

		//Use deferred shader
		shader.use();

		shader.setMat4("_ViewProjection", orangePortal.portalCamera.projectionMatrix() * orangePortal.portalCamera.viewMatrix());
		shader.setInt("_MainTex", 0);
		//Draw Monkey
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				drawAtPos(&monkeyModel, glm::vec3(i * 5, 0, j * -5), &shader);
			}
		}
		
		//Draw Ground
		glBindTextureUnit(0, marbleTexture);
		drawAtPos(&planeMesh, glm::vec3(20, -2, -20), &shader);
		drawAtPos(&cubeMesh, CubePos, &shader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//===Lighting Pass===
		runLightingPass(&portal_scene_shader, &bluePortal.display, &bluePortal.portalPerspective, &bluePortal.portalCamera);
		runLightingPass(&portal_scene_shader, &orangePortal.display, &orangePortal.portalPerspective, &orangePortal.portalCamera);
		runLightingPass(&display_shader, &display, &basePass, &camera);

		//===Light Orbs===
		glEnable(GL_DEPTH_TEST);
		//Copy lighting pass depth buffer to the current fbo
		glBindFramebuffer(GL_READ_FRAMEBUFFER, basePass.fbo);
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
		
		//===Draw Portals===
		portal_shader.use();
		portal_shader.setMat4("_ViewProjection", camera.projectionMatrix()* camera.viewMatrix());
		portal_shader.setFloat("time", time);
		portal_shader.setVec2("directions", directions);
		portal_shader.setFloat("squash", squash);
		portal_shader.setFloat("intensity", intensity);

		portal_shader.setVec3("color", blue);
		bluePortal.drawPortal(&portalMesh, &portal_shader, true);
		portal_shader.setVec3("color", orange);
		orangePortal.drawPortal(&portalMesh, &portal_shader, true);


		
		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

void runLightingPass(ew::Shader* displayShader, willowLib::DisplayPass* disp, willowLib::DeferredPass* def, ew::Camera* cam)
{
	glClearColor(BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(disp->vao);
	glBindTextureUnit(0, def->world_position);
	glBindTextureUnit(1, def->world_normal);
	glBindTextureUnit(2, def->albedo);
	glBindTextureUnit(3, shadow.map);

	//Use plain shader for final draw
	displayShader->use();

	//Maps from geometry pass
	displayShader->setInt("gPosition", 0);
	displayShader->setInt("gNormals", 1);
	displayShader->setInt("gAlbedo", 2);
	displayShader->setInt("shadowMap", 3);

	//Lighting variables
	displayShader->setVec3("viewPos", cam->position);
	displayShader->setVec3("ambientColor", BACKGROUND_COLOR / 2.0f); //Ambient color is realtive to background color, making it feel natural
	displayShader->setVec3("directionalLightPosition", glm::vec3(main_light_pos[0], main_light_pos[1], main_light_pos[2]));
	displayShader->setMat4("_LightSpaceMatrix", lightSpaceMatrix);

	displayShader->setFloat("material.Ka", material.Ka);
	displayShader->setFloat("material.Kd", material.Kd);
	displayShader->setFloat("material.Ks", material.Ks);
	displayShader->setFloat("material.Shininess", material.Shininess);
	for (int i = 0; i < MAX_POINT_LIGHTS; i++)
	{
		displayShader->setVec3(("pointLights[" + std::to_string(i) + "].position"), pointLights[i].position);
		displayShader->setVec4(("pointLights[" + std::to_string(i) + "].color"), pointLights[i].color);
		displayShader->setFloat(("pointLights[" + std::to_string(i) + "].radius"), pointLights[i].radius);
	}

	//Disable detph testing
	glDisable(GL_DEPTH_TEST);
	//Draw screen triangle
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
void runLightingPass(ew::Shader* displayShader, willowLib::DisplayPassToTexture* disp, willowLib::DeferredPass* def, ew::Camera* cam)
{
	glClearColor(BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(disp->vao);
	glBindTextureUnit(0, def->world_position);
	glBindTextureUnit(1, def->world_normal);
	glBindTextureUnit(2, def->albedo);
	glBindTextureUnit(3, shadow.map);
	glBindFramebuffer(GL_FRAMEBUFFER, disp->fbo);

	//Use plain shader for final draw
	displayShader->use();

	//Maps from geometry pass
	displayShader->setInt("gPosition", 0);
	displayShader->setInt("gNormals", 1);
	displayShader->setInt("gAlbedo", 2);
	displayShader->setInt("shadowMap", 3);

	//Lighting variables
	displayShader->setVec3("viewPos", cam->position);
	displayShader->setVec3("ambientColor", BACKGROUND_COLOR / 2.0f); //Ambient color is realtive to background color, making it feel natural
	displayShader->setVec3("directionalLightPosition", glm::vec3(main_light_pos[0], main_light_pos[1], main_light_pos[2]));
	displayShader->setMat4("_LightSpaceMatrix", lightSpaceMatrix);

	displayShader->setFloat("material.Ka", material.Ka);
	displayShader->setFloat("material.Kd", material.Kd);
	displayShader->setFloat("material.Ks", material.Ks);
	displayShader->setFloat("material.Shininess", material.Shininess);
	for (int i = 0; i < MAX_POINT_LIGHTS; i++)
	{
		displayShader->setVec3(("pointLights[" + std::to_string(i) + "].position"), pointLights[i].position);
		displayShader->setVec4(("pointLights[" + std::to_string(i) + "].color"), pointLights[i].color);
		displayShader->setFloat(("pointLights[" + std::to_string(i) + "].radius"), pointLights[i].radius);
	}

	//Disable detph testing
	glDisable(GL_DEPTH_TEST);
	//Draw screen triangle
	glDrawArrays(GL_TRIANGLES, 0, 6);
	//Unbind FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	if (ImGui::CollapsingHeader("Portal Positions")) {
		if (ImGui::Button("Reset Portals"))
		{
			bluePortal.transform.position = glm::vec3(5, 3, -5);
			orangePortal.transform.position = glm::vec3(-5, 3, -5);
		}
		ImGui::SliderFloat3("Blue Portal", glm::value_ptr(bluePortal.transform.position), -50, 50);
		ImGui::SliderFloat3("Orange Portal", glm::value_ptr(orangePortal.transform.position), -50, 50);
	}
	
	ImGui::End();

	ImGui::Begin("Maps");
	if (ImGui::CollapsingHeader("Base Camera Maps"))
	{
		ImVec2 windowSize = ImGui::GetWindowSize();
		windowSize.x /= 2;
		windowSize.y = windowSize.x;

		ImGui::Image((ImTextureID)basePass.world_position, windowSize, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::Image((ImTextureID)basePass.world_normal, windowSize, ImVec2(0, 1), ImVec2(1, 0));

		ImGui::Image((ImTextureID)basePass.albedo, windowSize, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::Image((ImTextureID)shadow.map, windowSize, ImVec2(0, 1), ImVec2(1, 0));
	}
	if (ImGui::CollapsingHeader("When looking through the Blue Portal, you'll see"))
	{
		ImVec2 windowSize = ImGui::GetWindowSize();
		windowSize.x /= 4;
		windowSize.y = windowSize.x;

		ImGui::Image((ImTextureID)bluePortal.portalPerspective.world_position, windowSize, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::Image((ImTextureID)bluePortal.portalPerspective.world_normal, windowSize, ImVec2(0, 1), ImVec2(1, 0));

		ImGui::Image((ImTextureID)bluePortal.portalPerspective.albedo, windowSize, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::Image((ImTextureID)bluePortal.display.scene, windowSize, ImVec2(0, 1), ImVec2(1, 0));
	}
	if (ImGui::CollapsingHeader("When looking through the Orange Portal, you'll see"))
	{
		ImVec2 windowSize = ImGui::GetWindowSize();
		windowSize.x /= 4;
		windowSize.y = windowSize.x;

		ImGui::Image((ImTextureID)orangePortal.portalPerspective.world_position, windowSize, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::Image((ImTextureID)orangePortal.portalPerspective.world_normal, windowSize, ImVec2(0, 1), ImVec2(1, 0));

		ImGui::Image((ImTextureID)orangePortal.portalPerspective.albedo, windowSize, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::Image((ImTextureID)orangePortal.display.scene, windowSize, ImVec2(0, 1), ImVec2(1, 0));
	}

	if (ImGui::CollapsingHeader("Portal Shader Effects")) {
		ImGui::SliderFloat3("Blue Portal Color", &blue.x, 0, 1);
		ImGui::SliderFloat3("Orange Portal Color", &orange.x, 0, 1);
		ImGui::SliderFloat2("Wobble Up and Down", &directions.x, 0, 20);
		ImGui::SliderFloat("How much wobble", &squash, 0.01, 10);
		ImGui::SliderFloat("How itense", &intensity, 0.01, 20);
		if (ImGui::Button("Reset Portal FX"))
		{
			directions = glm::vec2(1.0, 2.0);
			squash = 5.0;
			intensity = 0.02;
			blue = glm::vec3(0.0, 0.0, 0.9);
			orange = glm::vec3(1.0, 0.5, 0.0);
		}
	}

	ImGui::SliderFloat3("Cube Position", &CubePos.x, -10, 10);
	if (ImGui::Button("Reset Cube"))
	{
		CubePos = glm::vec3(1.0, 0.5, 2.0);
	}



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


void ResolveCollisions()
{
	bluePortal.updatePortalPerspective(&camera);
	orangePortal.updatePortalPerspective(&camera);

	bluePortalContact.CheckCollisions();
	orangePortalContact.CheckCollisions();

	bluePortalContact.HandleContacts();
	orangePortalContact.HandleContacts();
}

//
//void SetupPlayer()
//{
//	playerObject = new Particle(glm::vec3(0, 1, 0), 10);
//	//playerObject->GetPosition
//
//	collisionDetector = new ParticleContact;
//}