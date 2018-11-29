#include "GameMode.hpp"

#include "MenuMode.hpp"
#include "Load.hpp"
#include "MeshBuffer.hpp"
#include "Save.hpp"
#include "Scene.hpp"
#include "gl_errors.hpp" //helper for dumpping OpenGL error messages
#include "check_fb.hpp" //helper for checking currently bound OpenGL framebuffer
#include "read_chunk.hpp" //helper for reading a vector of structures from a file
#include "data_path.hpp" //helper to get paths relative to executable
#include "compile_program.hpp" //helper to compile opengl shader programs
#include "draw_text.hpp" //helper to... um.. draw text
#include "load_save_png.hpp"
#include "texture_program.hpp"
#include "depth_program.hpp"

#include "BasicLevel.hpp"
#include "GarnishLevel.hpp"
#include "OvenLevel.hpp"
#include "MenuLevel.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

#include <iostream>
#include <fstream>
#include <map>
#include <cstddef>
#include <random>

#define NUM_CLIPPING_VERTS 20

using namespace glm;

Load< MeshBuffer > meshes(LoadTagDefault, [](){
	return new MeshBuffer(data_path("vignette.pnct"));
});

Load< GLuint > meshes_for_texture_program(LoadTagDefault, [](){
	return new GLuint(meshes->make_vao_for_program(texture_program->program));
});

Load< GLuint > meshes_for_depth_program(LoadTagDefault, [](){
	return new GLuint(meshes->make_vao_for_program(depth_program->program));
});

Load< MeshBuffer > vegetable_meshes(LoadTagDefault, [](){
	return new MeshBuffer(data_path("vegetables.pnct"));
});

Load< GLuint > vegetable_meshes_for_texture_program(LoadTagDefault, [](){
	return new GLuint(vegetable_meshes->make_vao_for_program(texture_program->program));
});


Load< GLuint > vegetable_meshes_for_depth_program(LoadTagDefault, [](){
	return new GLuint(vegetable_meshes->make_vao_for_program(depth_program->program));
});

//used for fullscreen passes:
Load< GLuint > empty_vao(LoadTagDefault, [](){
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindVertexArray(0);
	return new GLuint(vao);
});

Load< GLuint > bloom_program(LoadTagDefault, [](){
        GLuint program = compile_program(
                //this draws a triangle that covers the entire screen:
                "#version 330\n"
                "void main() {\n"
                "	gl_Position = vec4(4 * (gl_VertexID & 1) - 1,  2 * (gl_VertexID & 2) - 1, 0.0, 1.0);\n"
                "}\n"
                ,
                //NOTE on reading screen texture:
                //texelFetch() gives direct pixel access with integer coordinates, but accessing out-of-bounds pixel is undefined:
                //	vec4 color = texelFetch(tex, ivec2(gl_FragCoord.xy), 0);
                //texture() requires using [0,1] coordinates, but handles out-of-bounds more gracefully (using wrap settings of underlying texture):
                //	vec4 color = texture(tex, gl_FragCoord.xy / textureSize(tex,0));

                "#version 330\n"
                "uniform sampler2D tex;\n"
               "out vec4 fragColor;\n"
                "void main() {\n"
                "	vec2 at = (gl_FragCoord.xy - 0.5 * textureSize(tex, 0)) / textureSize(tex, 0).y;\n"

                //pick a vector to move in for blur using function inspired by:
                //https://stackoverflow.com/questions/12964279/whats-the-origin-of-this-glsl-rand-one-liner
                "float distance = 8.0f;\n"
                "	vec2 ofs = distance * normalize(vec2(\n"
                "		fract(dot(gl_FragCoord.xy ,vec2(12.9898,78.233))),\n"
                "		fract(dot(gl_FragCoord.xy ,vec2(96.3869,-27.5796)))\n"
                "	));\n"
                //referenced guidance on Piazza:
                //https://piazza.com/class/jlfaf4xoz4y665?cid=65
                //I did read this too though:
                //https://learnopengl.com/Advanced-Lighting/Bloom
                "vec4 center = texture(tex, gl_FragCoord.xy/textureSize(tex, 0));\n"

                "vec4 blur = center;\n"
                "float centerB = dot(blur.rgb, vec3(1.0, 1.0, 1.0));\n"
                "float brightness;\n"
                "float threshold = 1.0;\n"
                "float influence = .2;\n"

                "vec4 n1 = texture(tex, (gl_FragCoord.xy+vec2(ofs.x, ofs.y))\n"
                "/textureSize(tex,0));\n"
                "brightness = dot(n1.rgb, vec3(1.0, 1.0, 1.0));\n"
                "if(brightness<3.0 && brightness>threshold)\n"
                "blur += influence*n1;\n"

                "n1 = texture(tex, (gl_FragCoord.xy+vec2(-ofs.y, ofs.x))\n"
                "/textureSize(tex,0));\n"
                "brightness = dot(n1.rgb, vec3(1.0, 1.0, 1.0));\n"
                "if(brightness<3.0 && brightness>threshold)\n"
                "blur += influence*n1;\n"

                "n1 = texture(tex, (gl_FragCoord.xy+vec2(-ofs.x, -ofs.y))\n"
                "/textureSize(tex,0));\n"
                "brightness = dot(n1.rgb, vec3(1.0, 1.0, 1.0));\n"
                "if(brightness<3.0 && brightness>threshold) \n"
                "blur += influence*n1;\n"

                "n1 = texture(tex, (gl_FragCoord.xy+vec2(ofs.y, -ofs.x))\n"
                "/textureSize(tex,0));\n"
                "brightness = dot(n1.rgb, vec3(1.0, 1.0, 1.0));\n"
                "if(brightness<3.0 && brightness>threshold)\n"
                "blur += influence*n1;\n"

                "	fragColor = vec4(blur.rgb, 1.0);\n"
                "}\n"
                );

        glUseProgram(program);

        glUniform1i(glGetUniformLocation(program, "tex"), 0);

        glUseProgram(0);

        return new GLuint(program);
});


