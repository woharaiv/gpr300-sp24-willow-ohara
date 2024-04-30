/*
*	Author: Eric Winebrenner
*/

#include "cameraController.h"
namespace ew {
	void CameraController::move(GLFWwindow* window, ew::Camera* camera, float deltaTime) {
		//Only allow movement if right mouse is held
		if (!glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
			//Release cursor
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			firstMouse = true;
			return;
		}
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
		//MOUSE AIMING
		{
			double mouseX, mouseY;
			glfwGetCursorPos(window, &mouseX, &mouseY);

			//First frame, set prevMouse values
			if (firstMouse) {
				firstMouse = false;
				prevMouseX = mouseX;
				prevMouseY = mouseY;
			}

			float mouseDeltaX = (float)(mouseX - prevMouseX);
			float mouseDeltaY = (float)(mouseY - prevMouseY);

			prevMouseX = mouseX;
			prevMouseY = mouseY;

			//Change yaw and pitch (degrees)
			yaw += mouseDeltaX * mouseSensitivity;
			pitch -= mouseDeltaY * mouseSensitivity;
			pitch = glm::clamp(pitch, -89.0f, 89.0f);

		}
		//KEYBOARD MOVEMENT
		{
			float yawRad = glm::radians(yaw);
			float pitchRad = glm::radians(pitch);

			//Construct forward, right, and up vectors
			glm::vec3 forward;
			forward.x = cosf(pitchRad) * sinf(yawRad);
			forward.y = sinf(pitchRad);
			forward.z = cosf(pitchRad) * -cosf(yawRad);
			forward = glm::normalize(forward);

			glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
			glm::vec3 up = glm::normalize(glm::cross(right, forward));

			//Keyboard movement
			float speed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) ? sprintMoveSpeed : moveSpeed;
			float moveDelta = speed * deltaTime;
			if (glfwGetKey(window, GLFW_KEY_W)) {
				camera->position += forward * moveDelta;
			}
			if (glfwGetKey(window, GLFW_KEY_S)) {
				camera->position -= forward * moveDelta;
			}
			if (glfwGetKey(window, GLFW_KEY_D)) {
				camera->position += right * moveDelta;
			}
			if (glfwGetKey(window, GLFW_KEY_A)) {
				camera->position -= right * moveDelta;
			}
			if (glfwGetKey(window, GLFW_KEY_E)) {
				camera->position += up * moveDelta;
			}
			if (glfwGetKey(window, GLFW_KEY_Q)) {
				camera->position -= up * moveDelta;
			}

			//Camera will now look at a position along this forward axis
			camera->target = camera->position + forward;
		}
	}
}
