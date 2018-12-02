#pragma once

#include "Level.hpp"

struct GarnishLevel : public Level {
    GarnishLevel(GameMode *gm,
            Scene::Object::ProgramInfo const &texture_program_info_,
            Scene::Object::ProgramInfo const &depth_program_info_);
    virtual ~GarnishLevel() {};


    Scene::Object::ProgramInfo texture_program_info;
    Scene::Object::ProgramInfo depth_program_info;

    Scene::Object *steak = nullptr;

	virtual void update(float elapsed) override;
	virtual void fall_off(Scene::Object *o) override;
    virtual void render_pass() override;


    void spawn_food();

    uint32_t message;
    float messagetime;
    float salt_time = 3.f;
    float total_time = 0.f;
    float time = 0.f;
    float score = 0.f;
    glm::vec3 pos = glm::vec3(10.0, 50.0f, 0.0f);
};
