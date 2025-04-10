#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <cstdlib> // For rand()
#include <algorithm>

#include <iostream> // debug

struct Particle
{
    float density;
    glm::vec2 position;
    glm::vec2 predictedPosition;
    glm::vec2 velocity;
    glm::vec2 gradient;
    glm::vec2 pressureAcceleration;

    Particle(const glm::vec2 &pos, const glm::vec2 &vel)
        : position(pos), velocity(vel), density(0.0f) {}
};

std::vector<Particle> generateUniformGridParticles(int numParticles, float minX, float maxX, float minY, float maxY);

std::vector<Particle> generateParticles(int numParticles, float minX, float maxX, float minY, float maxY);

class Simulation
{
private:
    float radius;
    float mass;
    float damping;
    float targetDensity;
    float pressureMultiplier;
    float mouseForce;
    glm::vec2 gravity;
    float poly6KernelConstant;
    float spikyKernelGradientConstant;

    void boundaryCondition(Particle &particles);

    void calculateDensity(std::vector<Particle> &particles);

    void calculatePressureForce(std::vector<Particle> &particles);

    float smoothingKernel(float dst);

    float smoothingKernelDerivative(float dst);

public:
    Simulation(float rad, float mas, float damp);

    void updateParticles(std::vector<Particle> &particles, float deltaTime, glm::vec3 mouseVector);
};
