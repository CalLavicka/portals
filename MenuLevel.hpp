#pragma once

#include "BasicLevel.hpp"

struct MenuLevel : public BasicLevel {
    MenuLevel(GameMode *gm,
            Scene::Object::ProgramInfo const &texture_program_info,
            Scene::Object::ProgramInfo const &depth_program_info);

    virtual void update(float elapsed) override;
	virtual void fall_off(Scene::Object *o) override {};
};
