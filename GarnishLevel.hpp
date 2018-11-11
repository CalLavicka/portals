#pragma once

#include "Level.hpp"

struct GarnishLevel : public Level {
    GarnishLevel(GameMode *gm, Scene::Object::ProgramInfo const &texture_program_info_,
                        Scene::Object::ProgramInfo const &depth_program_info_);

    Scene::Object::ProgramInfo texture_program_info;
    Scene::Object::ProgramInfo depth_program_info;

    Scene::Object *steak = nullptr;

    float score = 0.f;
    float time = 120.f;

	virtual void update(float elapsed) override;
};
