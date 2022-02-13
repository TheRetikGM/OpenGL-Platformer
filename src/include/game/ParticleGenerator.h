#pragma once
#include <glm/glm.hpp>
#include "shader.h"
#include "texture.h"
#include "game/GameObject.h"
#include "Timer.hpp"

struct Particle {
    glm::vec2 vPosition = glm::vec2(0.0f);
    glm::vec2 vVelocity = glm::vec2(0.0f);
    glm::vec4 vColor = glm::vec4(1.0f);
    float fLife = 0.0f;

    Particle() = default;
};

class ParticleGenerator
{
public:
    ParticleGenerator(Shader& shader, Texture2D& texture, unsigned int amount, float spawnLimit = 0.001f);

    void Update(float dt, GameObject& object, unsigned int newParticles, glm::vec2 offset = glm::vec2(0.0f));
    void Render();

protected:
    std::vector<Particle> particles;
    unsigned int nParticles;
    unsigned int nLastUsedParticle = 0;
    // Timer to limit number of particle spawns per second.
    Timer spawnTimer;
    bool bCanSpawnNext = true;
    float fSpawnLimit;

    // Size of the particle in tiles.
    glm::vec2 vParticleSize = glm::vec2(0.2f);
    Shader shader;
    Texture2D texture;
    unsigned int VAO;

    void init();

    // Resturn the first particle, that is currently unused. e.g. Life <= 0.0f or 0 if no particle is currently inactive.
    unsigned int firstUnusedParticle();

    void respawnParticle(Particle& particle, GameObject& object, glm::vec2 offset = glm::vec2(0.0f, 0.0f));

    void onTimerTick(Timer* t);
};