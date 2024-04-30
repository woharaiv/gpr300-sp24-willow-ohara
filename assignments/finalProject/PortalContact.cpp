
#include "PortalContact.h"

PortalContact::PortalContact()
{
	isColliding = false;
}

void PortalContact::CheckCollisions() //NOTE: This should be called int the update loop
{

	if (isColliding)
	{
		if (checkForNoCollision()) //if object is no longer colliding with portal
		{
			isColliding = false;
			TravelerExitPortal();
		}
	}

	if (checkForCollision()) //if object collide with portal
	{
		isColliding = true;
		TravelerEnterPortal();
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
	return 0; //TODO: Implement



}
bool PortalContact::checkForNoCollision()
{
	return 0; //TODO: Implement
}

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

#pragma endregion