Load< GLuint > blur_program(LoadTagDefault, [](){
	GLuint program = compile_program(
		//this draws a triangle that covers the entire screen:
		"#version 330\n"
		"void main() {\n"
		"	gl_Position = vec4(4 * (gl_VertexID & 1) - 1,  2 * (gl_VertexID & 2) - 1, 0.0, 1.0);\n"
		"}\n"
		,
		//NOTE on reading screen texture:
		//texelFetch() gives direct pixel access with integer coordinates, but accessing out-of-bounds pixel is undefined:
		//	vec4 color = texelFetch(tex, ivec2(gl_FragCoord.xy), 0);
		//texture() requires using [0,1] coordinates, but handles out-of-bounds more gracefully (using wrap settings of underlying texture):
		//	vec4 color = texture(tex, gl_FragCoord.xy / textureSize(tex,0));

		"#version 330\n"
		"uniform sampler2D color_tex;\n"
        "uniform sampler2D bloom_tex;\n"
		"out vec4 fragColor;\n"
		"void main() {\n"
		"	vec2 at = (gl_FragCoord.xy - 0.5 * textureSize(bloom_tex, 0)) / textureSize(bloom_tex, 0).y;\n"
		//make blur amount more near the edges and less in the middle:
		"	float amt = 10;\n"//(0.01 * textureSize(bloom_tex,0).y) * max(0.0,(length(at) - 0.3)/0.2);\n"
		//pick a vector to move in for blur using function inspired by:
		//https://stackoverflow.com/questions/12964279/whats-the-origin-of-this-glsl-rand-one-liner
		"	vec2 ofs = amt * normalize(vec2(\n"
		"		fract(dot(gl_FragCoord.xy ,vec2(12.9898,78.233))),\n"
		"		fract(dot(gl_FragCoord.xy ,vec2(96.3869,-27.5796)))\n"
		"	));\n"
		//do a four-pixel average to blur:
		"	vec4 blur =\n"
		"		+ 0.25 * texture(bloom_tex, (gl_FragCoord.xy + vec2(ofs.x,ofs.y)) / textureSize(bloom_tex, 0))\n"
		"		+ 0.25 * texture(bloom_tex, (gl_FragCoord.xy + vec2(-ofs.y,ofs.x)) / textureSize(bloom_tex, 0))\n"
		"		+ 0.25 * texture(bloom_tex, (gl_FragCoord.xy + vec2(-ofs.x,-ofs.y)) / textureSize(bloom_tex, 0))\n"
		"		+ 0.25 * texture(bloom_tex, (gl_FragCoord.xy + vec2(ofs.y,-ofs.x)) / textureSize(bloom_tex, 0))\n"
		"	;\n"
        "   vec4 fragColor1 = texture(color_tex, (gl_FragCoord.xy) /     textureSize(color_tex, 0));\n"
		"	fragColor = fragColor1 + vec4(blur.rgb, 1.0);\n" //blur;\n"
//        "   fragColor = texelFetch(bloom_tex, ivec2(gl_FragCoord.xy), 0);\n"
		"}\n"
	);

	glUseProgram(program);


	glUniform1i(glGetUniformLocation(program, "color_tex"), 0);
	glUniform1i(glGetUniformLocation(program, "bloom_tex"), 1);

    glUseProgram(0);

	return new GLuint(program);
});

