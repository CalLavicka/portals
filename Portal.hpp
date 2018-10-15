#pragma once

#include <glm/glm.hpp>

#include "Scene.hpp"

struct Portal {
	Portal();

	glm::vec2 position;
	glm::vec2 normal;
	
	void move(glm::vec2 const &vec);
	void rotate(float const &to_rot);

	Scene::Transform *portal_transform;
};

