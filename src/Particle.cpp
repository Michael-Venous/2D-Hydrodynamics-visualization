#include "Particle.h"

std::vector<Particle> generateUniformGridParticles(int numParticles, float minX, float maxX, float minY, float maxY)
{
    std::vector<Particle> particles;
    int particlesPerRow = static_cast<int>(sqrt(numParticles));
    float spacingX = (maxX - minX) / particlesPerRow;
    float spacingY = (maxY - minY) / particlesPerRow;

    for (int i = 0; i < particlesPerRow; ++i)
    {
        for (int j = 0; j < particlesPerRow; ++j)
        {
            glm::vec2 position(minX + i * spacingX, minY + j * spacingY);
            glm::vec2 velocity(0.0f, 0.0f); // Initial velocity is zero
            particles.emplace_back(position, velocity);
        }
    }

    return particles;
}

std::vector<Particle> generateParticles(int numParticles, float minX, float maxX, float minY, float maxY)
{
    std::vector<Particle> particles;
    particles.reserve(numParticles);

    for (int i = 0; i < numParticles; ++i)
    {
        // Random position within specified bounds
        glm::vec2 position = glm::vec2(
            minX + static_cast<float>(rand()) / RAND_MAX * (maxX - minX),
            minY + static_cast<float>(rand()) / RAND_MAX * (maxY - minY));

        // Initial velocity (e.g., stationary)
        glm::vec2 velocity = glm::vec2(0.0f, 0.0f);

        // Particle radius
        float radius = 0.01f;

        particles.emplace_back(position, velocity);
    }

    return particles;
}

Simulation::Simulation(float rad, float mas, float damp)
    : radius(rad), mass(mas), damping(damp), targetDensity(200.0f), pressureMultiplier(50.0f), gravity(0.0f, -9.81f)
{
    // Precompute constants for the smoothing kernel
    poly6KernelConstant = 4.0f / (M_PI * pow(radius, 8));
    spikyKernelGradientConstant = -30.0f / (M_PI * pow(radius, 5));
}

void Simulation::updateParticles(std::vector<Particle> &particles, float deltaTime, glm::vec3 mouseVector)
{

    for (auto &particle : particles)
    {
        particle.predictedPosition = particle.position + particle.velocity * deltaTime;
    }

    calculateDensity(particles);
    calculatePressureForce(particles);

    for (auto &particle : particles)
    {
        // Apply the mouse force (vector) to the particle's velocity
        glm::vec2 mousePos = glm::vec2(mouseVector.x, mouseVector.y); // Only use x, y for position
        float mouseForce = mouseVector.z;                             // Use z-component of the mouseVector for force magnitude

        // Calculate the direction vector from the particle to the mouse position
        glm::vec2 mouseForceVec = mousePos - particle.position;
        float distance = glm::length(mouseForceVec); // Calculate the distance to the mouse position

        // If the particle is within a reasonable distance from the mouse, apply force
        if (distance < 2.0f)
        {                                                  // You can adjust this threshold to control the influence range
            mouseForceVec = glm::normalize(mouseForceVec); // Normalize to get the direction

            // Scale the force vector by the mouse force magnitude and inverse of distance
            if (mouseForce > 0)
            {
                particle.velocity += mouseForceVec * (mouseForce / distance); // Attraction
            }
            else
            {
                particle.velocity -= mouseForceVec * (-mouseForce / distance); // Repulsion
            }
        }

        particle.velocity += particle.pressureAcceleration * deltaTime;
        particle.velocity *= damping;
        particle.velocity += gravity * deltaTime;
        particle.position += particle.velocity * deltaTime;

        boundaryCondition(particle);
    }
}

void Simulation::boundaryCondition(Particle &particle)
{
    float minX = -1.0f, maxX = 1.0f;
    float minY = -1.0f, maxY = 1.0f;
    constexpr float boundaryDamping = 0.5f; // Energy loss on collision (0.5 = 50% velocity retained)
    // X-axis collision
    if (particle.position.x < minX)
    {
        // Clamp position to boundary
        particle.position.x = minX;
        // Reflect velocity with damping
        particle.velocity.x *= -boundaryDamping;
    }
    else if (particle.position.x > maxX)
    {
        particle.position.x = maxX;
        particle.velocity.x *= -boundaryDamping;
    }

    // Y-axis collision
    if (particle.position.y < minY)
    {
        particle.position.y = minY;
        particle.velocity.y *= -boundaryDamping;
    }
    else if (particle.position.y > maxY)
    {
        particle.position.y = maxY;
        particle.velocity.y *= -boundaryDamping;
    }
    constexpr float boundaryPush = 0.05f;
    if (particle.position.x == minX)
        particle.velocity.x += boundaryPush;
    if (particle.position.x == maxX)
        particle.velocity.x -= boundaryPush;
    if (particle.position.y == minY)
        particle.velocity.y += boundaryPush;
    if (particle.position.y == maxY)
        particle.velocity.y -= boundaryPush;
}

// precomputes the density
void Simulation::calculateDensity(std::vector<Particle> &particles)
{
    for (auto &particle : particles)
    {
        particle.density = 0.0f;
        for (auto &particleCheck : particles)
        {
            glm::vec2 distVector = particle.predictedPosition - particleCheck.predictedPosition;
            float distance = glm::length(distVector);
            if (distance > radius)
                continue;

            float influence = smoothingKernel(distance);
            particle.density += influence * mass;
        }
    }
}

// calculating the pressure gradient
void Simulation::calculatePressureForce(std::vector<Particle> &particles)
{
    for (auto &particle : particles)
    {
        glm::vec2 pressureForce = glm::vec2(0.0f);

        for (auto &particleCheck : particles)
        {
            glm::vec2 distVector = particle.predictedPosition - particleCheck.predictedPosition;
            float distance = glm::length(distVector);

            if (distance > radius || &particle == &particleCheck)
                continue;

            glm::vec2 direction = distVector / distance;
            float influence = smoothingKernelDerivative(distance);

            float pressure_i = std::max((particle.density - targetDensity) * pressureMultiplier, 0.0f);
            float pressure_j = std::max((particleCheck.density - targetDensity) * pressureMultiplier, 0.0f);
            float avgPressure = (pressure_i + pressure_j) / 2.0f;
            pressureForce += -direction * (avgPressure * influence * mass / particleCheck.density);
        }
        particle.pressureAcceleration = pressureForce / particle.density;
    }
}

float Simulation::smoothingKernel(float dst)
{
    float diff = (radius * radius - dst * dst);
    return poly6KernelConstant * std::pow(diff, 3);
}

float Simulation::smoothingKernelDerivative(float dst)
{
    float diff = radius - dst;
    return spikyKernelGradientConstant * diff * diff;
}