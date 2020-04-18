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
//Game Systems
#include "Game/Game.hpp"
#include "Game/Map.hpp"
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
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::HandleKeyPressed(unsigned char keyCode)
{
	UNUSED(keyCode);
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

	RenderCurrentMap();
	RenderDebugTileView();

	RenderPerfInfo();
	g_renderContext->EndCamera();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::PostRender()
{

}

//------------------------------------------------------------------------------------------------------------------------------
void Game::Update(float deltaTime)
{
	PerformFPSCachingAndCalculation(deltaTime);
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

	int problemIndex = 1;

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

		m_mapInfos.push_back(info);

		node = node->NextSiblingElement("LevelNode");
		++problemIndex;
	}

	m_numMaps = problemIndex;
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

	RenderDebugTileView();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderDebugTileView() const 
{
	IntVec2 startDimensions = m_currentMap->GetMapDimensions();
	std::vector<Vertex_PCU> box_verts;
	AABB2 imageBounds;

	for (int tileIndex = 0; tileIndex < m_currentMap->GetTotalNumTiles(); tileIndex++)
	{
		Tile& tile = m_currentMap->GetTileAtIndex(tileIndex);

		IntVec2 tilePos = tile.GetTileCoordinates();

		imageBounds.m_minBounds = Vec2(tilePos);
		imageBounds.m_minBounds.x += (float)startDimensions.x;
		imageBounds.m_maxBounds = imageBounds.m_minBounds + Vec2::ONE;

		g_renderContext->BindTextureViewWithSampler(0U, nullptr);
		g_renderContext->BindShader(m_shader);

		box_verts.clear();
		AddVertsForAABB2D(box_verts, imageBounds, tile.GetTileColor());
		g_renderContext->DrawVertexArray(box_verts);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderPerfInfo() const
{
	g_renderContext->BindTextureViewWithSampler(0U, m_menuFont->GetTexture(), SAMPLE_MODE_POINT);
	g_renderContext->BindShader(m_shader);

	std::vector<Vertex_PCU> textVerts;
	m_menuFont->AddVertsForText2D(textVerts, Vec2(0, WORLD_HEIGHT - 0.5f), 0.5f, Stringf("Avg FPS: %.3f", m_avgFPS));

	g_renderContext->DrawVertexArray(textVerts);
}
