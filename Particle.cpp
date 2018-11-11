#include "Particle.hpp"
#include <glm/glm.hpp>
#include <random>

using namespace glm;

Particle::Particle(glm::vec2 start_pos, std::string spice_name)  {
    std::random_device r;
	std::seed_seq seed{r(), r(), r(), r(), r(), r(), r(), r()};
	std::mt19937 rnd{seed};

    glm::vec2 shift = vec2(rnd(), rnd());
    this->position = start_pos+shift;
    this->velocity = vec2(0.0f, 0.0f);
    this->acceleration = vec2(0.5f, -9.5f);
    this->spice = spice_name;
}

Particle::~Particle() {
}

void Particle::update(float elapsed){

}
