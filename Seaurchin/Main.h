#pragma once

#include <memory>

#include "Config.h"
#include "Setting.h"
#include "ExecutionManager.h"
#include "Debug.h"
#include "SceneDebug.h"
#include "Easing.h"
#include "Rendering.h"
#include "MoverFunctionExpression.h"
#include "ScriptSpriteMover.h"

#include <boost/chrono.hpp>
#include <SDL2/SDL.h>

void PreInitialize();
bool Initialize();
void Run();
void Terminate();