#include "MenuLevel.hpp"

MenuLevel::MenuLevel(GameMode *_gm,
        Scene::Object::ProgramInfo const &texture_program_info,
        Scene::Object::ProgramInfo const &depth_program_info) :
        BasicLevel(_gm, texture_program_info, depth_program_info) {

}

void MenuLevel::update(float elapsed) {

}
