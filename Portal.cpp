#include "Portal.hpp"
#include <glm/glm.hpp>

using namespace glm;

Portal::Portal() {
    this->position = vec2(0.0f, 1.0f);
    this->old_position = position;
    this->normal = vec2(0.0f, 1.0f);

    this->boundingbox = new BoundingBox(14.0f, 0.9f);
    this->update_boundingbox();  // set boundingbox position
}

Portal::~Portal() {
    delete this->boundingbox;
}

void Portal::move(vec2 const &vec) {
    this->position = this->position + vec;

    this->portal_transform->position = vec3(this->position, 0);
    
    this->update_boundingbox();  // update bbx when position changes
}

void Portal::rotate(float const &to_rot) {
    float cc = cos(to_rot);
    float ss = sin(to_rot);
    vec2 newnormal = vec2(normal.x * cc - normal.y * ss, normal.x * ss + normal.y * cc);
    normal = newnormal;

    //this->portal_transform->rotation = angleAxis(atan2f(normal.y, normal.x), vec3(0,0,1));
    this->portal_transform->rotation = angleAxis(atan2f(-normal.x, normal.y), vec3(0,0,1)) * angleAxis(0.3f, vec3(1,0,0));

    this->update_boundingbox();  // update bbx when orientation changes
}

void Portal::update(float const elapsed) {
    speed = (position - old_position) / elapsed;
    old_position = position;
}

void Portal::update_boundingbox() {
    this->boundingbox->update_origin(this->position, this->normal);
}

bool Portal::is_in_portal(const Scene::Object *obj) {
    Scene::Transform *object_transform = obj->transform;
    const BoundingBox *object_bbx = object_transform->boundingbox;
    std::vector< glm::vec2 > bbx_corners = object_bbx->get_corners();
    

    if(obj->portal_in != this) {
        return false;
    }

    for (auto &corner : bbx_corners) {
        // check every corner is in range
        float projected_length = glm::dot(corner - this->boundingbox->p0, this->boundingbox->parallel);
        if (projected_length < 0.0f || projected_length > this->boundingbox->width) return false;
    }
    

    // check the distance of center of object_bbx and the line pass through the center
    // of portal_center with direction "parallel"
    // pretty hard to describe in words...
    glm::vec2 object_center = glm::vec2(object_transform->position);
    //float parallel_dist = std::abs(glm::dot(object_center - this->position, this->boundingbox->parallel));
    
    //float center_dist = glm::distance(object_center, this->position);
    float norm_dot = glm::dot(object_center - this->position, this->normal);
    //float perpendicular_dist = std::sqrt(center_dist*center_dist - parallel_dist*parallel_dist);
    return norm_dot < 0.0f;  // return true when half way through

    /* return true upon touch */
    // float object_diag_len = std::sqrt(object_bbx->width*object_bbx->width + object_bbx->thickness*object_bbx->thickness);
    // return perpendicular_dist < 0.5f*(this->boundingbox->thickness + object_diag_len);
}

bool Portal::is_in_vicinity(const Scene::Transform *object_transform) {
    const BoundingBox *object_bbx = object_transform->boundingbox;
    std::vector< glm::vec2 > bbx_corners = object_bbx->get_corners();
    
    for (auto &corner : bbx_corners) {
        // check every corner is in range
        float projected_length = glm::dot(corner - this->boundingbox->p0, this->boundingbox->parallel);
        if (projected_length < 0.0f || projected_length > this->boundingbox->width) return false;
    }

    // Make sure its above portal
    glm::vec2 between = vec2(object_transform->position) - position;
    return glm::dot(between, normal) >= 0.0f;
}

bool Portal::should_teleport(const Scene::Object *obj) {
    vec2 mod_speed = obj->transform->speed - speed;
    return is_in_portal(obj) &&
           glm::dot(this->normal, mod_speed) < 0.0f;
}