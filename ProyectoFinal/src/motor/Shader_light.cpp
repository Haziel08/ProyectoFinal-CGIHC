//>>>>>>>>>>>>>>>>>>>>>> Shader_light >>>>>>>>>>>>>>>>>>>>>>
#include "Shader_light.h"

Shader::Shader()
{
	shader_id = 0;
	uniform_model = 0;
	uniform_projection = 0;
	uniform_color = 0;
	point_light_count = 0;
	spot_light_count = 0;
}

void Shader::CreateFromString(const char* vertex_code, const char* fragment_code)
{
	CompileShader(vertex_code, fragment_code);
}

void Shader::CreateFromFiles(const char* vertex_location, const char* fragment_location)
{
	std::string vertex_string = ReadFile(vertex_location);
	std::string fragment_string = ReadFile(fragment_location);
	const char* vertex_code = vertex_string.c_str();
	const char* fragment_code = fragment_string.c_str();

	CompileShader(vertex_code, fragment_code);
}

std::string Shader::ReadFile(const char* file_location)
{
	std::string content;
	std::ifstream file_stream(file_location, std::ios::in);

	if (!file_stream.is_open()) {
		printf("Failed to read %s! File doesn't exist.", file_location);
		return "";
	}

	std::string line = "";
	while (!file_stream.eof())
	{
		std::getline(file_stream, line);
		content.append(line + "\n");
	}

	file_stream.close();
	return content;
}

void Shader::CompileShader(const char* vertex_code, const char* fragment_code)
{
	shader_id = glCreateProgram();

	if (!shader_id)
	{
		printf("Error creating shader program!\n");
		return;
	}

	AddShader(shader_id, vertex_code, GL_VERTEX_SHADER);
	AddShader(shader_id, fragment_code, GL_FRAGMENT_SHADER);

	GLint result = 0;
	GLchar e_log[1024] = { 0 };

	glLinkProgram(shader_id);
	glGetProgramiv(shader_id, GL_LINK_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shader_id, sizeof(e_log), NULL, e_log);
		printf("Error linking program: '%s'\n", e_log);
		return;
	}

	glValidateProgram(shader_id);
	glGetProgramiv(shader_id, GL_VALIDATE_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shader_id, sizeof(e_log), NULL, e_log);
		printf("Error validating program: '%s'\n", e_log);
		return;
	}

	uniform_projection = glGetUniformLocation(shader_id, "projection");
	uniform_model = glGetUniformLocation(shader_id, "model");
	uniform_view = glGetUniformLocation(shader_id, "view");
	uniform_directional_light.uniform_color = glGetUniformLocation(shader_id, "directional_light.base.color");
	uniform_directional_light.uniform_ambient_intensity = glGetUniformLocation(shader_id, "directional_light.base.ambient_intensity");
	uniform_directional_light.uniform_direction = glGetUniformLocation(shader_id, "directional_light.direction");
	uniform_directional_light.uniform_diffuse_intensity = glGetUniformLocation(shader_id, "directional_light.base.diffuse_intensity");
	uniform_specular_intensity = glGetUniformLocation(shader_id, "material.specular_intensity");
	uniform_shininess = glGetUniformLocation(shader_id, "material.shininess");
	uniform_eye_position = glGetUniformLocation(shader_id, "eye_position");
	uniform_color = glGetUniformLocation(shader_id, "color");

	uniform_point_light_count = glGetUniformLocation(shader_id, "point_light_count");

	for (size_t i = 0; i < max_point_lights; i++)
	{
		char loc_buff[100] = { '\0' };

		snprintf(loc_buff, sizeof(loc_buff), "point_lights[%zu].base.color", i);
		uniform_point_light[i].uniform_color = glGetUniformLocation(shader_id, loc_buff);

		snprintf(loc_buff, sizeof(loc_buff), "point_lights[%zu].base.ambient_intensity", i);
		uniform_point_light[i].uniform_ambient_intensity = glGetUniformLocation(shader_id, loc_buff);

		snprintf(loc_buff, sizeof(loc_buff), "point_lights[%zu].base.diffuse_intensity", i);
		uniform_point_light[i].uniform_diffuse_intensity = glGetUniformLocation(shader_id, loc_buff);

		snprintf(loc_buff, sizeof(loc_buff), "point_lights[%zu].position", i);
		uniform_point_light[i].uniform_position = glGetUniformLocation(shader_id, loc_buff);

		snprintf(loc_buff, sizeof(loc_buff), "point_lights[%zu].constant", i);
		uniform_point_light[i].uniform_constant = glGetUniformLocation(shader_id, loc_buff);

		snprintf(loc_buff, sizeof(loc_buff), "point_lights[%zu].linear", i);
		uniform_point_light[i].uniform_linear = glGetUniformLocation(shader_id, loc_buff);

		snprintf(loc_buff, sizeof(loc_buff), "point_lights[%zu].exponent", i);
		uniform_point_light[i].uniform_exponent = glGetUniformLocation(shader_id, loc_buff);
	}

	uniform_spot_light_count = glGetUniformLocation(shader_id, "spot_light_count");

	for (size_t i = 0; i < max_spot_lights; i++)
	{
		char loc_buff[100] = { '\0' };

		snprintf(loc_buff, sizeof(loc_buff), "spot_lights[%zu].base.base.color", i);
		uniform_spot_light[i].uniform_colour = glGetUniformLocation(shader_id, loc_buff);

		snprintf(loc_buff, sizeof(loc_buff), "spot_lights[%zu].base.base.ambient_intensity", i);
		uniform_spot_light[i].uniform_ambient_intensity = glGetUniformLocation(shader_id, loc_buff);

		snprintf(loc_buff, sizeof(loc_buff), "spot_lights[%zu].base.base.diffuse_intensity", i);
		uniform_spot_light[i].uniform_diffuse_intensity = glGetUniformLocation(shader_id, loc_buff);

		snprintf(loc_buff, sizeof(loc_buff), "spot_lights[%zu].base.position", i);
		uniform_spot_light[i].uniform_position = glGetUniformLocation(shader_id, loc_buff);

		snprintf(loc_buff, sizeof(loc_buff), "spot_lights[%zu].base.constant", i);
		uniform_spot_light[i].uniform_constant = glGetUniformLocation(shader_id, loc_buff);

		snprintf(loc_buff, sizeof(loc_buff), "spot_lights[%zu].base.linear", i);
		uniform_spot_light[i].uniform_linear = glGetUniformLocation(shader_id, loc_buff);

		snprintf(loc_buff, sizeof(loc_buff), "spot_lights[%zu].base.exponent", i);
		uniform_spot_light[i].uniform_exponent = glGetUniformLocation(shader_id, loc_buff);

		snprintf(loc_buff, sizeof(loc_buff), "spot_lights[%zu].direction", i);
		uniform_spot_light[i].uniform_direction = glGetUniformLocation(shader_id, loc_buff);

		snprintf(loc_buff, sizeof(loc_buff), "spot_lights[%zu].edge", i);
		uniform_spot_light[i].uniform_edge = glGetUniformLocation(shader_id, loc_buff);
	}

}

