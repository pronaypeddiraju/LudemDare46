#include "Game/Map.hpp"

//------------------------------------------------------------------------------------------------------------------------------
Map::Map(Game* game, MapInfo& info)
	: m_game(game),
	m_info(info)
{
	m_totalBlocks = info.m_mapDimensions.x * info.m_mapDimensions.y;

	m_TOTAL_TILE_VERTS = m_totalBlocks * 6;
}

//------------------------------------------------------------------------------------------------------------------------------
Map::~Map()
{

}

