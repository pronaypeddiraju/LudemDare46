#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vertex_PCU.hpp"
#include "Engine/Math/AABB2.hpp"
#include <vector>

class TileDefenition;

enum eTileType
{
	TILE_TYPE_BLOCKING,
	TILE_TYPE_WALKABLE,
	NUM_TILE_TYPES
};

class Tile
{

public:
	Tile();
	~Tile();
	explicit Tile(bool isWalkable, unsigned int tileID, const IntVec2& mapDimensions);

	IntVec2					GetTileCoordinates();
	int						GetTileIndex();

	void			  		SetTileType(eTileType tileType);
	AABB2					GetTileDimensions() const;
	void					SetTileColor(const Rgba& color);
	const Rgba&				GetTileColor() const;

private:
	bool						m_isWalkable = false;	
	eTileType					m_tileType = TILE_TYPE_BLOCKING;

	AABB2						m_dimensions = AABB2(Vec2::ZERO, Vec2::ONE);
	unsigned int				m_tileID = 0;
	IntVec2						m_tileCoordinates = IntVec2(0, 0);	
	Rgba						m_tileColor = Rgba::CLEAR;
};
