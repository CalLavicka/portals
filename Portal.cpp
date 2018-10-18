#include "Portal.hpp"
#include <glm/glm.hpp>

using namespace glm;

Portal::Portal() {
    this->position = vec2();
    this->normal = vec2(0.0f, 1.0f);

    this->boundingbox = new BoundingBox(12.0f, 0.9f);
    this->update_boundingbox();  // set boundingbox position
}

Portal::~Portal() {
    delete this->boundingbox;
}

void Portal::move(vec2 const &vec) {
    this->position = this->position + vec;

    this->portal_transform->position = vec3(this->position, 0);
    //printf("X: %f, Y: %f, Z: %f\n", portal_transform->position.x,
    //portal_transform->position.y, portal_transform->position.z);
}

void Portal::rotate(float const &to_rot) {
    float cc = cos(to_rot);
    float ss = sin(to_rot);
    vec2 newnormal = vec2(normal.x * cc - normal.y * ss, normal.x * ss + normal.y * cc);
    normal = newnormal;

    //this->portal_transform->rotation = angleAxis(atan2f(normal.y, normal.x), vec3(0,0,1));
    this->portal_transform->rotation = angleAxis(atan2f(-normal.x, normal.y), vec3(0,0,1)) * angleAxis(0.3f, vec3(1,0,0));
}

void Portal::update_boundingbox() {
    this->boundingbox->update_origin(this->position, this->normal);
}