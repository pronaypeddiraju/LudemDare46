#pragma once
//Engine System
#include "Engine/Math/IntVec2.hpp"
//Game Systems
#include "Game/Game.hpp"
#include "Game/WFC/WFCArray2D.hpp"
#include "Game/WFC/WFCColor.hpp"
#include "Game/Tile.hpp"
#include "Game/LifeSource.hpp"

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
	std::string				m_name = "";
	int						m_kernelSize = 3;
	int						m_symmetry = 8;
	bool					m_periodic = false;
	bool					m_periodicInput = true;
	bool					m_ground = false;

	//We also want color information regarding tiles
	eTileType				m_defaultTileType = TILE_TYPE_BLOCKING;
	Rgba					m_pathColor = Rgba::CLEAR;
	Rgba					m_boundaryColor = Rgba::BLACK;
	Rgba					m_portalColor = Rgba::WHITE;
	Rgba					m_lifeSourceColor = Rgba::GREEN;

	//How long is this map to last?
	float					m_duration = 10.f;
};

//------------------------------------------------------------------------------------------------------------------------------
class Map
{
public:
	explicit Map(Game* game, MapInfo& info);
	~Map();

	void					GenerateImage();
	void					InitializeMap();

	Image*					GetImage();	//I want the pointer so I can edit it from Game if I want to

	bool					Update(float deltaTime);

	//Helpers
	int						GetIndexFromCoordinates(int x, int y);
	int						GetTotalNumTiles();
	const IntVec2&			GetMapDimensions() const;
	Tile&					GetTileAtIndex(int index);

	float					GetMapTime() const { return m_mapEndTime; }
	float					GetElapsedTime() const { return m_elapsedTime; }

	LifeSource&				GetLifeSource() { return m_lifeSource; }

private:
	//Internal Image generation methods
	void					GenerateOverlapping();

	void					SetupTiles();

	void					SetupMapRim();
	void					SetupStartArea();
	void					SetPathableTiles();

	void					SetMapDuration();
	void					SpawnLifeSource();

	Tile&					GetRandomTile();

	void					MakeMapImageFromColorArray(const Array2D<Color>& success);
	Rgba					MakeRgbaFromColor(Color color);
private:
	Game*					m_game = nullptr;
	MapInfo					m_info;

	bool					m_mapActive = false;

	std::vector<Tile>		m_tiles;
	Image*					m_mapImage = nullptr;

	int						m_totalTiles = 0;
	int						m_numMaxTest = 10;

	int						m_TOTAL_TILE_VERTS = 0;
	int						m_defaultNumOutputImages = 2;
	const std::string		m_WFC_START_PATH = "Data/Images/WFCInputSamples/";	

	float					m_mapEndTime = 0.f;

	LifeSource				m_lifeSource;

	float					m_elapsedTime = 0.f;
};