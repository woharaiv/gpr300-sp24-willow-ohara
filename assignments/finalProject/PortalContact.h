#pragma once

#include <ew/external/glad.h>
#include <ew/transform.h>

#include <willowLib/portals.h>
#include "ObjectTravel.h"

class PortalContact
{
private:

	ObjectTravel objectTravel;
	ew::Transform portalTransform;
	ew::Transform attachedPortalTransform;

	bool isColliding;

	int GetSign(int num);

	/*int portalWidth;
	int portalHeight;*/

	glm::vec3 portalDimensions; //width, height, length

	bool checkForCollision();
	/*bool checkForNoCollision();*/

public:

	PortalContact();
	PortalContact(ew::Transform portalTransform, ew::Transform attachedPortalTransform, glm::vec3 portalDimensions);
	PortalContact(glm::vec3 portalPos, glm::vec3 attachedPortalPos, glm::vec3 portalDimensions);

	void CheckCollisions();

	void HandleContacts();

	void TravelerEnterPortal();
	void TravelerExitPortal();

	void SetObjectTravel(ObjectTravel objectTravel);
	ObjectTravel GetObjectTravel();

	void SetPortalDimensions(glm::vec3 newDimensions);
	glm::vec3 GetPortalDimensions();

	/*void SetPortalWidth(int newWidth);
	void SetPortalHeight(int newHeight);

	int GetPortalWidth();
	int GetPortalHeight();*/

};
