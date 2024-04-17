#include <imgui.h>
#include <stdio.h>
#include <math.h>
#include <ew/external/glad.h>
#include <glm/glm.hpp>

class Particle
{
public:

	Particle();
	Particle::Particle(glm::vec3 initialPosition, float initialInverseMass);
	~Particle();
	void UpdatePosition(float time);
	void Integrate(float time);
	void SetAcceleration(glm::vec3 newAcceleration);
	void SetInverseMass(float newInverseMass);


	glm::vec3 AddScaledVector(const glm::vec3& vector1, const glm::vec3& vector2, float scale);

	glm::vec3 GetPosition();
	glm::vec3 GetVelocity();
	glm::vec3 GetAcceleration();
	glm::vec3 GetAccumulatedForces();
	float GetInverseMass();
	void SetVelocity(glm::vec3 newVelocity);
	


private:

	glm::vec3 velocity;
	glm::vec3 position;
	glm::vec3 acceleration;
	glm::vec3 accumulatedForces; //this value is reset every integration
	float inverseMass;

	const float damping = 0.999f; //between 0 and 1. Values slightly below 1 are optimal

	void clearAccumulatedForces();
	void AddForce(glm::vec3& force);

};