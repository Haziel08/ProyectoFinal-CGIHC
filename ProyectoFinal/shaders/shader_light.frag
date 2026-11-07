#version 330
//>>>>>>>>>>>>>>>>>>>>>> Entradas y uniforms >>>>>>>>>>>>>>>>>>>>>>>>>

in vec4 v_col;
in vec2 tex_coord;
in vec3 normal;
in vec3 frag_pos;
in vec4 v_color;

out vec4 color;
uniform float brillo_frutos = 0.0;
uniform float luces_cohete = 0.0;
uniform bool recolorar_superficie = false;

const int max_point_lights = 16;
const int max_spot_lights = 4;

struct Light
{
	vec3 color;
	float ambient_intensity;
	float diffuse_intensity;
};

struct DirectionalLight 
{
	Light base;
	vec3 direction;
};

struct PointLight
{
	Light base;
	vec3 position;
	float constant;
	float linear;
	float exponent;
};


struct SpotLight
{
	PointLight base;
	vec3 direction;
	float edge;
};

struct Material
{
	float specular_intensity;
	float shininess;
};

uniform int point_light_count;
uniform int spot_light_count;

uniform DirectionalLight directional_light;
uniform PointLight point_lights[max_point_lights];
uniform SpotLight spot_lights[max_spot_lights];

uniform sampler2D the_texture;
uniform Material material;

uniform vec3 eye_position;
uniform bool normales_geometricas;
uniform float emision_material;
uniform float opacidad_material = 1.0;
uniform float factor_noche = 1.0;
uniform sampler2D textura_noche;
uniform bool tiene_textura_noche = false;
uniform int tipo_emision = 0; // 0 permanente, 1 luminaria, 2 LEDs de pista.

uniform bool recorte_alfa = false;
uniform bool relieve_vibrante;
uniform vec2 desplazamiento_textura = vec2(0.0);
uniform int modo_efecto = 0; // 0: material, 1: fuego, 2: humo de la practica 10.
uniform float opacidad_particula = 1.0;
uniform float opacidad_aerea = 1.0;

vec4 CalcLightByDirection(Light light, vec3 direction)
{
	vec4 ambient_color = vec4(light.color, 1.0f) * light.ambient_intensity;
	//producto punto: coseno del ángulo entre los dos vectores y normalizar para que sus módulos sean 1
	float diffuse_factor = max(dot(normalize(normal), normalize(direction)), 0.0f);
	vec4 diffuse_color = vec4(light.color * light.diffuse_intensity * diffuse_factor, 1.0f);
	
	vec4 specular_color = vec4(0, 0, 0, 0);
	
	if(diffuse_factor > 0.0f)
	{//si hay diffuse color entonces existe specular color; dependemos de la posición de la cámara
		vec3 frag_to_eye = normalize(eye_position - frag_pos);
		vec3 reflected_vertex = normalize(reflect(normales_geometricas ? -direction : direction, normalize(normal)));
		
		float specular_factor = dot(frag_to_eye, reflected_vertex);
		if(specular_factor > 0.0f)
		{
			specular_factor = pow(specular_factor, material.shininess);
			specular_color = vec4(light.color * material.specular_intensity * specular_factor, 1.0f);
		}
	}

	return (ambient_color + diffuse_color + specular_color);
}

vec4 CalcDirectionalLight()
{
	return CalcLightByDirection(directional_light.base, directional_light.direction);
}

//nuevo
vec4 CalcPointLight(PointLight p_light)
{
	vec3 direction = frag_pos - p_light.position;
	float distance = length(direction);
	direction = normalize(direction);
	if (normales_geometricas) direction = -direction;
	
	vec4 color = CalcLightByDirection(p_light.base, direction);
	float attenuation = p_light.exponent * distance * distance +
						p_light.linear * distance +
						p_light.constant;
	
	return (color / attenuation);
}




vec4 CalcSpotLight(SpotLight s_light)
{
	vec3 ray_direction = normalize(frag_pos - s_light.base.position);
	float sl_factor = dot(ray_direction, s_light.direction);
	
	if(sl_factor > s_light.edge)
	{
		vec4 color = CalcPointLight(s_light.base);
		
		return color * (1.0f - (1.0f - sl_factor)*(1.0f/(1.0f - s_light.edge)));
		
	} else {
		return vec4(0, 0, 0, 0);
	}
}










