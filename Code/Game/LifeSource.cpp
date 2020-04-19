#include "Game/LifeSource.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//------------------------------------------------------------------------------------------------------------------------------
LifeSource::LifeSource()
	:	Entity(Vec2::ZERO, ENTITY_LIFESOURCE)
{

}

//------------------------------------------------------------------------------------------------------------------------------
LifeSource::~LifeSource()
{

}

//------------------------------------------------------------------------------------------------------------------------------
void LifeSource::Render()
{
	Vec2 top = m_position + Vec2(0, m_sizeXY.y);
	Vec2 bottom = m_position - Vec2(0, m_sizeXY.y);
	Vec2 left = m_position - Vec2(m_sizeXY.x, 0);
	Vec2 right = m_position + Vec2(m_sizeXY.x, 0);

	std::vector<Vertex_PCU> sourceVerts;

	AddVertsForTriangle2D(sourceVerts, top, left, bottom, m_color);
	AddVertsForTriangle2D(sourceVerts, bottom, right, top, m_color);

	g_renderContext->DrawVertexArray(sourceVerts);
}

