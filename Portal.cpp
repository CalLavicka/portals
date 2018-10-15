#include "Portal.hpp"
#include <glm/glm.hpp>

using namespace glm;

Portal::Portal() {
    this->position = vec2();
    this->normal = vec2(1.f, 0.f);
}

void Portal::move(vec2 const &vec) {
    this->position = this->position + vec;

    this->portal_transform->position = vec3(this->position, 0);
}

void Portal::rotate(float const &to_rot) {
    float cc = cos(to_rot);
    float ss = sin(to_rot);
    vec2 newnormal = vec2(normal.x * cc - normal.y * ss, normal.x * ss + normal.y * cc);
    normal = newnormal;

    this->portal_transform->rotation = angleAxis(atan2f(normal.y, normal.x), vec3(0,0,1));
}