#include "bloom_program.hpp"

#include "compile_program.hpp"
#include "gl_errors.hpp"

BloomProgram::BloomProgram() {
	program = compile_program(
		"#version 330\n"
		"uniform mat4 object_to_clip;\n"
		"uniform mat4x3 object_to_light;\n"
		"uniform mat3 normal_to_light;\n"
		"uniform mat4 light_to_spot;\n"
		"layout(location=0) in vec4 Position;\n" //note: layout keyword used to make sure that the location-0 attribute is always bound to something
		"in vec3 Normal;\n"
		"in vec4 Color;\n"
		"in vec2 TexCoord;\n"
		"out vec3 position;\n"
		"out vec3 normal;\n"
		"out vec4 color;\n"
		"out vec2 texCoord;\n"
		"out vec4 spotPosition;\n"
		"void main() {\n"
		"	gl_Position = object_to_clip * Position;\n"
		"	position = object_to_light * Position;\n"
		"	spotPosition = light_to_spot * vec4(position, 1.0);\n"
		"	normal = normal_to_light * Normal;\n"
		"	color = Color;\n"
		"	texCoord = TexCoord;\n"
		"}\n"
		,
		"#version 330\n"
		"uniform vec3 sun_direction;\n"
		"uniform vec3 sun_color;\n"
		"uniform vec3 sky_direction;\n"
		"uniform vec3 sky_color;\n"
		"uniform vec3 spot_position;\n"
		"uniform vec3 spot_direction;\n"
		"uniform vec3 spot_color;\n"
		"uniform vec2 spot_outer_inner;\n"
		"uniform sampler2D tex;\n"
		"uniform sampler2DShadow spot_depth_tex;\n"
		"in vec3 position;\n"
		"in vec3 normal;\n"
		"in vec4 color;\n"
		"in vec2 texCoord;\n"
		"in vec4 spotPosition;\n"
		"out vec4 fragColor;\n"
		"void main() {\n"
		"	vec3 total_light = vec3(0.0, 0.0, 0.0);\n"
		"	vec3 n = normalize(normal);\n"
		"	{ //sky (hemisphere) light:\n"
		"		vec3 l = sky_direction;\n"
		"		float nl = 0.5 + 0.5 * dot(n,l);\n"
		"		total_light += nl * sky_color;\n"
		"	}\n"
		"	{ //sun (directional) light:\n"
		"		vec3 l = sun_direction;\n"
		"		float nl = max(0.0, dot(n,l));\n"
		"		total_light += nl * sun_color;\n"
		"	}\n"
		"	{ //spot (point with fov + shadow map) light:\n"
		"		vec3 l = normalize(spot_position - position);\n"
		"		float nl = max(0.0, dot(n,l));\n"
		"		//TODO: look up shadow map\n"
		"		float d = dot(l,-spot_direction);\n"
		"		float amt = smoothstep(spot_outer_inner.x, spot_outer_inner.y, d);\n"
		"		float shadow = textureProj(spot_depth_tex, spotPosition);\n"
		"		total_light += shadow * nl * amt * spot_color;\n"
		"	}\n"
        //"   fragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"	fragColor = texture(tex, texCoord) * vec4(color.rgb * total_light, color.a);\n"
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
                "vec4 blur = fragColor;\n"
                "float centerB = dot(blur.rgb, vec3(1.0, 1.0, 1.0));\n"
                "float brightness;\n"
                "float threshold = 0.0;\n"
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

	object_to_clip_mat4 = glGetUniformLocation(program, "object_to_clip");
	object_to_light_mat4x3 = glGetUniformLocation(program, "object_to_light");
	normal_to_light_mat3 = glGetUniformLocation(program, "normal_to_light");

	sun_direction_vec3 = glGetUniformLocation(program, "sun_direction");
	sun_color_vec3 = glGetUniformLocation(program, "sun_color");
	sky_direction_vec3 = glGetUniformLocation(program, "sky_direction");
	sky_color_vec3 = glGetUniformLocation(program, "sky_color");

	spot_position_vec3 = glGetUniformLocation(program, "spot_position");
	spot_direction_vec3 = glGetUniformLocation(program, "spot_direction");
	spot_color_vec3 = glGetUniformLocation(program, "spot_color");
	spot_outer_inner_vec2 = glGetUniformLocation(program, "spot_outer_inner");

	light_to_spot_mat4 = glGetUniformLocation(program, "light_to_spot");

	glUseProgram(program);

	GLuint tex_sampler2D = glGetUniformLocation(program, "tex");
	glUniform1i(tex_sampler2D, 0);

	GLuint spot_depth_tex_sampler2D = glGetUniformLocation(program, "spot_depth_tex");
	glUniform1i(spot_depth_tex_sampler2D, 1);

	glUseProgram(0);

	GL_ERRORS();
}

Load< BloomProgram > bloom_program(LoadTagInit, [](){
	return new BloomProgram();
});
