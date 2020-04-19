#include "Game/Player.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Commons/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"

//------------------------------------------------------------------------------------------------------------------------------
Player::Player(Vec2 position, int playerID)
	: Entity(position, ENTITY_PLAYER),
	m_playerID(playerID)
{

}

//------------------------------------------------------------------------------------------------------------------------------
Player::~Player()
{

}

//------------------------------------------------------------------------------------------------------------------------------
void Player::Startup()
{

}

//------------------------------------------------------------------------------------------------------------------------------
void Player::Update(float deltaTime)
{
	//Get Xbox controller data
	XboxController playerController = g_inputSystem->GetXboxController(m_playerID);
	AnalogJoyStick leftStick = playerController.GetLeftJoystick();
	AnalogJoyStick rightStick = playerController.GetRightJoystick();

	float rightTrigger = playerController.GetRightTrigger();
	float leftTrigger = playerController.GetLeftTrigger();

	m_direction = leftStick.GetPosition();
	m_position += m_direction * m_acceleration;
}

