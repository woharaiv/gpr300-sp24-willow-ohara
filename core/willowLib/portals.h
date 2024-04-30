#pragma once
#include "../ew/transform.h"
#include "../ew/camera.h"
#include "../ew/procGen.h"
#include "../ew/shader.h"
#include "../ew/external/glad.h"
#include "passes.h"

namespace willowLib
{
	

	//Function from https://discussions.unity.com/t/converting-matrix4x4-to-quaternion-vector3/1438/2
	static glm::quat QuaternionFromMatrix(glm::mat4 m) {
		// Adapted from: http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm
		glm::quat q;
		q.w = sqrt(fmaxf(0, 1 + m[0][0] + m[1][1] + m[2][2])) / 2;
		q.x = sqrt(fmaxf(0, 1 + m[0][0] - m[1][1] - m[2][2])) / 2;
		q.y = sqrt(fmaxf(0, 1 - m[0][0] + m[1][1] - m[2][2])) / 2;
		q.z = sqrt(fmaxf(0, 1 - m[0][0] - m[1][1] + m[2][2])) / 2;
		q.x *= ((q.x * (m[2][1] - m[1][2])) >= 0 ? 1 : -1);
		q.y *= ((q.y * (m[0][2] - m[2][0])) >= 0 ? 1 : -1);
		q.z *= ((q.z * (m[1][0] - m[0][1])) >= 0 ? 1 : -1);
		return q;
	}

	class Portal
	{
	public:
		
		ew::Transform transform;
		//The yaw (rotation around up-down axis) of the portal, in radians.
		float yaw = 0;
		//The portal that this one connects to.
		Portal* linkedPortal = nullptr;
		//The camera that renders what is seen when looking through that portal.
		ew::Camera portalCamera;

		StencilPass portalStencil;
		DisplayPass display;
		DeferredPass portalPerspective;

		Portal(glm::vec3 startPos = glm::vec3(0));
		
		//Position based on the center of the portal rather than the the corner
		glm::vec3 centeredPosition() { return transform.position; }
		//Sets the portal's yaw (its rotation around the up-down axis).
		void setYaw(float radians);
		
		//Updates the portal's camera to properly show what that portal should display when viewed from the camera's current position.
		glm::vec3 updatePortalPerspective(ew::Camera* observer);

		void drawPortal(ew::Mesh* mesh, ew::Shader* shader, bool cullPortalSide = false);
	};
}