Load< GLuint > portal_depth_program(LoadTagDefault, [](){
	GLuint program = compile_program(
		//this draws a triangle that covers the entire screen:
		"#version 330\n"
		"uniform vec2 portalNorm;\n"
		"uniform mat4 mv;\n"
		"uniform mat4 cam_scale;\n"
		"void main() {\n"
		//"	gl_Position = vec4(4 * (gl_VertexID & 1) - 1,  2 * (gl_VertexID & 2) - 1, -1.0, 1.0);\n"
		"	if (gl_VertexID < 4) {\n" // Clipping plane 1 (through portal)
		"		vec4 pt = vec4(100000 * (2 * (gl_VertexID & 1) - 1), 0.0, 100000 * ((gl_VertexID & 2) - 1), 1.0);\n"
		"		gl_Position = cam_scale * mv * pt;\n"
		"	} else {\n"
		"		int idx = gl_VertexID - 4;\n"
		"		vec2 pt = vec2(mv * vec4(0.0, 0.0, 0.0, 1.0)) - portalNorm * 2.5;\n"
		"   	vec2 par = vec2(-portalNorm.y, portalNorm.x) * 1000.0;\n"
		"   	vec2 norm = portalNorm * 1000.0;\n"
		"   	pt = pt - norm * (idx & 1) + par * ((idx & 2) - 1);\n"
		"		gl_Position = cam_scale * vec4(pt, -1.0, 1.0);\n"
		"		gl_Position.z = -1.0;\n"
		"	}\n"
		"}\n"
		,

		"#version 330\n"
		"out vec4 fragColor;\n"
		"void main() {\n"
		"	fragColor = vec4(0.0, 1.0, 0.0, 0.0);\n"
		"}\n"
	);

	return new GLuint(program);
});


GLuint load_texture(std::string const &filename) {
	glm::uvec2 size;
	std::vector< glm::u8vec4 > data;
	load_png(filename, &size, &data, LowerLeftOrigin);

	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	GL_ERRORS();

	return tex;
}

Load< GLuint > wood_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/wood.png")));
});

Load< GLuint > marble_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/marble.png")));
});

Load< GLuint > white_tex(LoadTagDefault, [](){
	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glm::u8vec4 white(0xff, 0xff, 0xff, 0xff);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, glm::value_ptr(white));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	return new GLuint(tex);
});



Scene::Transform *p0_trans = nullptr;
Scene::Transform *p1_trans = nullptr;

