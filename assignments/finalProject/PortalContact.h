
#include <ew/external/glad.h>
#include <ew/transform.h>

#include <willowLib/portals.h>
#include "ObjectTravel.h"

class PortalContact
{
private:

	ObjectTravel objectTravel;
	ew::Transform portalTransform;
	ew::Transform attatchedPortalTransform;

	bool isColliding;

	int GetSign(int num);

	bool checkForCollision();
	bool checkForNoCollision();

public:

	PortalContact();

	void CheckCollisions();

	void HandleContacts();

	void SetObjectTravel(ObjectTravel objectTravel);
	ObjectTravel GetObjectTravel();

	void TravelerEnterPortal();
	void TravelerExitPortal();

};
