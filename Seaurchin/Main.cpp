#include "Main.h"

std::shared_ptr<Setting> setting;
std::unique_ptr<ExecutionManager> manager;

using namespace Rendering;

int main(int argc, char **argv){
    PreInitialize();
    if(!Initialize()){
        spdlog::error("Initialization failed.");
        Terminate();
        return 1;
    }

    Run();

    Terminate();
    return 0;
}

void PreInitialize(){
    setting = std::make_shared<Setting>();
    setting->Load(SU_SETTING_FILE);
    spdlog::debug("Pre-init successful, settings loaded");
}

bool Initialize(){
    int status = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    if(status != 0){
        spdlog::error("Failed to initialize SDL");
        return false;
    }

    GPU_SetDebugLevel(GPU_DebugLevelEnum::GPU_DEBUG_LEVEL_MAX);
    GPU_SetPreInitFlags(GPU_INIT_DISABLE_VSYNC);
    Rendering::gpu = GPU_Init(SU_RES_WIDTH, SU_RES_HEIGHT, GPU_DEFAULT_INIT_FLAGS);
    if(Rendering::gpu)
    {
        //SDL_SetWindowTitle(, (SU_APP_NAME " " SU_APP_VERSION));
        spdlog::info("Window created");
    }
    else{
        spdlog::error("Window creation failed.");
        return false;
    }

    MoverFunctionExpressionManager::Initialize();
    if(!easing::RegisterDefaultMoverFunctionExpressions()){
        spdlog::error("Failed to initialize easing functions");
        return false;
    }

    manager = std::make_unique<ExecutionManager>(setting);
    manager->Initialize();

    SSpriteMover::StrTypeId = manager->GetScriptInterfaceUnsafe()->GetEngine()->GetTypeIdByDecl("string");

    spdlog::info("Initialization complete");

    return true;
}

void Run(){
    using namespace std::chrono;

    //manager->AddScene(std::static_pointer_cast<Scene>(std::make_shared<SceneDebug>()));
    auto last_frame = high_resolution_clock::now();
    spdlog::info("Running system menu");
    manager->EnumerateSkins();
    manager->ExecuteSkin();
    while(!manager->shouldExit){
        auto frame_start = high_resolution_clock::now();
        const auto delta_time = duration_cast<nanoseconds>(frame_start - last_frame).count() / 1e9;
        last_frame = frame_start;
        manager->Tick(delta_time);
        manager->Draw();
    }
}

void Terminate(){
    if(manager) manager->Shutdown();
    manager.reset(nullptr);
    MoverFunctionExpressionManager::Finalize();
    if(setting) setting->Save();

    GPU_Quit();

    spdlog::info("Terminating...");
}