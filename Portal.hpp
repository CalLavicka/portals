#pragma once

#include <glm/glm.hpp>

using namespace glm;

struct Portal {
	vec2 position;
	vec2 normal;
	
	void update_position(vec2 const &new_postion);
	void rotate(float const &to_rot);
};