void GameMode::load_scene() {
	{
		// Initialize random gen
		std::random_device r;
		std::seed_seq seed{r(), r(), r(), r(), r(), r(), r(), r()};
		std::mt19937 rnd{seed};
		random_gen = rnd;
	}

	if (scene != nullptr) {
		delete scene;
		foods.clear();
		pots.clear();
	}

	Scene *ret = new Scene();

	//pre-build some program info (material) blocks to assign to each object:
	Scene::Object::ProgramInfo texture_program_info;
	texture_program_info.program = texture_program->program;
	texture_program_info.vao = *meshes_for_texture_program;
	texture_program_info.mvp_mat4  = texture_program->object_to_clip_mat4;
	texture_program_info.mv_mat4x3 = texture_program->object_to_light_mat4x3;
	texture_program_info.itmv_mat3 = texture_program->normal_to_light_mat3;
    texture_program_info.set_uniforms = [](){
        glUniform1f(texture_program->glow_amt_float, 0.0f);
    };

	texture_program_info.textures[0] = *white_tex;

    Scene::Object::ProgramInfo portal_program_info = texture_program_info;
    portal_program_info.set_uniforms = [](){
        glUniform1f(texture_program->glow_amt_float, 1.0f);
    };

	Scene::Object::ProgramInfo depth_program_info;
	depth_program_info.program = depth_program->program;
	depth_program_info.vao = *meshes_for_depth_program;
	depth_program_info.mvp_mat4  = depth_program->object_to_clip_mat4;

	// Adjust for veges
	texture_program_info.vao = *vegetable_meshes_for_texture_program;
    portal_program_info.vao = *vegetable_meshes_for_texture_program;
	depth_program_info.vao = *vegetable_meshes_for_depth_program;

	// Add in portal
	p0_trans = ret->new_transform();
	p1_trans = ret->new_transform();

	{ // Portal 1
		Scene::Object *obj = ret->new_object(p0_trans);
		obj->programs[Scene::Object::ProgramTypeDefault] = portal_program_info;
		obj->programs[Scene::Object::ProgramTypeShadow] = depth_program_info;

		MeshBuffer::Mesh const &mesh = vegetable_meshes->lookup("Portal1");
		obj->programs[Scene::Object::ProgramTypeDefault].start = mesh.start;
		obj->programs[Scene::Object::ProgramTypeDefault].count = mesh.count;

		obj->programs[Scene::Object::ProgramTypeShadow].start = mesh.start;
		obj->programs[Scene::Object::ProgramTypeShadow].count = mesh.count;
	}

	{ // Portal 2
		Scene::Object *obj = ret->new_object(p1_trans);
		obj->programs[Scene::Object::ProgramTypeDefault] = portal_program_info;
		obj->programs[Scene::Object::ProgramTypeShadow] = depth_program_info;

		MeshBuffer::Mesh const &mesh = vegetable_meshes->lookup("Portal2");
		obj->programs[Scene::Object::ProgramTypeDefault].start = mesh.start;
		obj->programs[Scene::Object::ProgramTypeDefault].count = mesh.count;

		obj->programs[Scene::Object::ProgramTypeShadow].start = mesh.start;
		obj->programs[Scene::Object::ProgramTypeShadow].count = mesh.count;
	}

	Scene::Transform *cam_trans = ret->new_transform();
	camera = ret->new_camera(cam_trans);
	camera->is_perspective = false;
	camera->ortho_scale = 50.f;
	camera->near = 1.f;

	cam_trans->position = glm::vec3(0,0,25);
	cam_trans->rotation = glm::angleAxis(glm::radians(0.f), glm::vec3(1.0f, 0.0f, 0.0f));


	scene = ret;

	switch(level) {
        case 0:
		current_level = new BasicLevel(this, texture_program_info,
                depth_program_info);
		scores[0] = 50;
		break;
	case 1:
		current_level = new OvenLevel(this, texture_program_info,
                depth_program_info);
		scores[1] = 0;
		break;
    case 2:
        current_level = new GarnishLevel(this, texture_program_info,
                depth_program_info);
        scores[2] = 0;
        break;
	default:
		current_level = new MenuLevel(this, texture_program_info,
                depth_program_info);
		show_level_select();
		break;
	}

	players[0].portal_transform = p0_trans;
	players[1].portal_transform = p1_trans;
}

GameMode::GameMode() {
	//load_scene();

	//SDL_SetRelativeMouseMode(SDL_TRUE);
}

GameMode::~GameMode() {
}

bool GameMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	//ignore any keys that are the result of automatic key repeat:
	if (evt.type == SDL_KEYDOWN && evt.key.repeat) {
		return false;
	}

	if(evt.type == SDL_KEYDOWN){
		//TODO add specification for which save state
		if(evt.key.keysym.scancode == SDL_SCANCODE_SPACE){
			show_pause_menu();
		}else if(evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE){

			SDL_SetRelativeMouseMode(SDL_FALSE);
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_P) {

			SDL_SetRelativeMouseMode(SDL_TRUE);
		}
	}

	return false;
}

bool GameMode::handle_mouse_event(ManyMouseEvent const &event, glm::uvec2 const &window_size) {
	if (event.device >= 2) {
		return false;
	}

	// printf("TYPE: %d, VALUE: %d, ITEM: %d, DEVICE: %d\n", event.type, event.value, event.item, event.device);
	Portal &portal = players[event.device];
	float &rot_speed = rot_speeds[event.device];
	float sensitivity = sensitivities[event.device];
	if (event.type == MANYMOUSE_EVENT_RELMOTION) {
		if (event.item == 0) {
			portal.move(glm::vec2(sensitivity * event.value / window_size.x, 0));
			return true;
		} else if (event.item == 1) {
			portal.move(glm::vec2(0, -sensitivity * event.value / window_size.y));
			return true;
		}
	}else if (event.type == MANYMOUSE_EVENT_BUTTON) {
		if (event.value == 0) {
			rot_speed = 0;
			return true;
		} else if (event.item == 0) {
			rot_speed = 3;
			return true;
		} else if (event.item == 1) {
			rot_speed = -3;
			return true;
		}
	}
	return false;
}

