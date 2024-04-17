#include "ParticleContact.h"

void ParticleContact::ResolveVelocity(float time)
{
	glm::vec3 separatingVelocity = CalculateSeparatingVelocity();

	if (separatingVelocity.x > 0 || separatingVelocity.y > 0 || separatingVelocity.z > 0)
	{
		return;
	}

	glm::vec3 newSeparatingVelocity = -separatingVelocity * restitution;

	glm::vec3 deltaVelocity = newSeparatingVelocity - separatingVelocity;

	float totalInverseMass = particles[0]->GetInverseMass();
	if (particles[1])
	{
		totalInverseMass += particles[1]->GetInverseMass();
	}

	if (totalInverseMass <= 0) //if all particles have infinite mass
	{
		return;
	}

	glm::vec3 impulse = deltaVelocity / totalInverseMass;

	glm::vec3 impulsePerIMass = contactDirection * impulse;

	particles[0]->SetVelocity(particles[0]->GetVelocity() + impulsePerIMass * particles[0]->GetInverseMass()); //applies the impulses

	if (particles[1])
	{
		//particle one moves in the opposite direction of particle zero
		particles[1]->SetVelocity(particles[1]->GetVelocity() + impulsePerIMass * -particles[1]->GetInverseMass());
	}
}

void ParticleContact::Resolve(float time)
{
	ResolveVelocity(time);
	ResolveInterpenetration(time); 
}

glm::vec3 ParticleContact::CalculateSeparatingVelocity() const
{
	//find the velocity in the direction of the contact
	glm::vec3 relativeVelocity = particles[0]->GetVelocity();
	if (particles[1])
	{
		relativeVelocity -= particles[1]->GetVelocity();
	}
	return relativeVelocity * contactDirection;
}

void ParticleContact::ResolveInterpenetration(float time)
{
	if (penetration <= 0)
	{
		return;
	}

	float totalInverseMass = particles[0]->GetInverseMass();

	if (particles[1])
	{
		totalInverseMass += particles[1]->GetInverseMass();
	}

	if (totalInverseMass <= 0)
	{
		return;
	}

	glm::vec3 movePerIMass = contactDirection * (penetration / totalInverseMass);

	//particleMovement[0] = movePerIMass * particles[0]->GetInverseMass();
	if (particles[1])
	{
		//particleMovement[1] = movePerIMass * -particles[1]->GetInverseMass();
	}
	else
	{
		//particleMovement[1].clear();
	}

	//particles[0]->GetPosition(particles[0]->GetPosition() + particleMovement[0]);

	if (particles[1])
	{
		//particles[1]->GetPosition(particles[1]->GetPosition() + particleMovement[1]);
	}

	//TODO: left off in chapter 7.2.3

}
