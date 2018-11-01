#pragma once

#include "Level.hpp"

struct OvenLevel : public Level {
    OvenLevel(GameMode *gm, Scene::Object::ProgramInfo const &texture_program_info_,
                        Scene::Object::ProgramInfo const &depth_program_info_);

    Scene::Object::ProgramInfo texture_program_info;
    Scene::Object::ProgramInfo depth_program_info;

    Scene::Object *steak = nullptr;

    float heat = 0.f;
    float time = 120.f;
    float score_timer = 1.f;

    float top = 20.f;
    float bottom = -20.f;

	virtual void update(float elapsed) override;
	virtual void fall_off(Scene::Object *o) override;
	virtual void render_pass() override;
};