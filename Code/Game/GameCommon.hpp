#pragma once
#include "Engine/Commons/EngineCommon.hpp"

constexpr float WORLD_WIDTH = 200.f;
constexpr float WORLD_HEIGHT = 100.f;
constexpr float WORLD_CENTER_X = WORLD_WIDTH / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_HEIGHT / 2.f;

constexpr float CAMERA_SHAKE_REDUCTION_PER_SECOND = 1.f;
constexpr float MAX_SHAKE = 2.0f;

constexpr float DEVCONSOLE_LINE_HEIGHT = 2.0f;

constexpr float CLIENT_ASPECT = 2.0f; // We are requesting a 2:1 aspect window area

class RenderContext;
class InputSystem;
class AudioSystem;

extern RenderContext* g_renderContext;
extern InputSystem* g_inputSystem;
extern AudioSystem* g_audio;

std::string GAME_LEVEL_LOAD_PATH = "Data/Gameplay/Levels.xml";
std::string GAME_SHADER_LOAD_PATH = "default_unlit.xml";