#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Game/Game.hpp"

//------------------------------------------------------------------------------------------------------------------------------
class Game;

enum eGeneratorType
{
	OVERLAPPING,
	TILING
};

//------------------------------------------------------------------------------------------------------------------------------
// All info required to generate map
// For now we will set them to good defaults so if parsing is a problem we can use the values we already have
//------------------------------------------------------------------------------------------------------------------------------
struct MapInfo
{
	eGeneratorType			m_type = OVERLAPPING;
	IntVec2					m_mapDimensions = IntVec2(48, 48);
	int						m_kernelSize = 3;
	int						m_symmetry = 0;
	bool					m_periodic = false;
	bool					m_periodicInput = true;
	bool					m_ground = false;
};

//------------------------------------------------------------------------------------------------------------------------------
class Map
{
public:
	explicit Map(Game* game, MapInfo& info);
	~Map();

private:
	Game*					m_game = nullptr;
	MapInfo					m_info;

	int						m_totalBlocks = 0;

	int						m_TOTAL_TILE_VERTS = 0;
	int						m_defaultNumOutputImages = 2;
};