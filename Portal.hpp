#pragma once

#include <glm/glm.hpp>

#include "Scene.hpp"
#include "BoundingBox.hpp"

struct Portal {
	Portal();
	~Portal();

	glm::vec2 position;
	glm::vec2 normal;

	void move(glm::vec2 const &vec);
	void rotate(float const &to_rot);

	Scene::Transform *portal_transform;
	BoundingBox *boundingbox = nullptr;

	void update_boundingbox();  // call this when position or normal is changed
	bool is_in_portal(const Scene::Transform *object_transform);
	bool should_teleport(const Scene::Transform *object_transform);
};
