#include "GarnishLevel.hpp"
#include "BoundingBox.hpp"
#include "texture_program.hpp"
#include "depth_program.hpp"
#include "data_path.hpp"
#include "Load.hpp"
#include "compile_program.hpp"
#include "gl_errors.hpp"

#include "draw_text.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


Load< MeshBuffer > garnish_meshes(LoadTagDefault, [](){
return new MeshBuffer(data_path("steakLevels.pnct"));
});

Load< GLuint > garnish_meshes_for_texture_program(LoadTagDefault, [](){
return new GLuint(garnish_meshes->make_vao_for_program(texture_program->program));
});

Load< GLuint > garnish_meshes_for_depth_program(LoadTagDefault, [](){
return new GLuint(garnish_meshes->make_vao_for_program(depth_program->program));
});

GarnishLevel::GarnishLevel(GameMode *gm,
                    Scene::Object::ProgramInfo const &texture_program_info_,
                    Scene::Object::ProgramInfo const &depth_program_info_) : Level(gm),
                    texture_program_info(texture_program_info_),
                    depth_program_info(depth_program_info_) {

    texture_program_info.vao = *garnish_meshes_for_texture_program;
	{ // set up steak and plate
    //TODO model plate and kitchen scene
		steak = gm->scene->new_object(gm->scene->new_transform());
	    steak->programs[Scene::Object::ProgramTypeDefault] =
            texture_program_info;

		steak->programs[Scene::Object::ProgramTypeShadow] = depth_program_info;

		MeshBuffer::Mesh const &mesh = garnish_meshes->lookup("steak");
		steak->programs[Scene::Object::ProgramTypeDefault].start = mesh.start;
		steak->programs[Scene::Object::ProgramTypeDefault].count = mesh.count;

		steak->programs[Scene::Object::ProgramTypeShadow].start = mesh.start;
		steak->programs[Scene::Object::ProgramTypeShadow].count = mesh.count;
		steak->transform->position = glm::vec3(0.f,-40.f,0.f);
        steak->transform->scale = glm::vec3(3.0f,3.0f,3.0f);
		steak->transform->rotation = glm::angleAxis(glm::radians(90.f), glm::vec3(0.f,1.f,0.f));
		steak->transform->boundingbox = new BoundingBox(2.0f, 2.0f);
		steak->transform->boundingbox->update_origin(steak->transform->position, glm::vec2(0.0f, 1.0f));
	}
    messagetime = 3.f;

}

std::string spice_names[] = {"Chive2c", "Salt", "Pepper"};

void GarnishLevel::spawn_food() {

	Scene::Object *obj = gm->scene->new_object(gm->scene->new_transform());
    obj->lifespan = 8.0f;

    obj->programs[Scene::Object::ProgramTypeDefault] = texture_program_info;
	obj->programs[Scene::Object::ProgramTypeShadow] = depth_program_info;


	MeshBuffer::Mesh const &mesh = garnish_meshes->lookup(spice_names[message]);
	obj->programs[Scene::Object::ProgramTypeDefault].start = mesh.start;
	obj->programs[Scene::Object::ProgramTypeDefault].count = mesh.count;

	obj->programs[Scene::Object::ProgramTypeShadow].start = mesh.start;
	obj->programs[Scene::Object::ProgramTypeShadow].count = mesh.count;
	obj->transform->position = pos + glm::vec3(gm->random_gen() % 10,0.f,0.f);
	obj->transform->rotation = glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f,0.f,0.f));
	obj->transform->boundingbox = new BoundingBox(2.0f, 2.0f);
	obj->transform->boundingbox->update_origin(obj->transform->position, glm::vec2(0.0f, 1.0f));
	gm->foods.push_back(obj);
}

void GarnishLevel::update(float elapsed) {
    total_time += elapsed;
    messagetime -= elapsed;
    time-=elapsed;
    if(time<=0.f){
        time+=0.6f;
        spawn_food();
    }
    if(message==0){
        pos.x = 65.0f*(sinf(total_time*0.1f));
    }else if(message==1){
        salt_time -= elapsed;
        if(salt_time<=0.f){
            salt_time = 3.f;
            pos.x = gm->random_gen()%150 - 75.f;
        }
    }else{
        pos.x = gm->random_gen()%150 - 75.f;
    }

    for(auto iter = gm->foods.begin(); iter != gm->foods.end();) {
		Scene::Transform *food_transform = (*iter)->transform;
        if(food_transform->position.y < -38.f
            && food_transform->position.x > steak->transform->position.x-10.f
            &&food_transform->position.x < steak->transform->position.x+10.f) {
                (*iter)->lifespan = 0.0f;
                gm->scores[gm->level]+=10;
                if(message==0 && gm->scores[gm->level]==200){
                    message++;
                    messagetime=3.f;
                }else if(message==1 && gm->scores[gm->level]==300){
                    message++;
                    messagetime=3.f;
                }else if(message==2 &&gm->scores[gm->level]==400){
                    gm->show_win();
                }
		}

        (*iter)->lifespan -= elapsed;
		if ((*iter)->lifespan < 0.f) {
			gm->scene->delete_transform(food_transform);
			gm->scene->delete_object(*iter);
			auto temp = iter;
			++iter;
			gm->foods.erase(temp);
			continue;
		}
		++iter;
    }
}


void GarnishLevel::fall_off(Scene::Object *o) {
    if(gm->scores[gm->level] >= 10) gm->scores[gm->level] -= 10;
    else if(gm->scores[gm->level] > 0) gm->scores[gm->level] = 0;
	if(gm->scores[gm->level] == 0) {
		gm->show_lose();
	}
}

void GarnishLevel::render_pass() {

	glDisable(GL_DEPTH_TEST);

	if (messagetime > 0.f) {
        std::string text;
        if(message==0) text= "TIME FOR SOME CHIVES";
        if(message==1) text= "TIME FOR SOME SALT";
        if(message==2) text= "TIME FOR SOME PEPPER";

        float height = 0.1f;

        float ypos = -0.3f;
        float width = text_width(text, height);
        draw_text(text, glm::vec2( -width/2.f, ypos), height,
            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

	glEnable(GL_DEPTH_TEST);
}
