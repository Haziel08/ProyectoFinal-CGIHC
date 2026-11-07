#pragma once
//>>>>>>>>>>>>>>>>>>>>>> Definiciones de Shader_light >>>>>>>>>>>>>>>>>>>>>>

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <glew.h>
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"

class Shader
{
public:
	Shader();

	void CreateFromString(const char* vertex_code, const char* fragment_code);
	void CreateFromFiles(const char* vertex_location, const char* fragment_location);

	std::string ReadFile(const char* file_location);

	GLuint GetProjectionLocation();
	GLuint GetModelLocation();
	GLuint GetViewLocation();
	GLuint GetAmbientIntensityLocation();
	GLuint GetAmbientcolorLocation();
	GLuint GetDiffuseIntensityLocation();
	GLuint GetDirectionLocation();
	GLuint GetSpecularIntensityLocation();
	GLuint GetShininessLocation();
	GLuint GetEyePositionLocation();
	GLuint getColorLocation();

	void SetDirectionalLight(DirectionalLight * d_light);
	void SetPointLights(PointLight * p_light, unsigned int light_count);
	void SetSpotLights(SpotLight * s_light, unsigned int light_count);

	void UseShader();
	void ClearShader();

	~Shader();

private:
	int point_light_count;
	int spot_light_count;
	GLuint shader_id, uniform_projection, uniform_model, uniform_color, uniform_view, uniform_eye_position,
		uniform_specular_intensity, uniform_shininess;

	struct {
		GLuint uniform_color;
		GLuint uniform_ambient_intensity;
		GLuint uniform_diffuse_intensity;

		GLuint uniform_direction;
	} uniform_directional_light;

	GLuint uniform_point_light_count;

	struct {
		GLuint uniform_color;
		GLuint uniform_ambient_intensity;
		GLuint uniform_diffuse_intensity;

		GLuint uniform_position;
		GLuint uniform_constant;
		GLuint uniform_linear;
		GLuint uniform_exponent;
	} uniform_point_light[max_point_lights];

	GLuint uniform_spot_light_count;

	struct {
		GLuint uniform_colour;
		GLuint uniform_ambient_intensity;
		GLuint uniform_diffuse_intensity;

		GLuint uniform_position;
		GLuint uniform_constant;
		GLuint uniform_linear;
		GLuint uniform_exponent;

		GLuint uniform_direction;
		GLuint uniform_edge;
	} uniform_spot_light[max_spot_lights];


	void CompileShader(const char* vertex_code, const char* fragment_code);
	void AddShader(GLuint the_program, const char* shader_code, GLenum shader_type);
};

