#pragma once

#include <ew/external/glad.h>
#include <ew/transform.h>

#include <willowLib/portals.h>
//#include "PortalContact.h"

class ObjectTravel
{
private:

	ew::Transform objectTransform;
	bool inPortalThreshold;
	ew::Transform portalOneTransform;
	ew::Transform portalTwoTransform;
	glm::vec3 previousObjectOffset;

	/*int objectWidth;
	int objectHeight;*/

	glm::vec3 objectDimensions; //width, height, length

public:

	ObjectTravel();
	ObjectTravel(ew::Transform objectTransform, ew::Transform portalOneTransform, ew::Transform portalTwoTransform, glm::vec3 objectDimensions);
	ObjectTravel( glm::vec3 objectPos, glm::vec3 portalOnePos, glm::vec3 portalTwoPos, glm::vec3 objectDimensions);

	void Teleport( glm::vec3 position, glm::quat quaternion);

	void EnterPortal();
	void ExitPortal();

	void UpdateObjectPosition(ew::Transform cameraPosition);

	#pragma region Getters&Setters

	ew::Transform GetObjectPosition();

	glm::vec3 GetPreviousObjectOffset();

	void SetPreviousObjectOffset(glm::vec3 newOffset);

	void SetPortalOneTransform(ew::Transform portalTransform);
	void SetPortalTwoTransform(ew::Transform portalTransform);

	ew::Transform GetPortalOneTransform();
	ew::Transform GetPortalTwoTransform();

	/*void SetObjectWidth(int newWidth);
	void SetObjectHeight(int newHeight);

	int GetObjectWidth();
	int GetObjectHeight();*/

	void SetObjectDimensions(glm::vec3 newDimensions);
	glm::vec3 GetObjectDimensions();

	#pragma endregion
	
};
