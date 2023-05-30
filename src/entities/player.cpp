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

const float PLAYER_JUMP_VELOCITY_MAX = 1.5f;
const float PLAYER_JUMP_VELOCITY_MIN = -PLAYER_JUMP_VELOCITY_MAX;

const float PLAYER_JUMP_HEIGHT = 1.0f;
const float PLAYER_JUMP_TIME_MAX = 3.0f;

Player::Player() : state(PlayerState::Idle), mesh(classes::Primitive::GetMesh(structs::PrimitiveType::Plane)), currentDir(1) {
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
    switch (state) {
        case PlayerState::Walking: {
            walkTextureIndex = 0;
            walkTextureTimer = 0;
            break;
        }

        case PlayerState::Jump: {
            core::Log::Info(std::to_string(stateTime));
            jumpTime = std::clamp(static_cast<float>(stateTime), 0.1f, PLAYER_JUMP_TIME_MAX);

            float jumpHeight = (PLAYER_JUMP_HEIGHT * (jumpTime / PLAYER_JUMP_TIME_MAX));
            jumpForce = (jumpTime / jumpHeight);
            jumpVelocity = 0;
            break;
        }
    }

    state = newState;
    stateTime = 0;
}

int Player::GetInputXAxis() {
    static auto inputManager = g_Engine->GetManager<managers::InputManager>();

    int xAxis = ((inputManager->GetInputDown("right")) - (inputManager->GetInputDown("left")));
    if (xAxis != 0) {
        currentDir = xAxis;
    }

    return xAxis;
}

void Player::Update(double deltaTime) {
    static auto inputManager = g_Engine->GetManager<managers::InputManager>();

    switch (state) {
        case PlayerState::Idle: {
            // move to Walking
            int xAxis = GetInputXAxis();
            if (xAxis != 0) {
                SetState(PlayerState::Walking);
                break;
            }

            // move to JumpPrepare
            if (inputManager->GetInputPressed("jump")) {
                SetState(PlayerState::JumpPrepare);
                break;
            }

            // update texture
            textureName = ((inputManager->GetInputDown("up")) ? PLAYER_TEXTURE_IDLE_LOOK_UP : PLAYER_TEXTURE_IDLE);
            break;
        }

        case PlayerState::Walking: {
            // move to Idle
            int xAxis = GetInputXAxis();
            if (xAxis == 0) {
                SetState(PlayerState::Idle);
                break;
            }

            // update walk texture
            static std::vector<std::string> walkTextures = {
                    PLAYER_TEXTURE_WALK_1,
                    PLAYER_TEXTURE_WALK_2,
                    PLAYER_TEXTURE_WALK_3,
                    PLAYER_TEXTURE_WALK_2
            };

            walkTextureTimer += deltaTime;
            if (walkTextureTimer > 1.33) {
                walkTextureTimer = 0;
                walkTextureIndex = ((++walkTextureIndex >= walkTextures.size()) ? 0 : walkTextureIndex);
            }

            textureName = walkTextures[walkTextureIndex];

            // update position
            static const float moveSpeed = 0.1f;
            position.x += (moveSpeed * static_cast<float>(xAxis));
            break;
        }

        case PlayerState::JumpPrepare: {
            // move to Jump
            if (inputManager->GetInputReleased("jump")) {
                SetState(PlayerState::Jump);
                break;
            }

            // update texture
            textureName = PLAYER_TEXTURE_JUMP_PREPARE;
            break;
        }

        case PlayerState::Jump: {
            if (position.y <= -5) {
                SetState((stateTime >= 2) ? PlayerState::LandFail : PlayerState::Idle);
                break;
            }

            // update jumpForce
            float forceModifier = ((stateTime >= (jumpTime * 0.5)) ? 1.0f : -1.0f);
            jumpVelocity = std::clamp((jumpVelocity + (jumpForce * forceModifier)), PLAYER_JUMP_VELOCITY_MIN, PLAYER_JUMP_VELOCITY_MAX);

            // update texture
            textureName = (((std::abs(jumpForce)) == 1) ? PLAYER_TEXTURE_JUMP_UPWARDS : PLAYER_TEXTURE_JUMP_DOWNWARDS);
            break;
        }

        case PlayerState::LandFail: {
            if (stateTime > 1) {
                SetState(PlayerState::Idle);
                break;
            }

            textureName = PLAYER_TEXTURE_LAND_FAIL;
            break;
        }
    }

    position.y += static_cast<float>(jumpVelocity);

    stateTime += deltaTime;
    core::Log::Info(fmt::format("jumpForce: {}, jumpVelocity: {}, y: {}", std::to_string(jumpForce), std::to_string(jumpVelocity), std::to_string(position.y)));
}

void Player::Render() {
    static auto assetManager = g_Engine->GetManager<managers::AssetManager>();
    auto texture = assetManager->GetAssetOrDefault<assets::Texture>(textureName);
    auto shader = assetManager->GetAssetOrDefault<assets::Shader>("player");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(shader->GetProgram());
        glBindTexture(GL_TEXTURE_2D, texture->textureId);

        glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(static_cast<float>(currentDir), 1.0f, 0.1f));
        shader->SetUniform("u_modelMatrix", (translation * scale));

        auto perspective = glm::perspective(glm::radians(90.0f), static_cast<float>(g_Engine->GetWindowWidth()) / static_cast<float>(g_Engine->GetWindowHeight()), 0.1f, 30.0f);
        shader->SetUniform("u_projection", perspective);

        mesh.Draw();
    glUseProgram(0);
    glDisable(GL_BLEND);
}