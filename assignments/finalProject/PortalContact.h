
#include <ew/external/glad.h>
#include <ew/transform.h>

#include <willowLib/portals.h>
#include "ObjectTravel.h"

class PortalContact
{
private:

	//void CheckCollisions();

	ObjectTravel objectTravel;
	ew::Transform portalTransform;
	ew::Transform attatchedPortalTransform;

	int GetSign(int num);

public:

	void HandleContacts();

	void SetObjectTravel(ObjectTravel objectTravel);
	ObjectTravel GetObjectTravel();

	void TravelerEnterPortal();
	void TravelerExitPortal();

};
