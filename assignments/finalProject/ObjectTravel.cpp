
#include "ObjectTravel.h"


ObjectTravel::ObjectTravel()
{
	inPortalThreshold = false;
	previousObjectOffset = glm::vec3(0, 0, 0);
}

ObjectTravel::ObjectTravel(ew::Transform objectTransform, ew::Transform portalOneTransform, ew::Transform portalTwoTransform, glm::vec3 objectDimensions)
{
	this->objectTransform = objectTransform;
	this->portalOneTransform = portalOneTransform;
	this->portalTwoTransform = portalTwoTransform;
	this->objectDimensions = objectDimensions;
	inPortalThreshold = false;
	previousObjectOffset = glm::vec3(0, 0, 0);
}
ObjectTravel::ObjectTravel(glm::vec3 objectPos, glm::vec3 portalOnePos, glm::vec3 portalTwoPos, glm::vec3 objectDimensions)
{
	ew::Transform newObjectTransform;
	newObjectTransform.position = objectPos;
	objectTransform = newObjectTransform;

	ew::Transform newPortalOneTransform;
	newPortalOneTransform.position = portalOnePos;
	portalOneTransform = newPortalOneTransform;

	ew::Transform newPortalTwoTransform;
	newPortalTwoTransform.position = portalTwoPos;
	portalTwoTransform = newPortalTwoTransform;

	this->objectDimensions = objectDimensions;

	inPortalThreshold = false;
	previousObjectOffset = glm::vec3(0, 0, 0);
}

//NOTE: Called when the player has gone completely through the portal
void ObjectTravel::Teleport(glm::vec3 position, glm::quat quaternion)
{
	objectTransform.position = position;
	objectTransform.rotation = quaternion;
}

//NOTE: Called whenever the object is colliding with the portal
void ObjectTravel::EnterPortal()
{
	inPortalThreshold = true;
}

//NOTE: Called whenever the object is no longer colliding with the portal
void ObjectTravel::ExitPortal()
{
	inPortalThreshold = false;
}

void ObjectTravel::UpdateObjectPosition(ew::Transform objectPosition)
{
	if (!inPortalThreshold)
	{
		//CheckCollisions();
		objectTransform = objectPosition;
		//CheckCollisions();
	}
	else
	{
		//TODO: more careful movement
	}
	
}

#pragma region Getters&Setters

ew::Transform ObjectTravel::GetObjectPosition()
{
	return objectTransform;
}

glm::vec3 ObjectTravel::GetPreviousObjectOffset()
{
	return previousObjectOffset;
}

void ObjectTravel::SetPreviousObjectOffset(glm::vec3 newOffset)
{
	previousObjectOffset = newOffset;
}

void ObjectTravel::SetPortalOneTransform(ew::Transform portalTransform)
{
	portalOneTransform = portalTransform;
}

ew::Transform ObjectTravel::GetPortalOneTransform()
{
	return portalOneTransform;
}

void ObjectTravel::SetPortalTwoTransform(ew::Transform portalTransform)
{
	portalTwoTransform = portalTransform;
}

ew::Transform ObjectTravel::GetPortalTwoTransform()
{
	return portalTwoTransform;
}

//void ObjectTravel::SetObjectWidth(int newWidth)
//{
//	objectWidth = newWidth;
//}
//
//void ObjectTravel::SetObjectHeight(int newHeight)
//{
//	objectHeight = newHeight;
//}
//
//int ObjectTravel::GetObjectWidth()
//{
//	return objectWidth;
//}
//
//int ObjectTravel::GetObjectHeight()
//{
//	return objectHeight;
//}

void ObjectTravel::SetObjectDimensions(glm::vec3 newDimensions)
{
	objectDimensions = newDimensions;
}

glm::vec3 ObjectTravel::GetObjectDimensions()
{
	return objectDimensions;
}

#pragma endregion
