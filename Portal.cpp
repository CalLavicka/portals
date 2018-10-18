#include "Portal.hpp"
#include <glm/glm.hpp>

using namespace glm;

Portal::Portal() {
    this->position = vec2(0.0f, 1.0f);
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
    printf("X: %f, Y: %f, Z: %f\n", portal_transform->position.x,
                                    portal_transform->position.y, 
                                    portal_transform->position.z);
}

void Portal::rotate(float const &to_rot) {
    float cc = cos(to_rot);
    float ss = sin(to_rot);
    vec2 newnormal = vec2(normal.x * cc - normal.y * ss, normal.x * ss + normal.y * cc);
    normal = newnormal;

    this->portal_transform->rotation = angleAxis(atan2f(normal.y, normal.x), vec3(0,0,1));
}

void Portal::update_boundingbox() {
    this->boundingbox->update_origin(this->position, this->normal);
}

bool Portal::is_in_portal(const BoundingBox *object_bbx) {
    // bool is_touch = false;
    // return is_touch;
    return object_bbx->p0.y < 1.0f;  // dummy implementation
}

bool Portal::should_teleport(const Scene::Transform *object_transform) {
    return is_in_portal(object_transform->boundingbox);
    // return is_in_portal(object_transform->boundingbox) &&
    //        glm::dot(this->normal, object_transform->speed) < 0.0f;
}