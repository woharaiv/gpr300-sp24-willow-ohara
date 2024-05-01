#include "portals.h"

namespace willowLib
{
	Portal::Portal(glm::vec3 startPos)
	{
		transform.position = startPos;
	}
	void Portal::setYaw(float radians)
	{
		transform.rotation = glm::rotate(transform.rotation, radians, glm::vec3(0, -1, 0));
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
		
		ew::Transform observerTransform, observerOffsetTransform;
		observerTransform.position = -observer->position;
		observerTransform.rotation = glm::quatLookAt(glm::normalize(observer->target - observer->position), glm::vec3(0, -1, 0));

		observerOffsetTransform.position = observer->target - observer->position;
		observerOffsetTransform.rotation = glm::quatLookAt(glm::normalize(observer->target - observer->position), glm::vec3(0, -1, 0));

		glm::mat4 m = (transform.modelMatrix() * linkedPortal->transform.modelMatrix() * observerTransform.modelMatrix());
		
		portalCamera.position = m[3];
		portalCamera.position.y *= -1.0f;
		portalCamera.position.y += 5.0f;

		portalCamera.target = portalCamera.position + observerOffsetTransform.position;

		return portalCamera.position;
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