#pragma once
#include "Engine/Renderer/Rgba.hpp"
#include "Game/Entity.hpp"

//------------------------------------------------------------------------------------------------------------------------------
class LifeSource : public Entity
{
public:
	LifeSource();
	~LifeSource();

	bool		IsAttained() const { return m_attained; }
	void		SetAttained(bool setValue) { m_attained = setValue; }

	void		Render();

	void		SetLifeSourceColor(const Rgba& color) { m_color = color; }
private:
	bool	m_attained = false;
	Vec2	m_sizeXY = Vec2(0.25f, 0.45f);

	Rgba	m_color;
};