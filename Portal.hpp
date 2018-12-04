#pragma once

#include <glm/glm.hpp>
#include "Scene.hpp"
#include "BoundingBox.hpp"

#include <unordered_set>

struct Portal {
	Portal();
	~Portal();

	glm::vec2 position;
	glm::vec2 old_position;
	glm::vec2 normal;
	glm::vec2 speed;

	void move(glm::vec2 const &vec);
	void rotate(float const &to_rot);
	void update(float const elapsed);
	void move_to(glm::vec2 const &pos);
	void rotate_to(glm::vec2 const &target_normal);

	Scene::Transform *portal_transform;
	BoundingBox *boundingbox = nullptr;

	std::unordered_set<Scene::Object *> vicinity;

	void update_boundingbox();  // call this when position or normal is changed
	bool is_in_portal(const Scene::Object *obj);
	bool is_in_vicinity(const Scene::Transform *object_transform);
	bool should_teleport(const Scene::Object *obj);

    bool should_bounce(const Scene::Object *obj);
};
