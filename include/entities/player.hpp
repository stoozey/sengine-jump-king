#ifndef SENGINE_JUMP_KING_PLAYER_HPP
#define SENGINE_JUMP_KING_PLAYER_HPP

#include <utility>

#include <core/entity.hpp>
#include <assets/texture.hpp>
#include <classes/mesh.hpp>

class Player: public core::Entity {
public:
    Player();

    void Update(double deltaTime) override;
    void Render() override;
private:
    enum class PlayerState {
        Idle,
        Walking,
        JumpPrepare,
        Jump,
        LandFail
    };

    classes::Mesh mesh;
    PlayerState state;
    std::string textureName;
    double stateTime;

    int walkDirLast = 1;
    int walkTextureIndex = 0;
    double walkTextureTimer;

    int GetInputXAxis();

    void SetState(PlayerState newState);
};

#endif //SENGINE_JUMP_KING_PLAYER_HPP