vec4 CalcPointLights()
{
	vec4 total_color = vec4(0, 0, 0, 0);
	for(int i = 0; i < point_light_count; i++)
	{		
		total_color += CalcPointLight(point_lights[i]);
	}
	
	return total_color;
}

vec4 CalcSpotLights()
{
	vec4 total_color = vec4(0, 0, 0, 0);
	for(int i = 0; i < spot_light_count; i++)
	{		
		total_color += CalcSpotLight(spot_lights[i]);
	}
	
	return total_color;
}


//>>>>>>>>>>>>>>>>>>>>>> Funcion principal del shader >>>>>>>>>>>>>>>>
void main()
{
	vec4 muestra = texture(the_texture, tex_coord + desplazamiento_textura);
	if (modo_efecto == 1)
	{
		float ancho = mix(0.49, 0.05, tex_coord.y);
		float borde = 1.0 - smoothstep(ancho * 0.60, ancho, abs(tex_coord.x - 0.5));
		float punta = 1.0 - smoothstep(0.40, 1.0, tex_coord.y);
		float raiz = smoothstep(0.0, 0.12, tex_coord.y);
		color = vec4(muestra.rgb * v_color.rgb * 2.2, muestra.a * borde * punta * raiz * opacidad_particula);
		return;
	}
	if (modo_efecto == 2)
	{
		color = vec4(muestra.rgb, muestra.a * opacidad_particula);
		return;
	}
    if (modo_efecto == 4) {
        float radio = length(tex_coord - vec2(0.5)) * 2.0;
        float borde = 1.0 - smoothstep(0.55, 1.0, radio);
        color = vec4(muestra.rgb * v_color.rgb, muestra.a * borde * opacidad_particula);
        return;
    }
	if (modo_efecto == 3)
	{
		float borde = pow(1.0 - clamp(dot(normalize(normal), normalize(eye_position - frag_pos)), 0.0, 1.0), 2.6);
		color = vec4(v_color.rgb, borde * opacidad_particula);
		return;
	}
	vec4 final_color = CalcDirectionalLight();
	final_color += CalcPointLights();
	final_color += CalcSpotLights();
	vec4 texel = texture(the_texture, tex_coord);
    if (recolorar_superficie) texel.rgb = vec3(dot(texel.rgb, vec3(.213,.715,.072)));
    if (tiene_textura_noche) texel = mix(texel, texture(textura_noche, tex_coord), factor_noche);
	if (recorte_alfa && texel.a < 0.1) discard;
	if (relieve_vibrante)
	{
		float luminancia = dot(texel.rgb, vec3(0.2126, 0.7152, 0.0722));
		texel.rgb = mix(vec3(luminancia), texel.rgb, 1.65);
		texel.rgb = max(vec3(0.0), (texel.rgb - vec3(0.18)) * 1.30 + vec3(0.18));
	}
	color = texel*v_color*final_color;
	color.rgb += texel.rgb * emision_material * (tipo_emision == 1 ? factor_noche : 1.0);
    float azul_fruto = smoothstep(.12,.38,texel.b-max(texel.r,texel.g));
    color.rgb += brillo_frutos * azul_fruto * texel.rgb;
    if (luces_cohete > 0.0)
    {
        vec2 led = abs(fract(tex_coord * vec2(12.0,18.0)) - .5);
        float baliza = 1.0 - smoothstep(.08,.13,length(led));
        color.rgb += vec3(1.0,.19,.06) * baliza * luces_cohete;
    }
    if (tipo_emision == 2) {
        bool rojo = texel.r > texel.g * 1.6 && texel.r > texel.b * 1.35;
        bool azul = texel.b > texel.r * 1.6 && texel.b > texel.g * 0.85;
        float led = (rojo || azul) ? smoothstep(0.45, 0.90, max(texel.r, texel.b)) : 0.0;
        color.rgb += texel.rgb * led * factor_noche * 1.6;
    }
	if (normales_geometricas) color.a = texture(the_texture, tex_coord).a * v_color.a;
	color.a *= opacidad_aerea * opacidad_material;
	
}
