#include "Game/Player.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Commons/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/VertexUtils.hpp"


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
	//AnalogJoyStick rightStick = playerController.GetRightJoystick();

	//float rightTrigger = playerController.GetRightTrigger();
	//float leftTrigger = playerController.GetLeftTrigger();

	m_direction = leftStick.GetPosition();
	m_position += m_direction * m_acceleration * deltaTime;
}

//------------------------------------------------------------------------------------------------------------------------------
void Player::Render() const
{
	//We want to render the player as a 0.5 across and 0.75 tall rect
	AABB2 imageBounds;
	imageBounds.m_minBounds = Vec2(m_position.x - m_halfWidth, m_position.y - m_heightDown);
	imageBounds.m_maxBounds = Vec2(m_position.x + m_halfWidth, m_position.y + m_heightUp);

	std::vector<Vertex_PCU> box_verts;
	AddVertsForAABB2D(box_verts, imageBounds, Rgba::ORGANIC_BLUE);
	g_renderContext->DrawVertexArray(box_verts);
}

//------------------------------------------------------------------------------------------------------------------------------
void Player::Shutdown()
{
	m_playerVerts.clear();
}

