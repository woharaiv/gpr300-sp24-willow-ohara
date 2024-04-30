
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

public:

	void Teleport( glm::vec3 position, glm::quat quaternion);

	void EnterPortal();
	void ExitPortal();

	void UpdateObjectPosition(ew::Transform cameraPosition);

	ew::Transform GetObjectPosition();

	glm::vec3 GetPreviousObjectOffset();

	void SetPreviousObjectOffset(glm::vec3 newOffset);

	void SetPortalOneTransform(ew::Transform portalTransform);
	void SetPortalTwoTransform(ew::Transform portalTransform);
	
	ew::Transform GetPortalOneTransform();
	ew::Transform GetPortalTwoTransform();
};
