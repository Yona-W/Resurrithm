set(PROJECT_NAME Seaurchin)

################################################################################
# Source groups
################################################################################
set(AngelScript
    "../library/angelscript/add_on/scriptarray/scriptarray.cpp"
    "../library/angelscript/add_on/scriptdictionary/scriptdictionary.cpp"
    "../library/angelscript/add_on/scriptbuilder/scriptbuilder.cpp"
    "../library/angelscript/add_on/scriptmath/scriptmath.cpp"
    "../library/angelscript/add_on/scriptmath/scriptmathcomplex.cpp"
    "../library/angelscript/add_on/scriptstdstring/scriptstdstring.cpp"
    "../library/angelscript/add_on/scriptstdstring/scriptstdstring_utils.cpp"
)
source_group("AngelScript" FILES ${AngelScript})

set(Interface_AngelScript
    "AngelScriptManager.cpp"
    "Interfaces.cpp"
    "ScriptFunction.cpp"
    "SkinHolder.cpp"
)
source_group("Interface / AngelScript" FILES ${Interface_AngelScript})

set(Interface__CPP
    "ExtensionManager.cpp"
    "Font.cpp"
    "MusicsManager.cpp"
    "Scene.cpp"
    "SoundManager.cpp"
    "SusAnalyzer.cpp"
)
source_group("Interface / C++" FILES ${Interface__CPP})

set(Interface_Character
    "Character.cpp"
    "CharacterInstance.cpp"
    "Result.cpp"
    "Skill.cpp"
)
source_group("Interface / Character" FILES ${Interface_Character})

set(Interface_Drawing
    "Easing.cpp"
    "MoverFunctionExpression.cpp"
    "ScriptResource.cpp"
    "ScriptScene.cpp"
    "ScriptSprite.cpp"
    "Rendering.cpp"
    "ScriptSpriteMover.cpp"
)
source_group("Interface / Drawing System" FILES ${Interface_Drawing})

set(Controller
    "Controller.cpp"
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
    "SceneDeveloperMode.cpp"
)
source_group("Built-in scene" FILES ${Builtin_Scene})

set(Player
    "AutoPlayerProcessor.cpp"
    "PlayableProcessor.cpp"
    "ScenePlayer.cpp"
    "ScenePlayer.Draw.cpp"
)
source_group("Player" FILES ${Player})

set(ALL_FILES
    ${AngelScript}
    ${Interface_AngelScript}
    ${Interface__CPP}
    ${Interface_Character}
    ${Interface_Drawing}
    ${Controller}
    ${Src_Files}
    ${Builtin_Scene}
    ${Player}
)


################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

# add_precompiled_header(${PROJECT_NAME} "PrecompiledHeader.h" "PrecompiledHeader.cpp")

################################################################################
# Output directory
################################################################################

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
    "${CMAKE_CURRENT_SOURCE_DIR}/../library/rang/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../library/sdl-gpu/include"
)

################################################################################
# Libraries
################################################################################

find_library(LibAngelscript NAMES libangelscript.a PATHS ../library/obj REQUIRED)
find_library(LibBass NAMES bass PATHS ../library/obj REQUIRED)
find_library(LibBassFx NAMES bass_fx PATHS ../library/obj REQUIRED)
find_library(LibBassMix NAMES bassmix PATHS ../library/obj REQUIRED)
find_library(LibSDL2_GPU NAMES libSDL2_gpu.a PATHS ../library/obj REQUIRED)
find_library(LibSpdlog NAMES libspdlog.a PATHS ../library/obj REQUIRED)

find_package(OpenGL REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(Threads REQUIRED)
find_package(SDL2 REQUIRED)
find_package(PNG REQUIRED)
find_package(Freetype REQUIRED)

target_link_libraries(${PROJECT_NAME}
    ${LibAngelscript}
    ${LibBass}
    ${LibBassFx}
    ${LibBassMix}
    ${LibSDL2_GPU}
    ${LibSpdlog}
    SDL2::SDL2
    OpenGL::GL
    PNG::PNG
    Freetype::Freetype
    ${Boost_LIBRARIES}
    Threads::Threads
)

################################################################################
# Compile and link options
################################################################################