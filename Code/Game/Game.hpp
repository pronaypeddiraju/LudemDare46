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
private:
	bool								m_isAlive = false;
	float								m_windowAspect = 0.f;
	uint								m_seed = 0;

	Camera*								m_mainCamera = nullptr;
	Rgba								g_clearScreenColor = Rgba(0.f, 0.f, 0.f, 1.f);

	std::vector<MapInfo>				m_MapInfos;

public:
	BitmapFont*							m_menuFont = nullptr;

	Shader*								m_shader = nullptr;
};