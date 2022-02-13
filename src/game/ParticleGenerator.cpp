#include "game/ParticleGenerator.h"
#include "glad/glad.h"
#include "Helper.hpp"
#include "tileCamera2D.h"
#include "game/game.h"

ParticleGenerator::ParticleGenerator(Shader& shader, Texture2D& texture, unsigned int amount, float spawnLimit)
    : shader(shader)
    , texture(texture)
    , nParticles(amount)
    , spawnTimer()
    , fSpawnLimit(spawnLimit)
{
    init();
}

void ParticleGenerator::Update(float dt, GameObject& object, unsigned int newParticles, glm::vec2 offset)
{
    // Update timer.
    spawnTimer.Update(dt);

    if (bCanSpawnNext)
    {
        // Add new particles.
        for (unsigned int i = 0; i < newParticles; i++)
        {
            int unusedParticle = firstUnusedParticle();
            respawnParticle(particles[unusedParticle], object, offset);
        }
        bCanSpawnNext = false;
    }

    // Update all particles
    for (unsigned int i = 0; i < nParticles; i++)
    {
        Particle& p = particles[i];
        p.fLife -= dt;  // Reduce life (by 1 per second).
        if (p.fLife > 0.0f)
        {
            p.vPosition -= p.vVelocity * dt;
            p.vColor.a -= dt * 2.5f;
        }
    }
}
void ParticleGenerator::Render()
{
    // use additive blending to give it a 'glow' effect.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    this->shader.Use();
    this->texture.Bind();
    glBindVertexArray(VAO);
    for (Particle& p : this->particles)
    {
        if (p.fLife > 0.0f)
        {
            glm::vec2 scale = vParticleSize * TileCamera2D::GetScale() * Game::TileSize;
            this->shader.SetVec4f("offset_scale", glm::vec4(TileCamera2D::GetScreenPosition(p.vPosition), scale));
            this->shader.SetVec4f("color", p.vColor);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }
    glBindVertexArray(0);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
// Resturn the first particle, that is currently unused. e.g. Life <= 0.0f or 0 if no particle is currently inactive.
unsigned int ParticleGenerator::firstUnusedParticle()
{
    // First search from last used particle this will usually return almost instantly.
    for (unsigned int i = nLastUsedParticle; i < this->nParticles; i++)
    {
        if (this->particles[i].fLife <= 0.0f)
        {
            nLastUsedParticle = i;
            return i;
        }
    }

    // Otherwise, do a linear search.
    for (unsigned int i = 0; i < nLastUsedParticle; i++)
    {
        if (this->particles[i].fLife <= 0.0f)
        {
            nLastUsedParticle = i;
            return i;
        }
    }

    nLastUsedParticle = 0;
    return 0;
}
void ParticleGenerator::respawnParticle(Particle& particle, GameObject& object, glm::vec2 offset)
{
    glm::vec2 half_size = object.Size * 0.3f;
    glm::vec2 random = glm::vec2(Helper::RandomFloat(-half_size.x, half_size.x), Helper::RandomFloat(-half_size.y, half_size.y));
    float rColor = 0.5f + Helper::RandomFloat(0.0f, 1.0f);
    particle.vPosition = object.Position + random + offset;
    particle.vColor = glm::vec4(rColor, rColor, rColor, 1.0f);
    particle.fLife = 0.9f;
    particle.vVelocity = object.Velocity * 0.1f;
}
void ParticleGenerator::onTimerTick(Timer* t)
{
    printf("here\n");
    this->bCanSpawnNext = true;
}
void ParticleGenerator::init()
{
    unsigned int VBO;
    float vertices[] = {
        // pos    // tex
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f,			
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindVertexArray(0);

    // Create this->nParticles default particle instances.
    for (unsigned int i = 0; i < this->nParticles; i++)
        this->particles.push_back(Particle());

    // Init timer
    spawnTimer.Repeat(true).Start(fSpawnLimit, std::bind(&ParticleGenerator::onTimerTick, this, std::placeholders::_1));
}