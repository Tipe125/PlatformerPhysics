#include "gameobject.h"
#include "GameData.h"

GameObject::GameObject()
{
	//set the Gameobject to the origin with no rotation and unit scaling 
	m_pos = Vector3::Zero;
	m_pitch = 0.0f;
	m_yaw = 0.0f;
	m_roll = 0.0f;
	m_scale = Vector3::One;

	m_worldMat = Matrix::Identity;
	m_fudge = Matrix::Identity;
}

GameObject::~GameObject()
{

}

void GameObject::Tick(GameData* _GD)
{
	if (m_physicsOn)
	{
		float newVelX = m_vel.x + _GD->m_dt * (m_acc.x - m_drag *m_vel.x);
		//float newVelY = m_vel.y + _GD->m_dt * (m_acc.y - m_drag*m_vel.y);
		float newVelY = m_acc.y + _GD->m_dt;
		//could also do same for Z

		Vector3 newVel = Vector3(newVelX, newVelY, 0);
			//m_vel + _GD->m_dt * (m_acc - m_drag*m_vel);
		
		Vector3 newPos = m_pos + (_GD->m_dt * m_vel);
		
		
		m_vel = newVel;
		m_pos = newPos;
	}

	//build up the world matrix depending on the new position of the GameObject
	//the assumption is that this class will be inherited by the class that ACTUALLY changes this
	Matrix  scaleMat = Matrix::CreateScale(m_scale);
	m_rotMat = Matrix::CreateFromYawPitchRoll(m_yaw, m_pitch, m_roll); //possible not the best way of doing this!
	Matrix  transMat = Matrix::CreateTranslation(m_pos);

	m_worldMat = m_fudge * scaleMat * m_rotMat * transMat;

	//zero acceleration ready for the next time round
	
	m_acc = Vector3::Zero;
}

ObjectType GameObject::GetType()
{
	return m_objectType;
}

void GameObject::setIsGrounded(bool isItGrounded)
{
	isGrounded = isItGrounded;
}

void GameObject::setFrictionMultiplier(int friction)
{
	frictionMultiplier = friction;
}

void GameObject::setHasJumped(bool hasItJumped)
{
	hasJumped = hasItJumped;
}

void GameObject::setOnLadder(bool isOnLadder)
{
	onLadder = isOnLadder;
}

void GameObject::CalculateFriction()
{
	friction = 0.046875*frictionMultiplier;
}

void GameObject::CalculateTopSpeed()
{
	topSpeed = 6 * speedXMultiplier;
}

void GameObject::CalculateGravity()
{
	grav = 0.21875*gravityMultiplier;
}

void GameObject::CalculateAccel()
{
	accel = 0.046875*accelMultiplier;
}

void GameObject::CalculateDecel()
{
	decel = 0.5*decelMultiplier;
}

void GameObject::CalculateJumpSpeed()
{
	jumpSpeed = 6.5*jumpMultiplier;
}

void GameObject::CalculateAirAccel()
{
	airAccel = 0.09375*accelMultiplier;
}
