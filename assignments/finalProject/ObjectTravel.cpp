
#include "ObjectTravel.h"


//NOTE: Called when the player has gone completely through the portal
void ObjectTravel::Teleport(glm::vec3 position, glm::quat quaternion)
{
	cameraTransform.position = position;
	cameraTransform.rotation = quaternion;
}

//NOTE: Called whenever the camera is within the portal threshold
void ObjectTravel::EnterPortal()
{

}

//NOTE: Called whenever the camera is no longer within the portal threshold
void ObjectTravel::ExitPortal()
{

}

void ObjectTravel::UpdateObjectPosition(ew::Transform objectPosition)
{
	if (!inPortalThreshold)
	{
		//CheckCollisions();
		cameraTransform = objectPosition;
	}
	else
	{
		//TODO: more careful movement
	}
	
}

ew::Transform ObjectTravel::GetObjectPosition()
{
	return objectTransform;
}

glm::vec3 ObjectTravel::GetPreviousObjectOffset()
{
	return previousCameraOffset;
}

void ObjectTravel::SetPreviousObjectOffset(glm::vec3 newOffset)
{
	previousCameraOffset = newOffset;
}

//void CameraTravel::SetPortalOneTransform(ew::Transform portalTransform)
//{
//	portalOneTransform = portalTransform;
//}
//
//ew::Transform CameraTravel::GetPortalOneTransform()
//{
//	return portalOneTransform;
//}
//
//void CameraTravel::SetPortalTwoTransform(ew::Transform portalTransform)
//{
//	portalTwoTransform = portalTransform;
//}
//
//ew::Transform CameraTravel::GetPortalTwoTransform()
//{
//	return portalTwoTransform;
//}
