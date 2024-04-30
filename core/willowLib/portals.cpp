#include "portals.h"

namespace willowLib
{
	Portal::Portal(glm::vec3 startPos)
	{
		transform.position = startPos;
	}
	void Portal::setYaw(float radians)
	{
		transform.rotation = glm::rotate(transform.rotation, radians, glm::vec3(0, 1, 0));
		yaw = radians;
	}
	glm::vec3 Portal::updatePortalPerspective(ew::Camera* observer)
	{
		//Don't run code if there is no other portal
		if (linkedPortal == nullptr)
		{
			std::printf("portal not linked");
			return glm::vec3(-1);
		}
		
		glm::mat4 m = (transform.modelMatrix() * linkedPortal->transform.modelMatrix() * observer->viewMatrix());
		
		//Place in portal cam at out portal position
		portalCamera.position = m[3];
		
		//Get target position from quaternion
		glm::quat q = QuaternionFromMatrix(m);
		glm::vec3 u(q.x, q.y, q.z);
		glm::vec3 v(0, 0, 1);
		glm::vec3 localLookAt = 2.0f * glm::dot(u, v) * u + (q.w * q.w - dot(u, u)) * v + 2.0f * q.w * cross(u, v);
		portalCamera.target = portalCamera.position + localLookAt;

		return portalCamera.position;
		// draw out portal to stencil buffer
		// draw scene from in portal camera without out potyal, using in stencil buffer to discard unnedded fragments
		// draw finished scene to texture
		// draw in portal, using its screen space uvs to map the portal camera's texture
		//
	}

	void Portal::drawPortal(ew::Mesh* mesh, ew::Shader* shader, bool cullPortalSide)
	{
		if (!cullPortalSide)
			glDisable(GL_CULL_FACE);
		shader->setMat4("_Model", transform.modelMatrix());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, display.scene);
		mesh->draw();
		if (!cullPortalSide)
			glEnable(GL_CULL_FACE);
	}
}