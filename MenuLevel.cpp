#include "MenuLevel.hpp"

using namespace glm;

MenuLevel::MenuLevel(GameMode *_gm, Scene::Object::ProgramInfo const &texture_program_info,
                            Scene::Object::ProgramInfo const &depth_program_info) : 
                                BasicLevel(_gm, texture_program_info, depth_program_info) {
    gm->players[0].move_to(vec2(-12.f, 0.f));
    gm->players[1].move_to(vec2(12.f, 0.f));
    
    gm->players[0].rotate_to(normalize(vec2(1.f, 10.f)));
    gm->players[1].rotate_to(normalize(vec2(-1.f, 10.f)));

    Scene::Object *obj = gm->scene->new_object(gm->scene->new_transform());
	obj->programs[Scene::Object::ProgramTypeDefault] = texture_program_info;
	obj->programs[Scene::Object::ProgramTypeDefault].textures[0] = *white_tex;

	obj->programs[Scene::Object::ProgramTypeShadow] = depth_program_info;

	MeshBuffer::Mesh const &mesh = vegetable_meshes->lookup("Broccoli");
	obj->programs[Scene::Object::ProgramTypeDefault].start = mesh.start;
	obj->programs[Scene::Object::ProgramTypeDefault].count = mesh.count;

	obj->programs[Scene::Object::ProgramTypeShadow].start = mesh.start;
	obj->programs[Scene::Object::ProgramTypeShadow].count = mesh.count;
	obj->transform->position = glm::vec3(12.f,5.f,0.f);
    obj->transform->speed = vec2(4.f, -20.f);
	obj->transform->rotation = glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f,0.f,0.f));
	obj->transform->boundingbox = new BoundingBox(2.0f, 2.0f);
	obj->transform->boundingbox->update_origin(obj->transform->position, glm::vec2(0.0f, 1.0f));
	gm->foods.push_back(obj);
}

void MenuLevel::update(float elapsed) {
    
}