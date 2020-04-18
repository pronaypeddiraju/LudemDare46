//Engine Systems
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/XMLUtils/XMLUtils.hpp"
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

	m_seed = (uint)GetCurrentTimeSeconds();
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
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::Render() const
{

}

//------------------------------------------------------------------------------------------------------------------------------
void Game::PostRender()
{

}

//------------------------------------------------------------------------------------------------------------------------------
void Game::Update(float deltaTime)
{
	
}

//------------------------------------------------------------------------------------------------------------------------------
bool Game::IsAlive()
{
	return m_isAlive;
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::LoadFonts()
{
	m_menuFont = g_renderContext->CreateOrGetBitmapFontFromFile("AppleIIFont", VARIABLE_WIDTH);
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

		node = node->NextSiblingElement("LevelNode");
		++problemIndex;
	}
}
