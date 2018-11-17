#pragma once

#include <glm/glm.hpp>
#include "Scene.hpp"

struct Particle {
	Particle(glm::vec2, std::string);
	~Particle();

	glm::vec2 position;
	glm::vec2 velocity;
    glm::vec2 acceleration;
    std::string spice;
    float lifespan;

    void update(float elapsed);
};
