#include "Main.h"

std::shared_ptr<Setting> setting;
std::shared_ptr<Logger> logger;
std::unique_ptr<ExecutionManager> manager;

using namespace Rendering;

int main(int argc, char **argv){
    PreInitialize();
    if(!Initialize()){
        logger->LogError("Initialization failed.");
        Terminate();
        return 1;
    }

    Run();

    Terminate();
    return 0;
}

void PreInitialize(){
    logger = std::make_shared<Logger>();
    logger->Initialize();
    logger->LogDebug("Logging initialized");

    setting = std::make_shared<Setting>();
    setting->Load(SU_SETTING_FILE);

    logger->LogDebug("Pre-init successful, settings loaded");
}

bool Initialize(){

    SDL_Window *window_ptr;
    SDL_Renderer *renderer_ptr;
    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)){
        logger->LogError("Failed to initialize SDL");
        return false;
    }

    if(SDL_CreateWindowAndRenderer(
        SU_RES_WIDTH,
        SU_RES_HEIGHT,
        SDL_WINDOW_OPENGL,
        &window_ptr,
        &renderer_ptr))
    {
        SDL_SetWindowTitle(window, (SU_APP_NAME " " SU_APP_VERSION));
        logger->LogInfo("Window created");
    }
    else{
        logger->LogError("Window creation failed.");
        return false;
    }

    MoverFunctionExpressionManager::Initialize();
    if(!easing::RegisterDefaultMoverFunctionExpressions()){
        logger->LogError("Failed to initialize easing functions");
        return false;
    }

    manager = std::make_unique<ExecutionManager>(setting);
    manager->Initialize();

    SSpriteMover::StrTypeId = manager->GetScriptInterfaceUnsafe()->GetEngine()->GetTypeIdByDecl("string");

    logger->LogDebug("Initialization complete");

    return true;
}

void Run(){
    using namespace std::chrono;

    manager->AddScene(std::static_pointer_cast<Scene>(std::make_shared<SceneDebug>()));
    auto start_time = high_resolution_clock::now();
    bool run = true;
    while(run){
        auto frame_start = high_resolution_clock::now();
        const auto delta_time = duration_cast<nanoseconds>(start_time - frame_start).count() / 1e9;
        manager->Tick(delta_time);
        manager->Draw();
    }
}

void Terminate(){
    if(manager) manager->Shutdown();
    manager.reset(nullptr);
    MoverFunctionExpressionManager::Finalize();
    if(setting) setting->Save();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    logger->LogInfo("Terminating...");
    if(logger) logger->Terminate();
}