#include "Particle.h"

const int MAXIMUM_PARTICLES = 10;


class ParticleContact
{
public:

private:

	Particle* particles[MAXIMUM_PARTICLES];

	float restitution;

	glm::vec3 contactDirection;

	float penetration;

	void ResolveVelocity(float time);

	void ResolveInterpenetration(float time);

protected:

	void Resolve(float time);

	glm::vec3 CalculateSeparatingVelocity() const;

};