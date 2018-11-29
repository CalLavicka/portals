#pragma once

#include "Level.hpp"

struct GarnishLevel : public Level {
    GarnishLevel(GameMode *gm,
            Scene::Object::ProgramInfo const &texture_program_info_,
            Scene::Object::ProgramInfo const &bloom_program_info_,
            Scene::Object::ProgramInfo const &depth_program_info_);


    Scene::Object::ProgramInfo texture_program_info;
    Scene::Object::ProgramInfo depth_program_info;

	virtual void update(float elapsed) override;
	virtual void fall_off(Scene::Object *o) override;

    void spawn_food();

    float time = 0.f;
    float score = 0.f;
    glm::vec3 pos = glm::vec3(10.0, 50.0f, 0.0f);
};
