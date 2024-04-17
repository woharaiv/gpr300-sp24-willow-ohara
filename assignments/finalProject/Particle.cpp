
#include "Particle.h"

Particle::Particle()
{
	position = glm::vec3(0, 0, 0);
	velocity = glm::vec3(0, 0, 0);
	acceleration = glm::vec3(0, 0, 0);
	inverseMass = 1.0f;
	clearAccumulatedForces();
}

Particle::Particle(glm::vec3 initialPosition, float initialInverseMass)
{
	position = initialPosition;
	inverseMass = initialInverseMass;
	velocity = glm::vec3(0, 0, 0);
	acceleration = glm::vec3(0, 0, 0);
	clearAccumulatedForces();
}

Particle::~Particle()
{
	//clear any dynamic memory
	printf("deleting particle");
}

void Particle::UpdatePosition(float time)
{
	//position += velocity * time + acceleration * time * 0.5f;
	position = AddScaledVector(position, velocity, time);
	position = AddScaledVector(position, acceleration, time * time * 0.5f);

}

glm::vec3 Particle::AddScaledVector(const glm::vec3& vector1, const glm::vec3& vector2, float scale)
{
	glm::vec3 newVector = vector1;
	newVector += vector2.x * scale;
	newVector += vector2.y * scale;
	newVector += vector2.z * scale;
	return newVector;
}

void Particle::Integrate(float time)
{
	if (inverseMass <= 0.0f) //Do not integrate things with infinite mass
	{
		return;
	}

	assert(time > 0.0); //ensures that the time is greater than 0

	glm::vec3 newAcceleration = acceleration;
	newAcceleration = AddScaledVector(newAcceleration, accumulatedForces, inverseMass);

	velocity = AddScaledVector(velocity, newAcceleration, time);

	velocity *= powf(damping, time);

	//clears the accumulated forces every frame
	clearAccumulatedForces();
}

void Particle::clearAccumulatedForces()
{
	accumulatedForces = glm::vec3(0, 0, 0);
}

void Particle::AddForce(glm::vec3& force)
{
	accumulatedForces += force;
}


#pragma region Getters and setters

//Getters: ----------------------------------------

glm::vec3 Particle::GetVelocity()
{
	return velocity;
}

glm::vec3 Particle::GetPosition()
{
	return position;
}

glm::vec3 Particle::GetAcceleration()
{
	return acceleration;
}

glm::vec3 Particle::GetAccumulatedForces()
{
	return accumulatedForces;
}

float Particle::GetInverseMass()
{
	return inverseMass;
}

//Setters: ----------------------------------------

void Particle::SetAcceleration(glm::vec3 newAcceleration)
{
	acceleration = newAcceleration;
}

void Particle::SetInverseMass(float newInverseMass)
{
	inverseMass = newInverseMass;
}

void Particle::SetVelocity(glm::vec3 newVelocity)
{
	velocity = newVelocity;
}



#pragma endregion


