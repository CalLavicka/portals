#include "Portal.hpp"
#include <glm/glm.hpp>

void Portal::update_position(vec2 const &new_position) {
    this->position = new_position;
}

void Portal::rotate(float const &to_rot) {
    float cc = cos(to_rot);
    float ss = sin(to_rot);
    vec2 newnormal = vec2(normal.x * cc - normal.y * ss, normal.x * ss + normal.y * cc);
    normal = newnormal;
}