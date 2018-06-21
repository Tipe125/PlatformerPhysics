#include "Player.h"
#include <dinput.h>
#include "GameData.h"

Player::Player(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF) : CMOGO(_fileName, _pd3dDevice, _EF)
{
	//any special set up for Player goes here
	m_fudge = Matrix::CreateRotationY(XM_PI);
	speedXMultiplier = 40;
	speedYMultiplier = 40;
	gravityMultiplier = 3;
	jumpMultiplier = 40;
	accelMultiplier = 50;
	frictionMultiplier = 40;
	decelMultiplier = 50;
	
	
	m_pos.y = 0.5f;
	speed = 0.0f;

	topSpeed = 6*speedXMultiplier;
	accel = 0.046875*accelMultiplier;
	decel = 0.5*decelMultiplier;
	friction = 0.046875*frictionMultiplier;
	isGrounded = true;
	hasJumped = false;
	onLadder = false;
	airAccel = 0.09375*accelMultiplier;
	speedY = 0.0f;
	grav = 0.21875*gravityMultiplier;
	jumpSpeed = 6.5*jumpMultiplier;

	

	
	
	SetPhysicsOn(true);
}

Player::~Player()
{
	//tidy up anything I've created
}


void Player::Tick(GameData* _GD)
{
	//CURRENT PROBLEMS: 
	

	CalculateFriction();
	CalculateTopSpeed();
	CalculateGravity();
	CalculateAccel();
	CalculateDecel();
	CalculateJumpSpeed();
	CalculateAirAccel();

	//checks whether player is on the ground 
	if (m_pos.y <= 0.5)//OR IF CHECKFORCOLLISION WITH EACH PLATFORM IS TRUE
	{
		isGrounded = true;
		hasJumped = false;
		speedY = 0.0f;
	}
	if (_GD->m_keyboardState[DIK_D] & 0x80)
	{
		//checks if player is pressing against their direction or forward
		if (speed < 0)
		{
			speed += decel;	
		}
		else if (speed < topSpeed)
		{
			if (!isGrounded)
			{
				speed += airAccel;
			}
			else
			{
				speed += accel;
			}
		}
	}
	else if (_GD->m_keyboardState[DIK_A] & 0x80)
	{
		if (speed > 0)
		{
			speed -= decel;
		}
		else if (speed > -topSpeed)
		{
			if (!isGrounded)
			{
				speed -= airAccel;
			}
			else
			{
				speed -= accel;
			}
		}
	}
	else
	{
		if (isGrounded)
		{
			if (speed < 0.125*speedXMultiplier && speed > -0.125 *speedXMultiplier && !onLadder )
			{
				speed = 0;
			}
			else
			{
				if (speed > 0)
				{
					speed -= friction;
				}
				else if (speed < 0)
				{
					speed += friction;
				}
				
			}
			
		}
	}
	//jump functionality 
	if (_GD->m_keyboardState[DIK_SPACE] & 0x80)
	{
		if (isGrounded)
		{
			speedY = jumpSpeed;
			isGrounded = false;
			printf("jumped\n");
			hasJumped = true;
		}
		else 
		{
			speedY -= grav;
			
				if (speedY <= -16 * jumpMultiplier)
				{
					speedY = -16 * jumpMultiplier;
				}
			
		}
	}
	else
	{
		if (hasJumped)
		{
			if (speedY > 4 * jumpMultiplier)
			{
				speedY = 4 * jumpMultiplier;
			}
		}
		if (!isGrounded)
		{
			
			speedY -= grav;
			if (speedY <= -16 * jumpMultiplier)
			{
				speedY = -16 * jumpMultiplier;
			}
		}
	}
	if (speedY > 0 && speedY < 4 * jumpMultiplier)
	{
		float airDrag = 0.96875;
		if (speed > 0.125 * speedXMultiplier)
		{
			speed = speed * airDrag;
		}
	}
	SetDrag(friction);

	if (_GD->m_keyboardState[DIK_W] & 0x80)
	{
		if (onLadder)
		{
			m_pos.y = m_pos.y + 0.1;
		}
	}

	if (_GD->m_keyboardState[DIK_S] & 0x80)
	{
		if (onLadder)
		{
			m_pos.y = m_pos.y - 0.1;
		}
	}
	//SetDragY(grav);
	//create the transform based on speed
	Vector3 forwardMove = speed * Vector3(1, 0, 0);
	upMove = speedY * Vector3(0, 1, 0);
	Matrix rotMove = Matrix::CreateRotationY(m_yaw);
	upMove = Vector3::Transform(upMove, rotMove);
	forwardMove = Vector3::Transform(forwardMove, rotMove);
	
	//make the player move based on speed calcs
	m_acc += forwardMove;
	m_acc += upMove;
	
	
	CMOGO::Tick(_GD);

	
	
	//limit motion of the player
	/*float length = m_pos.Length();
	float maxLength = 500.0f;
	if (length > maxLength)
	{
		m_pos.Normalize();
		m_pos *= maxLength;
		m_vel *= -0.9; //VERY simple bounce back
	}*/

	
	//apply my base behaviour
	
	
}