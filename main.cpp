#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "header/shaderClass.h"
#include "header/VAO.h"
#include "header/VBO.h"
#include "header/EBO.h"
#include "header/Particle.h"

std::vector<float> generateCircleVertices(const glm::vec2 &center, float radius, int numSegments)
{
	std::vector<float> vertices;
	vertices.reserve((numSegments + 2) * 2); // +2 for center and closing point
	vertices.push_back(center.x);			 // Center of the circle
	vertices.push_back(center.y);

	for (int i = 0; i <= numSegments; ++i)
	{
		float angle = 2.0f * M_PI * i / numSegments;
		vertices.push_back(center.x + radius * cos(angle));
		vertices.push_back(center.y + radius * sin(angle));
	}
	return vertices;
}

int main()
{
	// GLFW init
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow *window = glfwCreateWindow(800, 800, "ParticleSim9000", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// load GL + functions
	gladLoadGL();
	glViewport(0, 0, 800, 800);

	// Compiles, attaches, and generates shaderprogram.
	Shader shaderProgram("../../Resource Files/Shaders/default.vert", "../../Resource Files/Shaders/default.frag");

	// Circle parameters
	glm::vec2 center(0.0f, 0.0f); // Circle center
	float visualRadius = 0.1f;	  // Circle radius
	int numSegments = 10;		  // Number of segments

	// Generate circle vertices
	std::vector<float> circleVertices = generateCircleVertices(center, visualRadius, numSegments);

	// Generates Vertex Array Object and binds it
	VAO VAO1;
	VAO1.Bind();
	// Generates Vertex Buffer Object and links it to vertices
	VBO VBO1(circleVertices.data(), circleVertices.size() * sizeof(float));
	// Links VBO to VAO
	VAO1.LinkVBO(VBO1, 0);
	VAO1.Unbind();
	VBO1.Unbind();

	// Set up the simulation parameters
	float mass = 1.0f;		  // Mass of the particles
	float damping = 1.0f;	  // Damping multiplier
	float timeStep = 0.003f;  // Time step for the simulation
	float radius = 0.08f;	  // Radius of the particles
	float mouseForce = 0.02f; // Mouse force multiplier

	Simulation simulation = Simulation(radius, mass, damping);

	std::vector<Particle> particles = generateParticles(500, -0.5, 0.5, -0.5, 0.5);
	// Main while loop
	while (!glfwWindowShouldClose(window))
	{
		// Mouse cursor
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
		bool attract = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
		bool repel = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
		float finalMouseForce = mouseForce * attract - mouseForce * repel;
		glm::vec3 mouseVector((2.0f * mouseX) / 800 - 1.0f, 1.0f - (2.0f * mouseY) / 800, finalMouseForce);

		simulation.updateParticles(particles, timeStep, mouseVector);

		// Clear BG
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		shaderProgram.Activate();

		// draw every particle
		for (const auto &particle : particles)
		{
			// Compute color
			float speed = glm::length(particle.velocity);
			float maxSpeed = 2.0f;
			float t = std::min(speed / maxSpeed, 1.0f);
			glm::vec3 color = glm::mix(glm::vec3(0.0f, 0.0f, 1.0f), // blue
									   glm::vec3(1.0f, 0.0f, 0.0f), // red
									   t);
			shaderProgram.setVec3("particleColor", color);

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(particle.position, 0.0f));
			model = glm::scale(model, glm::vec3(visualRadius, visualRadius, 1.0f));
			shaderProgram.setMat4("model", model);
			VAO1.Bind();
			glDrawArrays(GL_TRIANGLE_FAN, 0, circleVertices.size() / 2);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Delete objects
	VAO1.Delete();
	VBO1.Delete();
	shaderProgram.Delete();
	glfwDestroyWindow(window);
	glfwTerminate();
}