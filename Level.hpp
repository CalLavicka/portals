#pragma once

// forward declaration of level
class Level;
#include "GameMode.hpp"
#include "Sound.hpp"

class Level {
public:
	GameMode *gm;
	Level(GameMode *gm_) : gm(gm_) {}
	virtual ~Level() { if (this->bgm) this->bgm->stop(); };
	virtual void update(float elapsed) {}
	virtual bool collision(Scene::Object *o1, Scene::Object *o2) { return false; }
	virtual void fall_off(Scene::Object *o) {}
	virtual void render_pass() {}

    std::shared_ptr< Sound::PlayingSample > bgm;
};