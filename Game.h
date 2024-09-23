#ifndef GAME_H
#define GAME_H

#include "Engine/Game.h"
#include "Engine/Scene.h"
#include "Engine/Entity.h"
#include "Engine/Components.h"
#include "Engine/Systems.h"
#include "Colliders.h"
#include "Player.h"
#include <iostream>
#include <SDL2/SDL.h>
#include "Render.h"
#include <entt/entt.hpp>

const int WIDTH = 1024;
const int HEIGHT = 768;

class MovementSystem : public UpdateSystem {
public:
    void run(float dT) {
        auto view = scene->r.view<PositionComponent, VelocityComponent>();
        for (auto e : view) {
            auto& pos = view.get<PositionComponent>(e);
            auto& vel = view.get<VelocityComponent>(e);
            pos.x += (vel.x * dT);
            pos.y += (vel.y * dT);
        }
    }
};

class WallHitSystem : public UpdateSystem {
public:
    void run(float dT) {
        auto view = scene->r.view<PositionComponent, VelocityComponent, SpriteComponent>();
        for (auto e : view) {
            auto& pos = view.get<PositionComponent>(e);
            auto& spr = view.get<SpriteComponent>(e);
            auto& vel = view.get<VelocityComponent>(e);
            int newPosX = pos.x + vel.x * dT;
            int newPosY = pos.y + vel.y * dT;

            // Rebotar en los bordes
            if (newPosX < 0 || newPosX + spr.width > WIDTH) {
                vel.x *= -1;
            }
            if (newPosY < 0 || newPosY + spr.height > HEIGHT) {
                vel.y *= -1;
            }
        }
    }
};

class LoseSystem : public UpdateSystem {
public:
    void run(float dT) {
        auto view = scene->r.view<PositionComponent, VelocityComponent, SpriteComponent>();
        for (auto e : view) {
            auto& pos = view.get<PositionComponent>(e);
            auto& spr = view.get<SpriteComponent>(e);
            auto& vel = view.get<VelocityComponent>(e);
            int newPosY = pos.y + vel.y * dT;

            // Verificar si el jugador pierde (cuando un objeto cae al fondo)
            if (newPosY + spr.height > HEIGHT) {
                std::cout << "YOU LOST" << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }
    }
};

class EnemyCheckSystem : public UpdateSystem {
public:
    void run(float dT) override {
        auto view = scene->r.view<EnemyCollider>();
        bool allEnemiesEliminated = true;

        // Verificar si todos los enemigos han sido destruidos
        for (auto e : view) {
            auto& enemy = view.get<EnemyCollider>(e);
            if (!enemy.isDestroyed) {
                allEnemiesEliminated = false;
                break;
            }
        }

        if (allEnemiesEliminated) {
            std::cout << "YOU WIN" << std::endl;
            std::exit(EXIT_SUCCESS);
        }
    }
};

class SquareSpawnSetupSystem : public SetupSystem {
public:
    void run() {
        // Crear entidad para el jugador
        auto* square = scene->createEntity("SQUARE", WIDTH / 2, HEIGHT - 40);
        square->addComponent<BarCollider>(200, 30);
        square->addComponent<SpriteComponent>(200, 30, SDL_Color{0, 0, 255});
        square->addComponent<VelocityComponent>(0, 0);
        square->addComponent<PlayerComponent>(true, 500, 500);

        // Crear entidad para el obstáculo
        auto* square2 = scene->createEntity("SQUARE2", 100, 100);
        square2->addComponent<ColliderComponent>(100, 100, false, true);
        square2->addComponent<VelocityComponent>(-200, 200);
        square2->addComponent<SpriteComponent>(100, 100, SDL_Color{229, 235, 231});

        // Crear enemigos
        crearEnemigo(500, 0);
        crearEnemigo(650, 200);
        crearEnemigo(750, 0);
        crearEnemigo(300, 200);
    }

private:
    void crearEnemigo(int x, int y) {
        auto* enemy = scene->createEntity("Enemy", x, y);
        enemy->addComponent<SpriteComponent>(100, 50, SDL_Color{255, 0, 0});
        enemy->addComponent<EnemyCollider>(false);
    }
};

class SquareRenderSystem : public RenderSystem {
public:
    void run(SDL_Renderer* renderer) {
        auto view = scene->r.view<PositionComponent, SpriteComponent>();
        for (auto e : view) {
            auto& pos = view.get<PositionComponent>(e);
            auto& spr = view.get<SpriteComponent>(e);
            SDL_SetRenderDrawColor(renderer, spr.color.r, spr.color.g, spr.color.b, spr.color.a);
            SDL_Rect r = {pos.x, pos.y, spr.width, spr.height};
            SDL_RenderFillRect(renderer, &r);
        }
    }
};

class DemoGame : public Game {
public:
    Scene* sampleScene;
    entt::registry r;

public:
    DemoGame() : Game("SAMPLE", WIDTH, HEIGHT) {}

    void setup() {
        sampleScene = new Scene("SAMPLE SCENE", r);

        addSetupSystem<SquareSpawnSetupSystem>(sampleScene);
        addUpdateSystem<ColliderResetSystem>(sampleScene);
        addUpdateSystem<PlayerColliderSystem>(sampleScene);
        addUpdateSystem<LoseSystem>(sampleScene);
        addUpdateSystem<PlayerWallHitSystem>(sampleScene);
        addUpdateSystem<EnemyCollisionSystem>(sampleScene);
        addUpdateSystem<WallHitSystem>(sampleScene);
        addUpdateSystem<EnemyCheckSystem>(sampleScene); // Verificar si los enemigos han sido eliminados
        addEventSystem<PlayerMovementSystem>(sampleScene);
        addUpdateSystem<MovementSystem>(sampleScene);
        addRenderSystem<SquareRenderSystem>(sampleScene);

        setScene(sampleScene);
    }
};

#endif // GAME_H
