#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <classes/primitive.hpp>
#include <structs/primitive_type.hpp>
#include <core/engine.hpp>
#include <managers/asset_manager.hpp>
#include <assets/texture.hpp>
#include <assets/shader.hpp>
#include <managers/input_manager.hpp>
#include <core/log.hpp>

#include "entities/player.hpp"

const std::string PLAYER_TEXTURE_IDLE = "player/idle";
const std::string PLAYER_TEXTURE_IDLE_LOOK_UP = "player/idle_look_up";
const std::string PLAYER_TEXTURE_JUMP_PREPARE = "player/jump_prepare";
const std::string PLAYER_TEXTURE_JUMP_UPWARDS = "player/jump_upwards";
const std::string PLAYER_TEXTURE_JUMP_DOWNWARDS = "player/jump_downwards";
const std::string PLAYER_TEXTURE_LAND_FAIL = "player/land_fail";
const std::string PLAYER_TEXTURE_WALK_1 = "player/walk_1";
const std::string PLAYER_TEXTURE_WALK_2 = "player/walk_2";
const std::string PLAYER_TEXTURE_WALK_3 = "player/walk_3";

Player::Player() : state(PlayerState::Idle), mesh(classes::Primitive::GetMesh(structs::PrimitiveType::Plane)) {
    SetState(state);

    position.z = -3.0f;

    static auto assetManager = g_Engine->GetManager<managers::AssetManager>();
    assetManager->LoadAsset<assets::Texture>(PLAYER_TEXTURE_IDLE);
    assetManager->LoadAsset<assets::Texture>(PLAYER_TEXTURE_IDLE_LOOK_UP);
    assetManager->LoadAsset<assets::Texture>(PLAYER_TEXTURE_JUMP_PREPARE);
    assetManager->LoadAsset<assets::Texture>(PLAYER_TEXTURE_JUMP_UPWARDS);
    assetManager->LoadAsset<assets::Texture>(PLAYER_TEXTURE_JUMP_DOWNWARDS);
    assetManager->LoadAsset<assets::Texture>(PLAYER_TEXTURE_LAND_FAIL);
    assetManager->LoadAsset<assets::Texture>(PLAYER_TEXTURE_WALK_1);
    assetManager->LoadAsset<assets::Texture>(PLAYER_TEXTURE_WALK_2);
    assetManager->LoadAsset<assets::Texture>(PLAYER_TEXTURE_WALK_3);
}

void Player::SetState(PlayerState newState) {
    state = newState;
    stateTime = 0;

    switch (state) {
        case PlayerState::Walking: {
            walkTextureIndex = 0;
            walkTextureTimer = 0;
            break;
        }
    }
}

int Player::GetInputXAxis() {
    static auto inputManager = g_Engine->GetManager<managers::InputManager>();
    return ((inputManager->GetInputDown("right")) - (inputManager->GetInputDown("left")));
}

void Player::Update(double deltaTime) {
    static auto inputManager = g_Engine->GetManager<managers::InputManager>();

    switch (state) {
        case PlayerState::Idle: {
            textureName = ((inputManager->GetInputDown("up")) ? PLAYER_TEXTURE_IDLE_LOOK_UP : PLAYER_TEXTURE_IDLE);

            int xAxis = GetInputXAxis();
            if (xAxis != 0) {
                SetState(PlayerState::Walking);
            }

            break;
        }

        case PlayerState::Walking: {
            // exit state if no longer moving
            int xAxis = GetInputXAxis();
            if (xAxis == 0) return SetState(PlayerState::Idle);

            walkDirLast = xAxis;

            // update walk texture
            static std::vector<std::string> walkTextures = {
                    PLAYER_TEXTURE_WALK_1,
                    PLAYER_TEXTURE_WALK_2,
                    PLAYER_TEXTURE_WALK_3,
                    PLAYER_TEXTURE_WALK_2
            };

            walkTextureTimer += deltaTime;
            if (walkTextureTimer > 1.33f) {
                walkTextureTimer = 0;
                walkTextureIndex = ((++walkTextureIndex >= walkTextures.size()) ? 0 : walkTextureIndex);
            }

            textureName = walkTextures[walkTextureIndex];

            // update position
            const float moveSpeed = 0.1f;
            position.x += (moveSpeed * static_cast<float>(xAxis));
            break;
        }

        case PlayerState::JumpPrepare: {
            break;
        }

        case PlayerState::Jump: {
            break;
        }

        case PlayerState::LandFail: {
            break;
        }
    }

    core::Log::Info(fmt::format("state time: {}", stateTime));
    stateTime += deltaTime;
}

void Player::Render() {
    static auto assetManager = g_Engine->GetManager<managers::AssetManager>();
    auto texture = assetManager->GetAssetOrDefault<assets::Texture>(textureName);
    auto shader = assetManager->GetAssetOrDefault<assets::Shader>("player");

    glUseProgram(shader->GetProgram());
        glBindTexture(GL_TEXTURE_2D, texture->textureId);

        glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(static_cast<float>(walkDirLast), 1.0f, 0.1f));
        shader->SetUniform("u_modelMatrix", (translation * scale));

        auto perspective = glm::perspective(glm::radians(90.0f), static_cast<float>(g_Engine->GetWindowWidth()) / static_cast<float>(g_Engine->GetWindowHeight()), 0.1f, 30.0f);
        shader->SetUniform("u_projection", perspective);

        mesh.Draw();
    glUseProgram(0);
}