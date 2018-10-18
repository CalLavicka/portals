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

	glm::vec2 parallel;  // parallel = normalize(0 -> 1)
    glm::vec2 normal;    // normal   = normalize(0 -> 3)

    void update_origin(const glm::vec2 &position_, const glm::vec2 &normal_);
};