void GameMode::update(float elapsed) {

	{ // Update portals
		players[0].update(elapsed);
		players[1].update(elapsed);
	}

	current_level->update(elapsed);

	players[0].rotate(elapsed * rot_speeds[0]);
	players[1].rotate(elapsed * rot_speeds[1]);

	auto update_vicinity = [](Scene::Object *obj, Portal &p, Portal &op) {
		if (obj->portal_in == &op) {
			obj->portal_in->vicinity.erase(obj);
		}
		obj->portal_in = &p;
		p.vicinity.insert(obj);
	};

	for(auto iter = foods.begin(); iter != foods.end();) {
		Scene::Transform *food_transform = (*iter)->transform;

		{  // teleport / see if in a portal
			float threshold = std::max(players[0].boundingbox->width, players[0].boundingbox->thickness) +
							std::max(food_transform->boundingbox->width, food_transform->boundingbox->thickness);
			bool updated = false;
			if (glm::distance(players[0].portal_transform->position, food_transform->position) < threshold) {
				if (players[0].should_teleport(*iter)) {
					teleport(food_transform, 1);  // GameMode::teleport(object, destination_portal)
					update_vicinity(*iter, players[1], players[0]);
					updated = true;
				} else if (players[0].is_in_vicinity(food_transform)) {
					update_vicinity(*iter, players[0], players[1]);
					updated = true;
                } else if (players[0].should_bounce(food_transform)) {
                    food_transform->speed -= 2*glm::dot(food_transform->speed,
                                        players[0].normal)*players[0].normal;
                    food_transform->speed *= 0.8f;
                }
			}
			if (!updated && glm::distance(players[1].portal_transform->position, food_transform->position) < threshold) {
				if (players[1].should_teleport(*iter)) {
					teleport(food_transform, 0);  // GameMode::teleport(object, destination_portal)
					update_vicinity(*iter, players[0], players[1]);
					updated = true;
				} else if (players[1].is_in_vicinity(food_transform)) {
					update_vicinity(*iter, players[1], players[0]);
					updated = true;
                } else if (players[1].should_bounce(food_transform)) {
                    //credit to this for how to physics
                    //https://gamedev.stackexchange.com/questions/23672/determine-resulting-angle-of-wall-collision/23674
                    food_transform->speed -= 2*glm::dot(food_transform->speed,
                                        players[0].normal)*players[0].normal;
                    food_transform->speed *= 0.8f;

                }
			}
			if (!updated) {
				if((*iter)->portal_in != nullptr) {
					(*iter)->portal_in->vicinity.erase(*iter);
					(*iter)->portal_in = nullptr;
				}
			}
		}

		{  // update vegetbale speed, position, and boundingbox
			float g = -9.81f;
			food_transform->speed.y += g * elapsed;
			food_transform->speed.y = std::max(-200.0f, food_transform->speed.y);  // speed limit on cube
			food_transform->position.x += food_transform->speed.x * elapsed;
			food_transform->position.y += food_transform->speed.y * elapsed;
			food_transform->boundingbox->update_origin(food_transform->position);

			if (food_transform->position.y >= 50.f && food_transform->speed.y > 0.f) {
				if (food_transform->speed.x > 0.f) {
					food_transform->speed.x = glm::max(food_transform->speed.x - food_transform->speed.y, 0.f);
				}else {
					food_transform->speed.x = glm::min(food_transform->speed.x + food_transform->speed.y, 0.f);
				}
                food_transform->speed.y *= -0.2f;
			}

			if (food_transform->position.x >= 70.f && food_transform->speed.x > 0.f) {
				food_transform->speed.x = -food_transform->speed.x / 2.f;
			} else if (food_transform->position.x <= -70.f && food_transform->speed.x < 0.f) {
				food_transform->speed.x = -food_transform->speed.x / 2.f;
			}
		}


		bool collided = false;
		for(Scene::Object * pot : pots) {
			// TODO: Check for collision with pot with bounding boxes
			if(food_transform->position.y < -38.f && food_transform->position.x > pot->transform->position.x - 10.f &&
					food_transform->position.x < pot->transform->position.x + 10.f) {

				collided = current_level->collision(*iter, pot);

				if (collided) break;
			}
		}
		if(collided) {
			scene->delete_transform(food_transform);
			scene->delete_object(*iter);
			auto temp = iter;
			++iter;
			foods.erase(temp);
			continue;
		}

		if (food_transform->position.y < -60.f) {
			// OFF THE TABLE
			printf("Food fell off...\n");
			current_level->fall_off(*iter);
			scene->delete_transform(food_transform);
			scene->delete_object(*iter);
			auto temp = iter;
			++iter;
			foods.erase(temp);
			continue;
		}

		++iter;
	}
}

//GameMode will render to some offscreen framebuffer(s).
//This code allocates and resizes them as needed:
struct Framebuffers {
	glm::uvec2 size = glm::uvec2(0,0); //remember the size of the framebuffer

	//This framebuffer is used for fullscreen effects:
	GLuint color_tex = 0;
	GLuint depth_rb = 0;
	GLuint fb = 0;

    //This framebuffer is used for bloom effects:
	GLuint bloom_color_tex = 0;
	GLuint bloom_fb = 0;

