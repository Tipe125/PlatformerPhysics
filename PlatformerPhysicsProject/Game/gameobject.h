#ifndef _GAME_OBJECT_H
#define _GAME_OBJECT_H

//=================================================================
//Base Game Object Class
//=================================================================

#include "CommonStates.h"
#include "SimpleMath.h"
#include "ObjectType.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

class Camera;
struct ID3D11DeviceContext;
struct GameData;
struct DrawData;

class GameObject
{
public:
	GameObject();
	virtual ~GameObject();

	virtual void Tick(GameData* _GD);
	virtual void Draw(DrawData* _DD) = 0;

	//getters
	Vector3		GetPos() { return m_pos; }
	Vector3		GetScale() { return m_scale; }

	float		GetPitch() { return m_pitch; }
	float		GetYaw() { return m_yaw; }
	float		GetRoll() { return m_roll; }

	bool		IsPhysicsOn() { return m_physicsOn; }
	float		GetDrag() { return m_drag; }
	ObjectType GetType();
	float GetXSpeed() { return speed; }
	float GetYSpeed() { return speedY; }
	int GetFrictionMultiplier() { return frictionMultiplier; }
	float GetJumpSpeed() { return jumpSpeed; }

	//ANTTWEAKBARSTUFF
	int* twGetGravityMultiplier() { return &gravityMultiplier; }
	int* twGetFrictionMultiplier() { return &frictionMultiplier; }
	int* twGetAccelMultiplier() { return &accelMultiplier; }
	int* twGetDecelMultiplier() { return &decelMultiplier; }
	int* twGetJumpMultiplier() { return &jumpMultiplier; }
	int* twGetSpeedXMultiplier() { return &speedXMultiplier; }




	//setters
	void		setType(ObjectType type) { m_objectType = type; }
	void		setOnPlatform(bool isOnPlatform) { onPlatform = isOnPlatform; }
	void		setSpeedY(float newSpeedY) { speedY = newSpeedY; }
	void		setSpeed(float newSpeed) { speed = newSpeed; }
	void		setIsGrounded(bool isItGrounded);
	void		setFrictionMultiplier(int friction);
	void		setHasJumped(bool hasItJumped);
	void		setOnLadder(bool isOnLadder);
	
	void		SetPos(Vector3 _pos) { m_pos = _pos; }

	void		SetScale(float _scale) { m_scale = _scale * Vector3::One; }
	void		SetScale(float _x, float _y, float _z) { m_scale = Vector3(_x, _y, _z); }
	void		SetScale(Vector3 _scale) { m_scale = _scale; }

	void		SetPitch(float _pitch) { m_pitch = _pitch; }
	void		SetYaw(float _yaw) { m_yaw = _yaw; }
	void		SetRoll(float _roll) { m_roll = _roll; }
	void		SetPitchYawRoll(float _pitch, float _yaw, float _roll) { m_pitch = _pitch; m_yaw = _yaw; m_roll = _roll; }

	void		SetPhysicsOn(bool _physics) { m_physicsOn = _physics; }
	void		TogglePhysics() { m_physicsOn = !m_physicsOn; }
	void		SetDrag(float _drag) { m_drag = _drag; }
	void		SetDragY(float _dragY) { m_dragY = _dragY; }

	void		SetHeight(int newHeight) { height = newHeight; }
	void		SetWidth(int newWidth) { width = newWidth; }

	//calculations
	void CalculateFriction();
	void CalculateTopSpeed();
	void CalculateGravity();
	void CalculateAccel();
	void CalculateDecel();
	void CalculateJumpSpeed();
	void CalculateAirAccel();


protected:

	//World transform/matrix of this GO and it components
	Matrix m_worldMat;
	Matrix m_rotMat;
	Matrix m_fudge;
	Vector3 m_pos;
	float m_pitch, m_yaw, m_roll;
	Vector3 m_scale;

	
	//very basic physics
	bool m_physicsOn = false;
	float m_drag = 0.0f;
	float m_dragY = 0.0f;
	Vector3 m_vel = Vector3::Zero;
	Vector3 m_acc = Vector3::Zero;
	ObjectType m_objectType = GAMEOBJECT;
	bool onPlatform;

	bool isGrounded;
	bool hasJumped;
	bool onLadder;
	float speedY;
	float speed;
	float topSpeed;
	float accel;
	float decel;
	float friction;
	float airAccel;
	float grav;
	float jumpSpeed;
	int height;
	int width;
	Vector3 upMove;
	

	//multipliers
	int speedXMultiplier;
	int speedYMultiplier;
	int gravityMultiplier;
	int jumpMultiplier;
	int accelMultiplier;
	int frictionMultiplier;
	int decelMultiplier;
};

#endif