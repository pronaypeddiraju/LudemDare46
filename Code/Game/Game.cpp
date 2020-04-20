//Engine Systems
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/XMLUtils/XMLUtils.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Renderer/TextureView.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp" 
//Game Systems
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"

//------------------------------------------------------------------------------------------------------------------------------
Game::Game()
{
	m_isAlive = true;

	LoadFonts();

	g_devConsole->SetBitmapFont(*m_menuFont);
	g_debugRenderer->SetDebugFont(m_menuFont);

	m_seed = (uint)GetCurrentTimeHPC();
	g_RNG = new RandomNumberGenerator(m_seed);
}

//------------------------------------------------------------------------------------------------------------------------------
Game::~Game()
{
	
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::StartUp()
{
	g_windowContext->HideMouse();

	SetupCameras();
	LoadShaders();
	
	ReadLevelsXML();

	MakeMap(m_mapIndex);

	m_player = new Player(Vec2(1.5f, 1.5f), 0);
	m_player->Startup();

	m_gameInit = false;
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::HandleKeyPressed(unsigned char keyCode)
{
	UNUSED(keyCode);
	switch (keyCode)
	{
	break;
	case ENTER_KEY:
	{
		m_gameInit = true;
	}
	default:
		break;
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::HandleKeyReleased(unsigned char keyCode)
{
	UNUSED(keyCode);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::HandleCharacter(unsigned char charCode)
{
	UNUSED(charCode);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	delete m_mainCamera;
	m_mainCamera = nullptr;

	delete m_currentMapTexture;
	m_currentMapTexture = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::Render() const
{
	//Get the ColorTargetView from rendercontext
	ColorTargetView *colorTargetView = g_renderContext->GetFrameColorTarget();
	m_mainCamera->SetColorTarget(colorTargetView);
	m_mainCamera->SetViewport(Vec2::ZERO, Vec2::ONE);

	g_renderContext->BeginCamera(*m_mainCamera);
	g_renderContext->ClearColorTargets(m_clearScreenColor);

	if (!m_gameInit)
	{
		RenderGameInitScreen();
	}
	else
	{
		if (!m_obtainedLifeSource && !m_loadInitiated && !m_gameLost)
		{
			RenderGamePlay();
		}
		else
		{
			RenderGameState();
		}
	}

	//RenderDebugPlayerView();
	//RenderMapLoadSequence();
	//RenderPerfInfo();
	g_renderContext->EndCamera();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::PostRender()
{

}

//------------------------------------------------------------------------------------------------------------------------------
void Game::Update(float deltaTime)
{
	if (!m_gameInit)
		return;

	PerformFPSCachingAndCalculation(deltaTime);

	if (m_loadInitiated)
	{
		HandleLoadCase(deltaTime);
	}
	else
	{
		HandleGameLogic(deltaTime);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::SetPlayerLife()
{
	float initHealth = m_player->GetInitLife();
	float elapsedTime = m_currentMap->GetElapsedTime();
	float maxTime = m_currentMap->GetMapTime();

	float rangeFloat = RangeMapFloat(elapsedTime, 0, maxTime, 0, 1);

	m_player->SetLife( 1.f - rangeFloat);	//The reverse of how much time has elapsed is the life of the player
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::LoadNextMap()
{
	//reset information on game
	delete m_currentMap;
	m_currentMap = nullptr;

	delete m_currentMapTexture;
	m_currentMapTexture = nullptr;

	MakeMap(m_mapIndex);

	m_player->SetPosition(Vec2(1.5f, 1.5f));
	m_player->SetActive(false);

	m_obtainedLifeSource = false;
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::UpdateCollisions(const Map& map)
{
	if (m_player != nullptr)
	{
		PushEntityOutOfSolid(*m_player);

		bool result = HasObtainedLifeSource();
		if (result && !m_obtainedLifeSource)
		{
			m_obtainedLifeSource = result;
			m_collectedFragements++;
			m_currentMap->GetLifeSource().SetAttained(true);
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------------
bool Game::HasObtainedLifeSource()
{
	bool result = DoDiscsOverlap(m_player->GetPosition(), m_player->GetPhysicsRadius(), m_currentMap->GetLifeSource().GetPosition(), m_currentMap->GetLifeSource().GetPhysicsRadius());
	return result;
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::UpdatePlayerCamera()
{
	Vec2 playerPosition = m_player->GetPosition();

	Vec2 bottomLeftPos = playerPosition + Vec2(-WORLD_CENTER_Y * CLIENT_ASPECT, -WORLD_CENTER_Y);
	Vec2 topRightPos = playerPosition + Vec2(WORLD_CENTER_Y * CLIENT_ASPECT, WORLD_CENTER_Y);

	//Bottom Left position of camera
	Vec2 diffVector = Vec2(0.f, 0.f) - bottomLeftPos;

	Vec2 bottomLeftCamPos;
	bottomLeftCamPos.x = GetHigherValue(diffVector.x, 0.f);
	bottomLeftCamPos.y = GetHigherValue(diffVector.y, 0.f);
	diffVector = bottomLeftCamPos;

	bottomLeftPos += diffVector;
	topRightPos += diffVector;

	//Top right position of camera
	diffVector = Vec2(m_currentMap->GetMapDimensions()) - topRightPos;

	Vec2 topRightCamPos;
	topRightCamPos.x = GetLowerValue(diffVector.x, 0.f);
	topRightCamPos.y = GetLowerValue(diffVector.y, 0.f);
	diffVector = topRightCamPos;

	bottomLeftPos += diffVector;
	topRightPos += diffVector;

	m_currentCamBounds = AABB2(bottomLeftPos, topRightPos);

	m_mainCamera->SetOrthoView(bottomLeftPos, topRightPos);
}

//------------------------------------------------------------------------------------------------------------------------------
bool Game::IsAlive()
{
	return m_isAlive;
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::LoadFonts()
{
	m_menuFont = g_renderContext->CreateOrGetBitmapFontFromFile("SquirrelProportionalFont", PROPORTIONAL);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::SetupCameras()
{
	IntVec2 clientSize = g_windowContext->GetTrueClientBounds();
	m_windowAspect = g_windowContext->GetTrueClientAspect();

	//Create main camera
	m_mainCamera = new Camera();
	m_mainCamera->SetOrthoView(Vec2::ZERO, Vec2(WORLD_WIDTH, WORLD_HEIGHT));
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::LoadShaders()
{
	//Get the Shader
	m_shader = g_renderContext->CreateOrGetShaderFromFile(GAME_SHADER_LOAD_PATH);
	m_shader->SetDepth(eCompareOp::COMPARE_LEQUAL, true);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::ReadLevelsXML()
{
	//Open the xml file and parse it
	tinyxml2::XMLDocument levelDoc;
	levelDoc.LoadFile(GAME_LEVEL_LOAD_PATH.c_str());

	if (levelDoc.ErrorID() != tinyxml2::XML_SUCCESS)
	{

		ERROR_AND_DIE(">> Error loading Levels XML file ");
		return;
	}

	//We loaded the file successfully
	//Now let's read all the Overlapping problems
	XMLElement* root = levelDoc.RootElement();
	XMLElement* node = root->FirstChildElement("LevelNode");

	int problemIndex = 0;

	while (node != nullptr)
	{
		XMLElement* generatorNode = node->FirstChildElement("WFCGenerator");
		MapInfo info;
		
		int type = 0;
		type = ParseXmlAttribute(*generatorNode, "type", type);
		info.m_type = (eGeneratorType)type;

		info.m_mapDimensions.x = ParseXmlAttribute(*generatorNode, "width", info.m_mapDimensions.x);
		info.m_mapDimensions.y = ParseXmlAttribute(*generatorNode, "height", info.m_mapDimensions.y);
		info.m_kernelSize = ParseXmlAttribute(*generatorNode, "kernelSize", info.m_kernelSize);
		info.m_symmetry = ParseXmlAttribute(*generatorNode, "symmetry", info.m_symmetry);
		info.m_periodic = ParseXmlAttribute(*generatorNode, "periodic", info.m_periodic);
		info.m_periodicInput = ParseXmlAttribute(*generatorNode, "periodicInput", info.m_periodicInput);
		info.m_ground = ParseXmlAttribute(*generatorNode, "ground", info.m_ground);
		info.m_name = ParseXmlAttribute(*generatorNode, "name", info.m_name);

		generatorNode = node->FirstChildElement("MapMaker");
		info.m_pathColor = ParseXmlAttribute(*generatorNode, "pathColor", info.m_pathColor);
		info.m_boundaryColor = ParseXmlAttribute(*generatorNode, "boundaryColor", info.m_boundaryColor);
		info.m_portalColor = ParseXmlAttribute(*generatorNode, "portalColor", info.m_portalColor);

		generatorNode = node->FirstChildElement("LifeSource");
		info.m_lifeSourceColor = ParseXmlAttribute(*generatorNode, "lifeSourceColor", info.m_lifeSourceColor);
		info.m_duration = ParseXmlAttribute(*generatorNode, "duration", info.m_duration);

		generatorNode = node->FirstChildElement("MapStep");
		if (generatorNode != nullptr)
		{
			//We have a map step to perform
			MapStep mapStep;
			int type = ParseXmlAttribute(*generatorNode, "type", 0);
			mapStep.m_type = (eMapStepType)type;
			mapStep.m_num = ParseXmlAttribute(*generatorNode, "num", 0);
			mapStep.m_startColor = ParseXmlAttribute(*generatorNode, "colorToElim", mapStep.m_startColor);
			mapStep.m_destColor = ParseXmlAttribute(*generatorNode, "replaceColor", mapStep.m_destColor);
			info.m_mapStep.push_back(mapStep);
		}

		m_mapInfos.push_back(info);

		node = node->NextSiblingElement("LevelNode");
		++problemIndex;
	}

	m_numMaps = problemIndex;
	m_maxFragments = m_numMaps;
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::MakeMap(int mapIndex)
{
	m_currentMap = new Map(this, m_mapInfos[mapIndex]);
	m_currentMap->GenerateImage();
	m_currentMap->InitializeMap();

	Texture2D* texture = Texture2D::CreateTextureFromImage(g_renderContext, *m_currentMap->GetImage());
	m_currentMapTexture = texture->CreateTextureView();

	delete texture;
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::PushEntityOutOfSolid(Entity& entity)
{
	IntVec2 entityCoordinates = entity.GetPosition();

	//Check for North, South, East, West Tiles
	PushEntityOutOfTileIfSolid(entity, entityCoordinates, IntVec2(0, 1));
	PushEntityOutOfTileIfSolid(entity, entityCoordinates, IntVec2(0, -1));
	PushEntityOutOfTileIfSolid(entity, entityCoordinates, IntVec2(1, 0));
	PushEntityOutOfTileIfSolid(entity, entityCoordinates, IntVec2(-1, 0));

	//Check NE,NW,SE,SW Tiles
	PushEntityOutOfTileIfSolid(entity, entityCoordinates, IntVec2(1, 1));
	PushEntityOutOfTileIfSolid(entity, entityCoordinates, IntVec2(1, -1));
	PushEntityOutOfTileIfSolid(entity, entityCoordinates, IntVec2(-1, 1));
	PushEntityOutOfTileIfSolid(entity, entityCoordinates, IntVec2(-1, -1));
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::PushEntityOutOfTileIfSolid(Entity& entity, IntVec2 entityCoordinates, IntVec2 tileCoordinates)
{
	//First check if the tile is Solid
	int xCoord = entityCoordinates.x + tileCoordinates.x;
	int yCoord = entityCoordinates.y + tileCoordinates.y;

	if (xCoord < 0 || yCoord < 0 || xCoord == m_currentMap->GetMapDimensions().x || yCoord == m_currentMap->GetMapDimensions().y)
		return;

	int tileIndex = m_currentMap->GetIndexFromCoordinates(xCoord, yCoord);

	if (!m_currentMap->GetTileAtIndex(tileIndex).IsBlocking())
	{
		return;
	}

	//If we are here, the tile is blocking
	Vec2 mins = Vec2(entityCoordinates + tileCoordinates);
	AABB2 tileBounds = AABB2(mins, mins + Vec2::ONE);
	PushDiscOutOfAABB2(entity.GetPositionEditable(), entity.GetPhysicsRadius(), tileBounds);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::HandleLoadCase(float deltaTime)
{
	m_elapsedLoadTime += deltaTime;

	if (m_elapsedLoadTime > m_maxLoadTime)
	{
		m_loadInitiated = false;
		m_elapsedLoadTime = 0.f;
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::HandleGameLogic(float deltaTime)
{
	bool playerAlive = m_currentMap->Update(deltaTime);
	if (playerAlive)
	{
		SetPlayerLife();

		m_player->Update(deltaTime);
		UpdateCollisions(*m_currentMap);
		ResetPlayerIfInactive();
		UpdatePlayerCamera();
	}
	else
	{
		//Handle player death
		m_gameComplete = true;
		m_gameLost = true;
	}

	if (!m_currentMap->IsActive())
	{
		m_mapIndex++;
		if (m_mapIndex < m_numMaps)
		{
			m_loadInitiated = true;
			m_elapsedLoadTime = 0.f;
			LoadNextMap();
		}
		else
		{
			m_gameComplete = true;
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::PerformFPSCachingAndCalculation(float deltaTime)
{
	m_fpsCache[m_fpsCacheIndex] = m_fpsLastFrame = 1.f / deltaTime;

	if (m_fpsLastFrame > m_fpsHighest)
	{
		m_fpsHighest = m_fpsLastFrame;
	}

	if (m_fpsLastFrame < m_fpsLowest)
	{
		m_fpsLowest = m_fpsLastFrame;
	}

	m_deltaTime = deltaTime;

	m_fpsCacheIndex++;
	if (m_fpsCacheIndex == 1000)
	{
		m_fpsCacheIndex = 0;
	}

	int entriesCounted = 0;
	m_avgFPS = 0.f;
	for (int fpsIndex = 0; fpsIndex < 1000; fpsIndex++)
	{
		if (m_fpsCache[fpsIndex] == 0.f)
		{
			//This entry has not been filled yet
			continue;
		}

		m_avgFPS += m_fpsCache[fpsIndex];
		entriesCounted++;
	}

	m_avgFPS /= entriesCounted;
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderCurrentMap() const
{
	AABB2 imageBounds;
	imageBounds.m_minBounds = Vec2::ZERO;
	imageBounds.m_maxBounds = m_currentMap->GetImage()->GetImageDimensions();

	g_renderContext->BindTextureViewWithSampler(0U, m_currentMapTexture, SAMPLE_MODE_POINT);
	g_renderContext->BindShader(m_shader);

	std::vector<Vertex_PCU> box_verts;
	AddVertsForAABB2D(box_verts, imageBounds, Rgba::WHITE);
	g_renderContext->DrawVertexArray(box_verts);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderPlayer() const
{
	g_renderContext->BindTextureViewWithSampler(0U, nullptr);
	g_renderContext->BindShader(m_shader);

	m_player->Render();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderLifeSource() const
{
	g_renderContext->BindTextureViewWithSampler(0U, nullptr);
	g_renderContext->BindShader(m_shader);

	LifeSource& lifeSource = m_currentMap->GetLifeSource();
	lifeSource.Render();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderDebugTileView() const 
{
	IntVec2 startDimensions = m_currentMap->GetMapDimensions();
	std::vector<Vertex_PCU> box_verts;
	AABB2 imageBounds;

	g_renderContext->BindTextureViewWithSampler(0U, nullptr);
	g_renderContext->BindShader(m_shader);

	for (int tileIndex = 0; tileIndex < m_currentMap->GetTotalNumTiles(); tileIndex++)
	{
		Tile& tile = m_currentMap->GetTileAtIndex(tileIndex);
		if(!tile.IsBlocking())
			continue;

		IntVec2 tilePos = tile.GetTileCoordinates();

		imageBounds.m_minBounds = Vec2(tilePos);
		imageBounds.m_minBounds.x += (float)startDimensions.x;
		imageBounds.m_maxBounds = imageBounds.m_minBounds + Vec2::ONE;

		box_verts.clear();
		AddVertsForAABB2D(box_verts, imageBounds, tile.GetTileColor());
		g_renderContext->DrawVertexArray(box_verts);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderDebugPlayerView() const
{
	Vec2 playerPos = m_player->GetPosition();

	g_renderContext->BindTextureViewWithSampler(0U, nullptr);
	g_renderContext->BindShader(m_shader);

	std::vector<Vertex_PCU> ringVerts;
	AddVertsForRing2D(ringVerts, playerPos + Vec2((float)m_currentMap->GetMapDimensions().x, 0.f), m_player->GetPhysicsRadius(), 0.1f, Rgba::MAGENTA);
	g_renderContext->DrawVertexArray(ringVerts);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderPerfInfo() const
{
	g_renderContext->BindTextureViewWithSampler(0U, m_menuFont->GetTexture(), SAMPLE_MODE_POINT);
	g_renderContext->BindShader(m_shader);

	std::vector<Vertex_PCU> textVerts;
	m_menuFont->AddVertsForText2D(textVerts, Vec2(m_currentCamBounds.m_maxBounds.x - 10.f, m_currentCamBounds.m_maxBounds.y - 0.5f), 0.5f, Stringf("Avg FPS: %.3f", m_avgFPS), Rgba::ORGANIC_YELLOW);

	g_renderContext->DrawVertexArray(textVerts);
}

void Game::RenderLoadScreen() const
{
	//Render the load screen where we are going to start loading the next map
	g_renderContext->BindTextureViewWithSampler(0U, m_menuFont->GetTexture(), SAMPLE_MODE_POINT);
	g_renderContext->BindShader(m_shader);

	std::vector<Vertex_PCU> textVerts;
	Vec2 renderPosition = m_currentCamBounds.m_minBounds + Vec2(1.f, WORLD_CENTER_Y + 1.f);
	m_menuFont->AddVertsForText2D(textVerts, renderPosition, 0.25f, "Searching for some damn food...", Rgba::ORGANIC_YELLOW);

	g_renderContext->DrawVertexArray(textVerts);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderGameEndScreen() const
{
	//Render the load screen where we are going to start loading the next map
	g_renderContext->BindTextureViewWithSampler(0U, m_menuFont->GetTexture(), SAMPLE_MODE_POINT);
	g_renderContext->BindShader(m_shader);

	std::vector<Vertex_PCU> textVerts;
	Vec2 renderPosition = m_currentCamBounds.m_minBounds + Vec2(1.f, WORLD_CENTER_Y + 1.f);
	
	if (!m_gameLost)
	{
		m_menuFont->AddVertsForText2D(textVerts, renderPosition, m_HUDFontHeight, "You saved the tiger from Carole Baskin", Rgba::ORGANIC_GREEN);
		renderPosition.y -= 1.f;
		m_menuFont->AddVertsForText2D(textVerts, renderPosition, m_HUDFontHeight, "Your tiger can now eat for a week", Rgba::ORGANIC_YELLOW);
		renderPosition.y -= m_HUDFontHeight;
		m_menuFont->AddVertsForText2D(textVerts, renderPosition, m_HUDFontHeight, "You can always financially recover from this", Rgba::ORGANIC_YELLOW);
	}
	else
	{
		m_menuFont->AddVertsForText2D(textVerts, renderPosition, m_HUDFontHeight, "You couldn't feed your tiger", Rgba::ORGANIC_RED);
		renderPosition.y -= 1.f;
		m_menuFont->AddVertsForText2D(textVerts, renderPosition, m_HUDFontHeight, "Carole Baskin has your tiger, you now have a law suit", Rgba::ORGANIC_YELLOW);
		renderPosition.y -= m_HUDFontHeight;
		m_menuFont->AddVertsForText2D(textVerts, renderPosition, m_HUDFontHeight, "You will never financially recover from this", Rgba::ORGANIC_YELLOW);
	}

	g_renderContext->DrawVertexArray(textVerts);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderGameInitScreen() const
{
	//Render the load screen where we are going to start loading the next map
	g_renderContext->BindTextureViewWithSampler(0U, m_menuFont->GetTexture(), SAMPLE_MODE_POINT);
	g_renderContext->BindShader(m_shader);

	std::vector<Vertex_PCU> textVerts;
	Vec2 renderPosition = m_currentCamBounds.m_minBounds + Vec2(1.f, WORLD_CENTER_Y + 1.f);
	m_menuFont->AddVertsForText2D(textVerts, renderPosition, m_HUDFontHeight, "Carole Baskin is out to destroy you!", Rgba::ORGANIC_YELLOW);
	renderPosition.y -= 1.f;
	m_menuFont->AddVertsForText2D(textVerts, renderPosition, m_HUDFontHeight, "Find food so you can feed your tiger", Rgba::ORGANIC_GREEN);
	renderPosition.y -= m_HUDFontHeight;
	m_menuFont->AddVertsForText2D(textVerts, renderPosition, m_HUDFontHeight, "Collect green diamonds on the map", Rgba::ORGANIC_GREEN);

	renderPosition.y -= 1.f;
	m_menuFont->AddVertsForText2D(textVerts, renderPosition, m_HUDFontHeight, "You need an xBox controller to source your food", Rgba::WHITE);
	renderPosition.y -= m_HUDFontHeight;
	m_menuFont->AddVertsForText2D(textVerts, renderPosition, m_HUDFontHeight, "Press ENTER to continue", Rgba::WHITE);

	g_renderContext->DrawVertexArray(textVerts);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderGamePlay() const
{
	RenderCurrentMap();
	//RenderDebugTileView();

	RenderLifeSource();

	RenderPlayer();
	RenderTimeRemaining();
	RenderFragmentsCollected();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderGameState() const
{
	if (!m_gameComplete)
	{
		RenderLoadScreen();
	}
	else
	{
		//The game is completed
		RenderGameEndScreen();
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::ResetPlayerIfInactive()
{
	if (!m_player->IsActive())
	{
		m_player->SetActive(true);
		m_player->SetPosition(Vec2(1.5f, 1.5f));
		m_player->SetLife(1.f);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderTimeRemaining() const
{
	g_renderContext->BindTextureViewWithSampler(0U, m_menuFont->GetTexture(), SAMPLE_MODE_POINT);

	float elapsedTime = m_currentMap->GetElapsedTime();
	float maxTime = m_currentMap->GetMapTime();

	float remainingTime = maxTime - elapsedTime;

	if (remainingTime < 0.f)
	{
		remainingTime = 0.f;
	}

	std::vector<Vertex_PCU> textVerts;
	m_menuFont->AddVertsForText2D(textVerts, Vec2(m_currentCamBounds.m_minBounds.x, m_currentCamBounds.m_maxBounds.y - m_HUDFontHeight), m_HUDFontHeight, Stringf("Time Remaining: %.2f", remainingTime), Rgba::ORGANIC_YELLOW);

	g_renderContext->DrawVertexArray(textVerts);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderFragmentsCollected() const
{
	g_renderContext->BindTextureViewWithSampler(0U, m_menuFont->GetTexture(), SAMPLE_MODE_POINT);
	g_renderContext->BindShader(m_shader);

	std::vector<Vertex_PCU> textVerts;
	std::string printString = Stringf("Damn Food: %d/%d", m_collectedFragements, m_maxFragments);
	m_menuFont->AddVertsForText2D(textVerts, Vec2(m_currentCamBounds.m_minBounds.x, m_currentCamBounds.m_maxBounds.y - m_HUDFontHeight * 2.f), m_HUDFontHeight, printString, Rgba::ORGANIC_GREEN);


	g_renderContext->DrawVertexArray(textVerts);
}
