#pragma once

#include <array>
#include <glm/glm.hpp>

struct BoundingBox {
	// Object->normal is pointing upwards
	//     3 -------- 2
	//     |          |
	//     0 -------- 1
    
    BoundingBox() {};
    BoundingBox(float width_, float thickness_) : width(width_), thickness(thickness_) {};

	float width = 0.0f;      // length(0 -> 1)
	float thickness = 0.0f;  // length(0 -> 3)

	glm::vec2 p0;

	glm::vec2 parallel = glm::vec2(1.0f, 0.0f);  // parallel = normalize(0 -> 1)
    glm::vec2 normal = glm::vec2(0.0f, 1.0f);    // normal   = normalize(0 -> 3)

	// std::array< glm::vec2, 4 > get_corners();
    void update_origin(const glm::vec2 &position_);
    void update_origin(const glm::vec2 &position_, const glm::vec2 &normal_);
};