GLuint Shader::GetProjectionLocation()
{
	return uniform_projection;
}
GLuint Shader::GetModelLocation()
{
	return uniform_model;
}
GLuint Shader::GetViewLocation()
{
	return uniform_view;
}
GLuint Shader::GetAmbientcolorLocation()
{
	return uniform_directional_light.uniform_color;
}
GLuint Shader::GetAmbientIntensityLocation()
{
	return uniform_directional_light.uniform_ambient_intensity;
}
GLuint Shader::GetDiffuseIntensityLocation()
{
	return uniform_directional_light.uniform_diffuse_intensity;
}
GLuint Shader::GetDirectionLocation()
{
	return uniform_directional_light.uniform_direction;
}
GLuint Shader::GetSpecularIntensityLocation()
{
	return uniform_specular_intensity;
}
GLuint Shader::GetShininessLocation()
{
	return uniform_shininess;
}
GLuint Shader::GetEyePositionLocation()
{
	return uniform_eye_position;
}

GLuint Shader::getColorLocation()
{
	return uniform_color;
}
void Shader::SetDirectionalLight(DirectionalLight * d_light)
{
	d_light->UseLight(uniform_directional_light.uniform_ambient_intensity, uniform_directional_light.uniform_color,
		uniform_directional_light.uniform_diffuse_intensity, uniform_directional_light.uniform_direction);
}

void Shader::SetPointLights(PointLight * p_light, unsigned int light_count)
{
	if (light_count > max_point_lights) light_count = max_point_lights;

	glUniform1i(uniform_point_light_count, light_count);

	for (size_t i = 0; i < light_count; i++)
	{
		p_light[i].UseLight(uniform_point_light[i].uniform_ambient_intensity, uniform_point_light[i].uniform_color,
			uniform_point_light[i].uniform_diffuse_intensity, uniform_point_light[i].uniform_position,
			uniform_point_light[i].uniform_constant, uniform_point_light[i].uniform_linear, uniform_point_light[i].uniform_exponent);
	}
}

void Shader::SetSpotLights(SpotLight * s_light, unsigned int light_count)
{
	if (light_count > max_spot_lights) light_count = max_spot_lights;

	glUniform1i(uniform_spot_light_count, light_count);

	for (size_t i = 0; i < light_count; i++)
	{
		s_light[i].UseLight(uniform_spot_light[i].uniform_ambient_intensity, uniform_spot_light[i].uniform_colour,
			uniform_spot_light[i].uniform_diffuse_intensity, uniform_spot_light[i].uniform_position, uniform_spot_light[i].uniform_direction,
			uniform_spot_light[i].uniform_constant, uniform_spot_light[i].uniform_linear, uniform_spot_light[i].uniform_exponent,
			uniform_spot_light[i].uniform_edge);
	}
}


void Shader::UseShader()
{
	glUseProgram(shader_id);
}

void Shader::ClearShader()
{
	if (shader_id != 0)
	{
		glDeleteProgram(shader_id);
		shader_id = 0;
	}

	uniform_model = 0;
	uniform_projection = 0;
	uniform_color = 0;
	uniform_view = 0;

}


void Shader::AddShader(GLuint the_program, const char* shader_code, GLenum shader_type)
{
	GLuint the_shader = glCreateShader(shader_type);

	glShaderSource(the_shader, 1, &shader_code, nullptr);
	glCompileShader(the_shader);

	GLint result = 0;
	GLchar e_log[1024] = { 0 };

	glGetShaderiv(the_shader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(the_shader, sizeof(e_log), NULL, e_log);
		printf("Error compiling the %d shader: '%s'\n", shader_type, e_log);
		return;
	}

	glAttachShader(the_program, the_shader);
}

Shader::~Shader()
{
	ClearShader();
}
