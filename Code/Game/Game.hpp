//------------------------------------------------------------------------------------------------------------------------------
#pragma once
//Engine Systems
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Commons/EngineCommon.hpp"
//Game Systems
//Third Party

//------------------------------------------------------------------------------------------------------------------------------
struct MapInfo;
struct Camera;
class BitmapFont;
class Shader;
class Map;

//------------------------------------------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();

	void								StartUp();
	void								Shutdown();

	void								Update(float deltaTime);

	void								HandleKeyPressed( unsigned char keyCode );
	void								HandleKeyReleased( unsigned char keyCode );
	void								HandleCharacter( unsigned char charCode );

	void								Render() const;
	void								PostRender();
	
	bool								IsAlive();

private:
	void								LoadFonts();

	//Initializers
	void								SetupCameras();
	void								LoadShaders();
	void								ReadLevelsXML();

	//Map Handling
	void								MakeMap(int mapIndex);

	//Performance Monitor
	void								PerformFPSCachingAndCalculation(float deltaTime);

	//Render Functions
	void								RenderCurrentMap() const;
	void								RenderDebugTileView() const;
	void								RenderPerfInfo() const;
private:
	bool								m_isAlive = false;
	float								m_windowAspect = 0.f;
	uint								m_seed = 0;

	Camera*								m_mainCamera = nullptr;
	Rgba								m_clearScreenColor = Rgba(0.f, 0.f, 0.f, 1.f);

	std::vector<MapInfo>				m_mapInfos;
	Map*								m_currentMap = nullptr;
	TextureView*						m_currentMapTexture = nullptr;

	//Performance Data
	float								m_deltaTime = 0.f;
	float								m_fpsCache[1000] = { 0.f };
	float								m_fpsLastFrame = 0.f;
	float								m_avgFPS = 0.f;
	int									m_fpsCacheIndex = 0;
	int									m_numConnectedPlayers = 0;
	float								m_fpsLowest = 10.f;
	float								m_fpsHighest = 0.f;

public:
	BitmapFont*							m_menuFont = nullptr;
	Shader*								m_shader = nullptr;

	int									m_mapIndex = 0;
	int									m_numMaps = 0;
};