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
            glm::vec2 velocity(0.0f, 0.0f);
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

Simulation::Simulation(float rad, float mas, float damp, float targetDens, float pressureMult)
    : radius(rad), mass(mas), damping(damp), targetDensity(targetDens), pressureMultiplier(pressureMult), gravity(0.0f, -9.81f)
{
    // Precompute constants for the smoothing kernel
    poly6KernelConstant = 4.0f / (M_PI * pow(radius, 8));
    spikyKernelGradientConstant = -30.0f / (M_PI * pow(radius, 5));
}

void Simulation::updateParticles(std::vector<Particle> &particles, float deltaTime, glm::vec3 mouseVector)
{
    // Clamp deltaTime to prevent instability
    deltaTime = std::clamp(deltaTime, 0.001f, 0.033f);

    // Calculate predicted positions first
    for (auto &particle : particles)
    {
        particle.predictedPosition = particle.position + particle.velocity * deltaTime;
    }

    calculateDensity(particles);
    calculatePressureForce(particles);

    for (auto &particle : particles)
    {
        // Calculate acceleration including mouse force
        glm::vec2 acceleration = particle.pressureAcceleration + gravity;

        // Apply mouse force as acceleration BEFORE Verlet integration
        const float mouseRadius = 1.0f;
        glm::vec2 mousePos(mouseVector.x, mouseVector.y);
        glm::vec2 toMouse = mousePos - particle.position;
        float distance = glm::length(toMouse);
        if (distance < mouseRadius && distance > 0.01f)
        {
            float normalizedDist = distance / mouseRadius;
            float falloff = (1.0f - normalizedDist * normalizedDist);
            float mouseAccel = mouseVector.z * 50.0f * falloff / (distance + 0.1f);
            acceleration += glm::normalize(toMouse) * mouseAccel;
        }

        // Verlet integration with acceleration
        glm::vec2 newPosition = 2.0f * particle.position - particle.previousPosition + acceleration * deltaTime * deltaTime;

        // Update velocity
        particle.velocity = (newPosition - particle.previousPosition) / (2.0f * deltaTime);
        particle.velocity *= damping;
        constexpr float maxVel = 5.0f;
        particle.velocity = glm::clamp(particle.velocity,
                                       glm::vec2(-maxVel),
                                       glm::vec2(maxVel));

        // Update positions
        particle.previousPosition = particle.position;
        particle.position = newPosition;

        boundaryCondition(particle);
    }
}

void Simulation::boundaryCondition(Particle &particle)
{
    const float minX = -1.0f, maxX = 1.0f;
    const float minY = -1.0f, maxY = 1.0f;
    constexpr float boundaryDamping = 0.5f;
    constexpr float boundaryPush = 0.02f;
    constexpr float eps = 0.001f; // For position comparisons

    // Helper function for axis-aligned boundary handling
    auto handleAxis = [&](float &pos, float &prevPos, float minVal, float maxVal)
    {
        const float span = maxVal - minVal;

        // Check left boundary
        if (pos < minVal - eps)
        {
            const float overshoot = minVal - pos;
            pos = minVal + overshoot * boundaryDamping;
            prevPos = pos; // Reset previous position to prevent velocity spikes
        }
        // Check right boundary
        else if (pos > maxVal + eps)
        {
            const float overshoot = pos - maxVal;
            pos = maxVal - overshoot * boundaryDamping;
            prevPos = pos;
        }
        // Gentle boundary push for particles near edges
        else if (pos < minVal + eps)
        {
            particle.velocity.x += boundaryPush;
        }
        else if (pos > maxVal - eps)
        {
            particle.velocity.x -= boundaryPush;
        }
    };

    handleAxis(particle.position.x, particle.previousPosition.x, minX, maxX);
    handleAxis(particle.position.y, particle.previousPosition.y, minY, maxY);
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
            float pressureTerm = (pressure_i / (particle.density * particle.density) + pressure_j / (particleCheck.density * particleCheck.density));
            pressureForce += -direction * (mass * pressureTerm * influence);
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