
#include "PortalContact.h"


void PortalContact::CheckCollisions()
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

void PortalContact::HandleContacts()
{
	CheckCollisions();
}

void PortalContact::SetObjectTravel(ObjectTravel objectTravel)
{
	this->objectTravel = objectTravel;
}

ObjectTravel PortalContact::GetObjectTravel()
{
	return objectTravel;
}

int PortalContact::GetSign(int num)
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