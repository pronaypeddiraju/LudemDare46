#pragma once
#include "Engine/Math/Vec2.hpp"

//------------------------------------------------------------------------------------------------------------------------------
enum eEntityType
{
	ENTITY_UNDEFINED = -1,

	ENTITY_PLAYER,
	ENTITY_ENEMY,
	
	NUM_ENTITY_TYPES
};

//------------------------------------------------------------------------------------------------------------------------------
class Entity
{
public:
	Entity(Vec2 position, eEntityType type);
	~Entity();

	inline const Vec2&		GetPosition() const { return m_position; }
	inline Vec2&			GetPositionEditable() { return m_position; }
	inline float			GetPhysicsRadius() { return m_physicsRadius; }

private:

public:
	Vec2			m_velocity = Vec2::ZERO;
	Vec2			m_direction = Vec2::ZERO;
	Vec2			m_position = Vec2::ZERO;

	bool			m_isAlive = true;
	bool			m_isOffScreen = false;

	eEntityType		m_entityType = ENTITY_UNDEFINED;

	float	m_physicsRadius = 0.25f;
};