
#include "ObjectTravel.h"


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
		CheckCollisions();
	}
	else
	{
		//TODO: more careful movement
	}
	
}

void ObjectTravel::CheckCollisions()
{
	/*
	Psuedo code:

	if (object collide with portal 1)
	{
		portalContact.TravelerEnterPortal();
	}
	*/
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

#pragma endregion
