#include <core/engine.hpp>
#include <managers/asset_manager.hpp>
#include <managers/input_manager.hpp>
#include <loop_runners/entity_loop_runner.hpp>

#include "entities/player.hpp"

int main(int argv, char **args) {
    g_Engine->Initialize();

    //// add managers
    // asset manager
    g_Engine->AddManager<managers::AssetManager>();

    // input manager
    auto inputManager = g_Engine->AddManager<managers::InputManager>();
    inputManager->DefineInput("left");
    inputManager->DefineInput("right");
    inputManager->DefineInput("up");
    inputManager->DefineInput("jump");

    inputManager->TrackInput("left", SDL_SCANCODE_A);
    inputManager->TrackInput("right", SDL_SCANCODE_D);
    inputManager->TrackInput("up", SDL_SCANCODE_W);
    inputManager->TrackInput("jump", SDL_SCANCODE_SPACE);

    //// add loop runners
    // entity loop runner
    g_Engine->AddLoopRunner<loopRunners::EntityLoopRunner>();

    //// init entities
    auto entityLoopRunner = g_Engine->GetLoopRunner<loopRunners::EntityLoopRunner>();
    entityLoopRunner->CreateEntity<Player>();

    //// run engine
    g_Engine->RunLoop();

    //// cleanup
    g_Engine.reset();
    return 0;
}