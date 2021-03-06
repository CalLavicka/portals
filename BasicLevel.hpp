#pragma once

#include "Level.hpp"
#include "Portal.hpp"

struct BasicLevel : public Level {
    BasicLevel(GameMode *gm,
            Scene::Object::ProgramInfo const &texture_program_info,
                        Scene::Object::ProgramInfo const &depth_program_info);
    virtual ~BasicLevel() {};

    Scene::Object::ProgramInfo texture_program_info;
    Scene::Object::ProgramInfo depth_program_info;

	virtual void update(float elapsed) override;
	virtual bool collision(Scene::Object *o1, Scene::Object *o2) override;
	virtual void fall_off(Scene::Object *o) override;
	virtual void render_pass() override;
    
    Scene::Object *create_food(std::string veg_name);

    void spawn_food();
	uint32_t fruit_hit = 0;
	float fruit_timer = 0.f;
    float messagetime;
};