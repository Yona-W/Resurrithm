set(PROJECT_NAME Seaurchin)

################################################################################
# Source groups
################################################################################
set(AngelScript
    "../library/angelscript/add_on/scriptarray/scriptarray.cpp"
    "../library/angelscript/add_on/scriptarray/scriptarray.h"
    "../library/angelscript/add_on/scriptdictionary/scriptdictionary.cpp"
    "../library/angelscript/add_on/scriptdictionary/scriptdictionary.h"
    "../library/angelscript/add_on/scriptmath/scriptmath.cpp"
    "../library/angelscript/add_on/scriptmath/scriptmath.h"
    "../library/angelscript/add_on/scriptmath/scriptmathcomplex.cpp"
    "../library/angelscript/add_on/scriptmath/scriptmathcomplex.h"
    "../library/angelscript/add_on/scriptstdstring/scriptstdstring.cpp"
    "../library/angelscript/add_on/scriptstdstring/scriptstdstring.h"
    "../library/angelscript/add_on/scriptstdstring/scriptstdstring_utils.cpp"
)
source_group("AngelScript" FILES ${AngelScript})

set(Interface_AngelScript
    "AngelScriptManager.cpp"
    "AngelScriptManager.h"
    "Interfaces.cpp"
    "Interfaces.h"
    "ScriptFunction.cpp"
    "ScriptFunction.h"
    "SkinHolder.cpp"
    "SkinHolder.h"
    #"wscriptbuilder.cpp"
    #"wscriptbuilder.h"
)
source_group("Interface / AngelScript" FILES ${Interface_AngelScript})

set(Interface__CPP
    "ExtensionManager.cpp"
    "ExtensionManager.h"
    "Font.cpp"
    "Font.h"
    "MusicsManager.cpp"
    "MusicsManager.h"
    "Scene.cpp"
    "Scene.h"
    "SeaurchinExtension.h"
    "SoundManager.cpp"
    "SoundManager.h"
    "SusAnalyzer.cpp"
    "SusAnalyzer.h"
)
source_group("Interface / C++" FILES ${Interface__CPP})

set(Interface_Character
    "Character.cpp"
    "Character.h"
    "CharacterInstance.cpp"
    "CharacterInstance.h"
    "Result.cpp"
    "Result.h"
    "Skill.cpp"
    "Skill.h"
)
source_group("Interface / Character" FILES ${Interface_Character})

set(Interface_Drawing
    "Easing.cpp"
    "Easing.h"
    "MoverFunctionExpression.cpp"
    "MoverFunctionExpression.h"
    "ScriptResource.cpp"
    "ScriptResource.h"
    "ScriptScene.cpp"
    "ScriptScene.h"
    "ScriptSprite.cpp"
    "ScriptSprite.h"
    "ScriptSpriteMisc.h"
    "ScriptSpriteMover.cpp"
    "ScriptSpriteMover.h"
)
source_group("Interface / Drawing System" FILES ${Interface_Drawing})

set(Controller
    "Controller.cpp"
    "Controller.h"
)
source_group("Controller" FILES ${Controller})

set(Src_Files
    "Debug.cpp"
    "ExecutionManager.cpp"
    "ExecutionManager.Interface.cpp"
    "Main.cpp"
    "Misc.cpp"
    "Setting.cpp"
)
source_group("Source files" FILES ${Src_Files})

set(Builtin_Scene
    "SceneDebug.cpp"
    "SceneDebug.h"
    "SceneDeveloperMode.cpp"
    "SceneDeveloperMode.h"
)
source_group("Built-in scene" FILES ${Builtin_Scene})

set(Player
    "AutoPlayerProcessor.cpp"
    "PlayableProcessor.cpp"
    "ScenePlayer.cpp"
    "ScenePlayer.Draw.cpp"
    "ScenePlayer.h"
    "ScoreProcessor.h"
)
source_group("Player" FILES ${Player})

set(Header_Files
    "Config.h"
    "Debug.h"
    "ExecutionManager.h"
    "Main.h"
    "Misc.h"
    "PrecompiledHeader.cpp"
    "PrecompiledHeader.h"
    "resource.h"
    "Setting.h"
)
source_group("Header files" FILES ${Header_Files})

set(Header_Library
    "Crc32.h"
)
source_group("Header files / Library" FILES ${Header_Library})

set(ALL_FILES
    ${AngelScript}
    #${DxLib}
    ${Interface_AngelScript}
    ${Interface__CPP}
    ${Interface_Character}
    ${Interface_Drawing}
    ${Controller}
    ${Src_Files}
    ${Builtin_Scene}
    ${Player}
    ${Header_Files}
    ${Header_Library}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

add_precompiled_header(${PROJECT_NAME} "PrecompiledHeader.h" "PrecompiledHeader.cpp")

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE Seaurchin)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/x86/$<CONFIG>/"
    OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/x86/$<CONFIG>/"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/../library/fmt;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../library/angelscript/angelscript/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../library/angelscript/angelscript/add_on;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../library/boost;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../library/angelscript/add_on;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../library/freetype/include;"
    "/usr/include/freetype2"
    "${CMAKE_CURRENT_SOURCE_DIR}/../library/libpng;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../library/bass24_mix;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../library/bass24_fx;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../library/bass24;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../library/zlib;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../library/spdlog/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../library/libvorbis/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../library/libogg/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../library/tinytoml/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../library/glm;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../library/libjpeg/Release"
)

################################################################################
# Compile and link options
################################################################################


target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:
        -Og
    >
    $<$<CONFIG:Release>:
        -O3;
    >
    ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
    ${DEFAULT_CXX_EXCEPTION_HANDLING}
)