#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vertex_PCU.hpp"
#include "Game/Entity.hpp"
#include <vector>

//------------------------------------------------------------------------------------------------------------------------------
class Player : public Entity
{
public:
	explicit Player(Vec2 position, int playerID);
	~Player();

	void					Startup();

	void					Update(float deltaTime);
	void					Render() const;
	void					Shutdown();

	inline void				SetLife(float life) { m_life = life; }
	inline float			GetInitLife() {return m_initLife;}

	float					GetHalfWidth() const { return m_halfWidth; };
	float					GetHeightUp() const { return m_heightUp; }
	float					GetHeightDown() const { return m_heightDown; }

	void					SetActive(bool setValue) { m_isActive = setValue; }
	bool					IsActive() { return m_isActive; }

private:

public:
	std::vector<Vertex_PCU>		m_playerVerts;
	unsigned int				m_playerVertCount = 6;

private:
	float					m_acceleration = 50.f;
	float					m_life = 1.f;
	float					m_initLife = 100.f;

	int						m_playerID = 0;

	float					m_halfWidth = 0.25f;
	float					m_heightUp = 0.45f;
	float					m_heightDown = 0.45f;

	bool					m_isActive = false;
};