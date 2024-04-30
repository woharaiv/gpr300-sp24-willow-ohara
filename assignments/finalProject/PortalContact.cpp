
#include "PortalContact.h"

PortalContact::PortalContact()
{
	isColliding = false;
	portalDimensions = glm::vec3(1, 1, 1);
}

void PortalContact::CheckCollisions() //NOTE: This should be called int the update loop
{

	//if (isColliding)
	//{
	//	if (checkForNoCollision()) //if object is no longer colliding with portal
	//	{
	//		isColliding = false;
	//		TravelerExitPortal();
	//	}
	//}

	if (checkForCollision()) //if object collide with portal
	{
		isColliding = true;
		TravelerEnterPortal();
	}
	else
	{
		if (isColliding)
		{
			isColliding = false;
			TravelerExitPortal();
		}
	}

}

void PortalContact::HandleContacts() //NOTE: This should be called in the update loop
{
	glm::vec3 portalOffset = objectTravel.GetObjectPosition().position - portalTransform.position;

	glm::mat4 matrix = attatchedPortalTransform.modelMatrix() * portalTransform.modelMatrix() * objectTravel.GetObjectPosition().modelMatrix();

	int portalSide = GetSign(glm::dot(portalOffset, portalTransform.position)); //TODO: Change the portalTransform.position to the portals forward vector

	int portalSideOld = GetSign(glm::dot(objectTravel.GetPreviousObjectOffset(), portalTransform.position)); //TODO: Change the portalTransform.position to the portals forward vector

	if (portalSide != portalSideOld)
	{
		glm::vec3 oldPosition = objectTravel.GetObjectPosition().position;
		glm::quat oldRotation = objectTravel.GetObjectPosition().rotation;

		objectTravel.Teleport(matrix[3], objectTravel.GetObjectPosition().rotation);
	}
	else
	{
		objectTravel.SetPreviousObjectOffset(portalOffset);
	}
}

int PortalContact::GetSign(int num) //Helper function to get the sign of an integer
{
	if (num == 0)
	{
		return 0;
	}
	else if (num >= 0)
	{
		return 1;
	}
	else if (num < 0)
	{
		return -1;
	}
}

bool PortalContact::checkForCollision()
{
	/*int objectWidth = objectTravel.GetObjectWidth();
	int objectHeight = objectTravel.GetObjectHeight();*/

	glm::vec3 objectDimensions = objectTravel.GetObjectDimensions();

	glm::vec3 objectPosition = objectTravel.GetObjectPosition().position;

	glm::vec3 portalPosition = portalTransform.position;

	if (objectPosition.x < portalPosition.x + portalDimensions.x &&
		objectPosition.x + objectDimensions.x > portalPosition.x &&
		objectPosition.y < portalPosition.y + portalDimensions.y &&
		objectPosition.y + objectDimensions.y > portalPosition.y &&
		objectPosition.z < portalPosition.z + portalDimensions.z &&
		objectPosition.z + objectDimensions.z > portalPosition.z)
	{
		//Objects are colliding
		return 1;
	}
	else
	{
		return 0; 
	}
	
}

//bool PortalContact::checkForNoCollision()
//{
//	return 0; //TODO: Implement
//}

void PortalContact::TravelerEnterPortal() //TODO: Call this function when the cube collides with the portal
{
	objectTravel.EnterPortal();
	objectTravel.SetPreviousObjectOffset(objectTravel.GetObjectPosition().position - portalTransform.position);
}

void PortalContact::TravelerExitPortal() //TODO: Call this functionw when the cube is no longer colliding with the portal
{
	objectTravel.ExitPortal();
}

#pragma region Getters&Setters

void PortalContact::SetObjectTravel(ObjectTravel objectTravel)
{
	this->objectTravel = objectTravel;
}

ObjectTravel PortalContact::GetObjectTravel()
{
	return objectTravel;
}

void PortalContact::SetPortalDimensions(glm::vec3 newDimensions)
{
	portalDimensions = newDimensions;
}

glm::vec3 PortalContact::GetPortalDimensions()
{
	return portalDimensions;
}

//void PortalContact::SetPortalWidth(int newWidth)
//{
//	portalWidth = newWidth;
//}
//
//void PortalContact::SetPortalHeight(int newHeight)
//{
//	portalHeight = newHeight;
//}
//
//int PortalContact::GetPortalWidth()
//{
//	return portalWidth;
//}
//
//int PortalContact::GetPortalHeight()
//{
//	return portalHeight;
//}

#pragma endregion
