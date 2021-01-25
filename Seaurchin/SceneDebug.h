#pragma once

#include "Scene.h"
#include <SDL2/SDL.h>

class SceneDebug final : public Scene {
private:
    int call = 0;
    double calc = 0;
    double fps = 0;

public:
    ~SceneDebug() = default;

    void Tick(double delta) override;
    void Draw() override;
    bool IsDead() override;
};