	//This framebuffer is used for shadow maps:
	glm::uvec2 shadow_size = glm::uvec2(0,0);
	GLuint shadow_color_tex = 0; //DEBUG
	GLuint shadow_depth_tex = 0;
	GLuint shadow_fb = 0;

	void allocate(glm::uvec2 const &new_size, glm::uvec2 const &new_shadow_size) {
		//allocate full-screen framebuffer:
		if (size != new_size) {
			size = new_size;

			if (color_tex == 0) glGenTextures(1, &color_tex);
			glBindTexture(GL_TEXTURE_2D, color_tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glBindTexture(GL_TEXTURE_2D, 0);

            if (bloom_color_tex == 0) glGenTextures(1, &bloom_color_tex);
			glBindTexture(GL_TEXTURE_2D, bloom_color_tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glBindTexture(GL_TEXTURE_2D, 0);

			if (depth_rb == 0) glGenRenderbuffers(1, &depth_rb);
			glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size.x, size.y);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			if (fb == 0) glGenFramebuffers(1, &fb);
			glBindFramebuffer(GL_FRAMEBUFFER, fb);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_tex, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bloom_color_tex, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);
            GLenum bufs[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
            glDrawBuffers(2, bufs);
			check_fb();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

            if (bloom_fb == 0) glGenFramebuffers(1, &bloom_fb);
			glBindFramebuffer(GL_FRAMEBUFFER, bloom_fb);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloom_color_tex, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);
			check_fb();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			GL_ERRORS();
		}

		//allocate shadow map framebuffer:
		if (shadow_size != new_shadow_size) {
			shadow_size = new_shadow_size;

			if (shadow_color_tex == 0) glGenTextures(1, &shadow_color_tex);
			glBindTexture(GL_TEXTURE_2D, shadow_color_tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, shadow_size.x, shadow_size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glBindTexture(GL_TEXTURE_2D, 0);


			if (shadow_depth_tex == 0) glGenTextures(1, &shadow_depth_tex);
			glBindTexture(GL_TEXTURE_2D, shadow_depth_tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadow_size.x, shadow_size.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glBindTexture(GL_TEXTURE_2D, 0);

			if (shadow_fb == 0) glGenFramebuffers(1, &shadow_fb);
			glBindFramebuffer(GL_FRAMEBUFFER, shadow_fb);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadow_color_tex, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_depth_tex, 0);
			check_fb();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			GL_ERRORS();
		}
	}
} fbs;

void GameMode::draw(glm::uvec2 const &drawable_size) {
	fbs.allocate(drawable_size, glm::uvec2(512, 512));
	camera->aspect = drawable_size.x / float(drawable_size.y);

	glViewport(0,0,drawable_size.x, drawable_size.y);

	glBindFramebuffer(GL_FRAMEBUFFER, fbs.fb);

    GLfloat black[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    glClearBufferfv(GL_COLOR, 0, black);
    glClearBufferfv(GL_COLOR, 1, black);
	glClear(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Draw once for ambient light
	glUseProgram(texture_program->program);

	//don't use distant directional light at all (color == 0):
	glUniform3fv(texture_program->sun_color_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)));
	glUniform3fv(texture_program->sun_direction_vec3, 1, glm::value_ptr(glm::normalize(glm::vec3(0.0f, 0.0f,-1.0f))));
	//little bit of ambient light:
	glUniform3fv(texture_program->sky_color_vec3, 1, glm::value_ptr(glm::vec3(1.f,1.f,1.f)));
	glUniform3fv(texture_program->sky_direction_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 1.0f)));

	glUniform3fv(texture_program->spot_color_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)));
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);

	// Draw non-portalled things
    scene->draw(camera, Scene::Object::ProgramTypeDefault, nullptr);

    auto draw_portal = [this](Portal &p) {
		glUseProgram(*portal_depth_program);
		glBindVertexArray(*empty_vao);
		//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glClear(GL_DEPTH_BUFFER_BIT);

		static GLuint portal_norm = glGetUniformLocation(*portal_depth_program, "portalNorm");
		static GLuint mv_mat4 = glGetUniformLocation(*portal_depth_program, "mv");
		static GLuint cam_scale_mat4 = glGetUniformLocation(*portal_depth_program, "cam_scale");

		glm::mat4 mv = p.portal_transform->make_local_to_world();

		glm::mat4 cam_scale = camera->make_projection() * camera->transform->make_world_to_local();

		//glm::vec2 pt = glm::vec2(mvp * glm::vec4(players[0].position, 0, 1));

		glUniformMatrix4fv(mv_mat4, 1, GL_FALSE, glm::value_ptr(mv));
		glUniformMatrix4fv(cam_scale_mat4, 1, GL_FALSE, glm::value_ptr(cam_scale));
		glUniform2f(portal_norm, p.normal.x, p.normal.y);
		//printf("%f, %f\n", p.normal.x, p.normal.y);

		// Draw portal blocker
		glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


		// Draw portalled things
		scene->draw(camera, Scene::Object::ProgramTypeDefault, &p);
	};

	{ // Move everthing from portal 1 to portal 0, then render from portal 0
		for(Scene::Object * obj : players[1].vicinity) {
			teleport(obj->transform, 0, false);
			obj->portal_in = &players[0];
		}

		draw_portal(players[0]);
	}

	{ // Move everything to portal 1 now and draw from there, then move to og
		for(Scene::Object * obj : players[0].vicinity) {
			teleport(obj->transform, 1, false);
			obj->portal_in = &players[1];
		}
		for(Scene::Object * obj : players[1].vicinity) {
			teleport(obj->transform, 1, false);
			obj->portal_in = &players[1];
		}

		draw_portal(players[1]);

		for(Scene::Object * obj : players[0].vicinity) {
			teleport(obj->transform, 0, false);
			obj->portal_in = &players[0];
		}
	}

	// extra rendering from level?
	glUseProgram(texture_program->program);
	current_level->render_pass();

	//draw score
	if (level < 2) {
		glDisable(GL_DEPTH_TEST);
		std::string message = "SCORE "+std::to_string(scores[level]);
		float height = 0.1f;
		float width = text_width(message, height);
		draw_text(message, glm::vec2( 0.3 * width, 0.8f), height,
				glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

		glEnable(GL_DEPTH_TEST);
	}


	GL_ERRORS();

	//Copy scene from color buffer to screen, performing post-processing effects:
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fbs.color_tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fbs.bloom_color_tex);
	glBindVertexArray(*empty_vao);
    glUseProgram(*blur_program);
	glDrawArrays(GL_TRIANGLES, 0, 3);


	glUseProgram(0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GameMode::teleport(Scene::Transform *object_transform, const uint32_t to_portal_id, bool update_speed) {
	const Portal &from_portal = players[!to_portal_id];
	const Portal &  to_portal = players[ to_portal_id];

	{  // compute new position and speed
		// find angle between from_portal_normal and to_portal_normal (phi)
		//                    from_portal_normal and    -object_speed (theta)
		const glm::vec2 &from_normal = from_portal.normal;
		const glm::vec2 &  to_normal =   to_portal.normal;
		//object_transform->speed *= -1.0f;  // reverse speed

		/*
		auto angle_between = [=] (glm::vec2 from, glm::vec2 to) -> float {
			from = glm::normalize(from);
			to = glm::normalize(to);
			float sign = (from.x*to.y - from.y*to.x > 0.0f) ? 1.0f : -1.0f;
			return sign * std::acos(glm::dot(from, to));
		};
		float phi = angle_between(from_normal, to_normal);
		float theta = angle_between(from_normal, object_transform->speed);
		*/
        // rotate position by phi
		/*
		glm::mat4 pos_rotation = glm::rotate(glm::mat4(1.f), phi, glm::vec3(0.0f, 0.0f, 1.0f));
		auto pos_diff = glm::vec2(object_transform->position) - from_portal.position;
		auto rotated_pos_diff = glm::vec2(pos_rotation * glm::vec4(pos_diff, 0.0f, 1.0f));
		object_transform->position = glm::vec3(to_portal.position + rotated_pos_diff, 0.0f);
		*/

		// Instead, compute position along normal/parallel
		vec2 from_par = vec2(-from_normal.y, from_normal.x);
		vec2 to_par = vec2(-to_normal.y, to_normal.x);
		vec2 pos_diff = glm::vec2(object_transform->position) - from_portal.position;
		float norm_diff = glm::dot(pos_diff, from_normal);
		float par_diff = glm::dot(pos_diff, from_par);
		// new position along new normal and parallel, in opposite direction
		vec2 rotated_pos_diff = -norm_diff * to_normal - par_diff * to_par;
		object_transform->position = glm::vec3(to_portal.position + rotated_pos_diff, 0.0f);

		// Rotate object to opposite new normal
		float angle = atan2(-from_normal.x * to_normal.y + from_normal.y * to_normal.x, glm::dot(-to_normal, from_normal));
		object_transform->rotation = angleAxis(angle, vec3(0,0,1)) * object_transform->rotation;

		if (update_speed) {
			// rotate speed by phi - 2*theta
			/*
			glm::mat4 speed_rotation = glm::rotate(glm::mat4(1.f), phi - 2.0f*theta, glm::vec3(0.0f, 0.0f, 1.0f));
			auto new_speed = glm::vec2(speed_rotation * glm::vec4(object_transform->speed, 0.0f, 1.0f));
			// boost if new_speed is too slow
			float speed_lowerbound = 5.0f;
			object_transform->speed = (glm::length(new_speed) < speed_lowerbound) ?
									speed_lowerbound * glm::normalize(object_transform->speed) :
									new_speed;
			*/

			// Instead, compute speed along normal/parallel
			vec2 old_speed = object_transform->speed - from_portal.speed;
			float norm_spd = glm::dot(old_speed, from_normal);// - glm::dot(from_portal.speed, from_normal);
			float par_spd = glm::dot(old_speed, from_par);// - glm::dot(from_portal.speed, from_par);

			// If too slow along normal, give boost
			if(norm_spd > -5.f) norm_spd = -5.f;
			// new speed along new normal and parallel, in opposite direction
			vec2 new_speed = -norm_spd * to_normal - par_spd * to_par;
			object_transform->speed = new_speed + to_portal.speed;
		}
	}

	// update bbx
	object_transform->boundingbox->update_origin(object_transform->position);
}

void GameMode::show_pause_menu() {
	std::shared_ptr< MenuMode > menu = std::make_shared< MenuMode >();

	std::shared_ptr< Mode > game = shared_from_this();
	//menu->background = game;

	menu->choices.emplace_back("PAUSED");
	menu->choices.emplace_back("SAVE STATE 1", [game, this](){
			save(1,level, scores);
			Mode::set_current(game);
			});
	menu->choices.emplace_back("SAVE STATE 2", [game, this](){
			save(2,level, scores);
			Mode::set_current(game);
			});
	menu->choices.emplace_back("SAVE STATE 3", [game, this](){
			save(3,level, scores);
			Mode::set_current(game);
			});
	menu->choices.emplace_back("LOAD STATE 1", [game, this](){
			SaveData res = LoadSave(1);
			level = res.currentLevel;
			scores = res.personalBests;
			Mode::set_current(game);
			});
	menu->choices.emplace_back("LOAD STATE 2", [game, this](){
			SaveData res = LoadSave(2);
			level = res.currentLevel;
			scores = res.personalBests;
			Mode::set_current(game);
			});
	menu->choices.emplace_back("LOAD STATE 3", [game, this](){
			SaveData res = LoadSave(3);
			level = res.currentLevel;
			scores = res.personalBests;
			Mode::set_current(game);
			});
	menu->choices.emplace_back("QUIT", [](){
			Mode::set_current(nullptr);
			});

	menu->selected = 1;

	Mode::set_current(menu);
}

void GameMode::show_lose() {
	std::shared_ptr< MenuMode > menu = std::make_shared< MenuMode >();

	std::shared_ptr< Mode > game = shared_from_this();
	menu->background = game;

	menu->choices.emplace_back("GAME OVER");
	menu->choices.emplace_back("RESTAURANT BANKRUPT");
	menu->choices.emplace_back("QUIT", [](){
			Mode::set_current(nullptr);
			});

	menu->selected = 2;

	Mode::set_current(menu);
}

void GameMode::show_win() {

	std::shared_ptr< MenuMode > menu = std::make_shared< MenuMode >();

	std::shared_ptr< Mode > game = shared_from_this();
	menu->background = game;

	menu->choices.emplace_back("LEVEL PASSED");
	menu->choices.emplace_back("CONTINUE", [this, game](){
				level++;
				this->load_scene();
				Mode::set_current(game);
			});

	menu->selected = 1;

	Mode::set_current(menu);
}

void GameMode::show_level_select() {
	std::shared_ptr< MenuMode > menu = std::make_shared< MenuMode >();

	std::shared_ptr< Mode > game = shared_from_this();
	menu->background = game;

	menu->choices.emplace_back("SELECT LEVEL");
	menu->choices.emplace_back("VEGETABLES", [this, game](){

				SDL_SetRelativeMouseMode(SDL_TRUE);
				level = 0;
				this->load_scene();
				Mode::set_current(game);
			});
	menu->choices.emplace_back("OVEN", [this, game](){

				SDL_SetRelativeMouseMode(SDL_TRUE);
				level = 1;
				this->load_scene();
				Mode::set_current(game);
			});
    menu->choices.emplace_back("SPICEY", [this, game](){

				SDL_SetRelativeMouseMode(SDL_TRUE);
				level = 2;
				this->load_scene();
				Mode::set_current(game);
			});

	menu->selected = 1;

	Mode::set_current(menu);
}
