#include "Game/Map.hpp"
//Engine
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/Time.hpp"
//Game
#include "Game/WFC/WFCOverlappingModel.hpp"
#include "Game/WFC/WFCImage.hpp"


//------------------------------------------------------------------------------------------------------------------------------
Map::Map(Game* game, MapInfo& info)
	: m_game(game),
	m_info(info)
{
	m_totalTiles = info.m_mapDimensions.x * info.m_mapDimensions.y;
	m_TOTAL_TILE_VERTS = m_totalTiles * 6;

	bool isWalkable = false;
	for (int i = 0; i < m_totalTiles; i++)
	{
		m_tiles.push_back(Tile(isWalkable, i, m_info.m_mapDimensions));
	}
}

//------------------------------------------------------------------------------------------------------------------------------
Map::~Map()
{
	delete m_mapImage;
	m_mapImage = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------
void Map::GenerateImage()
{
	GenerateOverlapping();
}

//------------------------------------------------------------------------------------------------------------------------------
void Map::InitializeMap()
{
	SetupTiles();

	SpawnLifeSource();
	SetMapDuration();
}

//------------------------------------------------------------------------------------------------------------------------------
void Map::SetupTiles()
{
	SetupMapRim();
	SetupStartArea();

	SetPathableTiles();
	
	PerformMapSteps();
	//Teleport to recieve a new life
	//SetupRandomTeleports();
}

//------------------------------------------------------------------------------------------------------------------------------
void Map::SetupMapRim()
{
	int i;
	int index = 0;
	//0,0  to 15,0 and 0,29 to 15,29
	for (i = 0; i < m_info.m_mapDimensions.x; i++)
	{
		index = GetIndexFromCoordinates(i, 0);
		m_tiles[index].SetTileType(TILE_TYPE_BLOCKING);
		m_tiles[index].SetTileColor(m_info.m_boundaryColor);
		m_mapImage->SetTexelColor(IntVec2(i, 0), m_info.m_boundaryColor);

		index = GetIndexFromCoordinates(i, m_info.m_mapDimensions.x - 1);
		m_tiles[index].SetTileType(TILE_TYPE_BLOCKING);
		m_tiles[index].SetTileColor(m_info.m_boundaryColor);
		m_mapImage->SetTexelColor(IntVec2(i, m_info.m_mapDimensions.x - 1), m_info.m_boundaryColor);
	}


	//0,0 to 0,29 and 15,0 to 15,29
	for (i = 0; i < m_info.m_mapDimensions.y; i++)
	{
		index = GetIndexFromCoordinates(0, i);
		m_tiles[index].SetTileType(TILE_TYPE_BLOCKING);
		m_tiles[index].SetTileColor(m_info.m_boundaryColor);
		m_mapImage->SetTexelColor(IntVec2(0, i), m_info.m_boundaryColor);

		index = GetIndexFromCoordinates(m_info.m_mapDimensions.y - 1, i);
		m_tiles[index].SetTileType(TILE_TYPE_BLOCKING);
		m_tiles[index].SetTileColor(m_info.m_boundaryColor);
		m_mapImage->SetTexelColor(IntVec2(m_info.m_mapDimensions.y - 1, i), m_info.m_boundaryColor);
	}

}

//------------------------------------------------------------------------------------------------------------------------------
void Map::SetupStartArea()
{
	int index;
	//Set all tiles to pathable between 1,1 to 5,5
	for (int i = 1; i <= 5; i++)
	{
		for (int j = 1; j <= 5; j++)
		{
			index = GetIndexFromCoordinates(i, j);
			m_tiles[index].SetTileType(TILE_TYPE_WALKABLE);
			m_tiles[index].SetTileColor(m_info.m_pathColor);
			m_mapImage->SetTexelColor(IntVec2(i, m_info.m_mapDimensions.y - 1 - j), m_info.m_pathColor);
		}
	}

	//Create the stone "bunker"
	//(4,2), (4,3), (4,4), (3,4), (2,4)
	index = GetIndexFromCoordinates(4, 2);
	m_tiles[index].SetTileType(TILE_TYPE_BLOCKING);
	m_tiles[index].SetTileColor(m_info.m_boundaryColor);
	m_mapImage->SetTexelColor(IntVec2(4, m_info.m_mapDimensions.y - 1 - 2), m_info.m_boundaryColor);

	index = GetIndexFromCoordinates(4, 3);
	m_tiles[index].SetTileType(TILE_TYPE_BLOCKING);
	m_tiles[index].SetTileColor(m_info.m_boundaryColor);
	m_mapImage->SetTexelColor(IntVec2(4, m_info.m_mapDimensions.y - 1 - 3), m_info.m_boundaryColor);

	index = GetIndexFromCoordinates(4, 4);
	m_tiles[index].SetTileType(TILE_TYPE_BLOCKING);
	m_tiles[index].SetTileColor(m_info.m_boundaryColor);
	m_mapImage->SetTexelColor(IntVec2(4, m_info.m_mapDimensions.y - 1 - 4), m_info.m_boundaryColor);

	index = GetIndexFromCoordinates(3, 4);
	m_tiles[index].SetTileType(TILE_TYPE_BLOCKING);
	m_tiles[index].SetTileColor(m_info.m_boundaryColor);
	m_mapImage->SetTexelColor(IntVec2(3, m_info.m_mapDimensions.y - 1 - 4), m_info.m_boundaryColor);

	index = GetIndexFromCoordinates(2, 4);
	m_tiles[index].SetTileType(TILE_TYPE_BLOCKING);
	m_tiles[index].SetTileColor(m_info.m_boundaryColor);
	m_mapImage->SetTexelColor(IntVec2(2, m_info.m_mapDimensions.y - 1 - 4), m_info.m_boundaryColor);
}

//------------------------------------------------------------------------------------------------------------------------------
int Map::GetIndexFromCoordinates(int x, int y)
{
	return x + (y * m_info.m_mapDimensions.x);
}

//------------------------------------------------------------------------------------------------------------------------------
int Map::GetTotalNumTiles()
{
	return m_totalTiles;
}

//------------------------------------------------------------------------------------------------------------------------------
const IntVec2& Map::GetMapDimensions() const
{
	return m_mapImage->GetImageDimensions();
}

//------------------------------------------------------------------------------------------------------------------------------
Tile& Map::GetTileAtIndex(int index)
{
	return m_tiles[index];
}

//------------------------------------------------------------------------------------------------------------------------------
Tile& Map::GetRandomTile()
{
	int randIndex = g_RNG->GetRandomIntInRange(0, m_totalTiles - 1);

	return m_tiles[randIndex];
}

//------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------
Image* Map::GetImage()
{
	return m_mapImage;
}

//------------------------------------------------------------------------------------------------------------------------------
bool Map::Update(float deltaTime)
{
	if (m_lifeSource.IsAttained())
		m_mapActive = false;

	if (m_mapActive)
	{
		m_elapsedTime += deltaTime;
	}

	if (m_elapsedTime > m_mapEndTime)
	{
		return false;
	}
	else
	{
		return true;
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Map::GenerateOverlapping()
{
	const std::string image_path = m_WFC_START_PATH + m_info.m_name + ".png";
	std::optional<Array2D<Color>> imageColorArray = ReadImage(image_path);

	if (!imageColorArray.has_value())
	{
		throw "Error while loading " + image_path;
	}

	OverlappingWFCOptions options = { m_info.m_periodicInput, m_info.m_periodic, (uint)m_info.m_mapDimensions.y, (uint)m_info.m_mapDimensions.x, (uint)m_info.m_symmetry, m_info.m_ground, (uint)m_info.m_kernelSize };

	bool createdImage = false;
	for (int test = 0; test < m_numMaxTest; test++)
	{
		int seed = g_RNG->GetRandomIntInRange(0, INT_MAX);
		OverlappingWFC overlappingWFC(*imageColorArray, options, seed);
		std::optional<Array2D<Color>> success = overlappingWFC.Run();

		if (success.has_value())
		{
			MakeMapImageFromColorArray(*success);
			//WriteImageAsPNG(outFolderPath + name + "_" + std::to_string(i) + ".png", *success);

			createdImage = true;
			break;
		}
	}

	if (!createdImage)
	{
		ERROR_AND_DIE("An image was not genereated");
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Map::MakeMapImageFromColorArray(const Array2D<Color>& success)
{
	m_mapImage = new Image(success.m_height, success.m_width);

	for (int xIndex = 0; xIndex < (int)success.m_width; xIndex++)
	{
		for (int yIndex = 0; yIndex < (int)success.m_height; yIndex++)
		{
			//I'm retarded so in this function xIndex (j in parameter) is along width and yIndex (i in paramter) along height
			Color color = success.Get(yIndex, xIndex);
			Rgba rbga = MakeRgbaFromColor(color);
			//Image needs y reversed because Array2D stores Y top to bottom order
			m_mapImage->SetTexelColor(IntVec2(xIndex, m_info.m_mapDimensions.y - 1 - yIndex), rbga);
			m_tiles[GetIndexFromCoordinates(xIndex, yIndex)].SetTileColor(rbga);

			if (rbga == m_info.m_pathColor)
			{
				m_tiles[GetIndexFromCoordinates(xIndex, yIndex)].SetTileType(TILE_TYPE_WALKABLE);
			}

		}
	}
}

//------------------------------------------------------------------------------------------------------------------------------
Rgba Map::MakeRgbaFromColor(Color color)
{
	Rgba rgba;
	rgba.SetFromBytes(color.r, color.g, color.b, 255);	
	return rgba;
}

//------------------------------------------------------------------------------------------------------------------------------
void Map::PerformMapSteps()
{
	std::vector<MapStep>::iterator itr;
	itr = m_info.m_mapStep.begin();

	while (itr != m_info.m_mapStep.end())
	{
		//perform map step
		RunStepForType(*itr);
		itr++;
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Map::RunStepForType(const MapStep& step)
{
	if (step.m_type == ELIMINATE)
	{
		int numReplaced = 0;
		int numToReplace = step.m_num;

		while (numReplaced < numToReplace)
		{
			bool isRequiredTile = false;
			while (!isRequiredTile)
			{
				Tile& tile = GetRandomTile();

				if (tile.GetTileColor() == step.m_startColor)
				{
					//we got our tile
					tile.SetTileColor(step.m_destColor);
					tile.SetTileType(TILE_TYPE_WALKABLE);

					//Replace color in the image too
 					IntVec2 tileCoords = tile.GetTileCoordinates();
					m_mapImage->SetTexelColor(IntVec2(tileCoords.x, m_mapImage->GetImageDimensions().y - 1 - tileCoords.y), step.m_destColor);

					numReplaced++;

					isRequiredTile = true;
				}
			}
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Map::SetPathableTiles()
{
	for (int tileIndex = 0; tileIndex < m_totalTiles; tileIndex++)
	{
		Tile& tile = GetTileAtIndex(tileIndex);

		if (tile.GetTileColor() != m_info.m_boundaryColor)
		{
			tile.SetTileType(TILE_TYPE_WALKABLE);
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Map::SetMapDuration()
{
	m_mapActive = true;
	m_mapEndTime = m_info.m_duration;
}

//------------------------------------------------------------------------------------------------------------------------------
void Map::SpawnLifeSource()
{
	//Spawn a life source item at a random pathable tile
	bool hasSpawned = false;

	while (!hasSpawned)
	{
		Tile& tile = GetRandomTile();

		//Check if tile is in required region
		IntVec2 tileCoords = tile.GetTileCoordinates();
		if (tileCoords.x > 5 && tileCoords.x < m_info.m_mapDimensions.x - 5 &&
			tileCoords.y > 5 && tileCoords.y < m_info.m_mapDimensions.y - 5)
		{
			//Check if tile is pathable
			if (!tile.IsBlocking())
			{
				//Make life source at this tile
				m_lifeSource.SetPosition(Vec2(tileCoords) + Vec2(0.5f, 0.5f));
				m_lifeSource.SetLifeSourceColor(m_info.m_lifeSourceColor);
				hasSpawned = true;
			}
		}
	}

}

