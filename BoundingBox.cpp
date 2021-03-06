#include "BoundingBox.hpp"

#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

std::vector< glm::vec2 > BoundingBox::get_corners() const {
    return std::vector< glm::vec2 > {
        this->p0,
        this->p0 + this->parallel*this->width,
        this->p0 + this->parallel*this->width + this->normal*this->thickness,
        this->p0 + this->normal*this->thickness
    };
}

void BoundingBox::update_origin(const glm::vec2 &position_, bool normal_changed) {
    float dw = this->width / 2.0f;
    float dt = this->thickness / 2.0f;
    if (normal_changed) {  // updata parallel only when normal is changed
        glm::mat4 rotation = glm::rotate(glm::mat4(1.f), -90.0f * float(M_PI) / 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));
        this->parallel = glm::vec2(rotation * glm::vec4(this->normal, 0.0f, 1.0f));
    }

    this->p0 = position_ - this->normal * dt - this->parallel * dw;
}

void BoundingBox::update_origin(const glm::vec2 &position_, const glm::vec2 &normal_) {
    this->normal = normal_;
    update_origin(position_, true);
}