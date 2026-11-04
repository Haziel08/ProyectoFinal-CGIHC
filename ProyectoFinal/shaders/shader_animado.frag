#version 330
// Shader animado: textura que se desplaza con el tiempo, transparencia y
// luz direccional (difusa + especular).
//   Agua:  usarLuz = 1, transparencia ~0.6, ondas suaves
//   Fuego: usarLuz = 0, transparencia ~0.9, textura subiendo (velocidadUV en Y)
//          y en C++ usar glBlendFunc(GL_SRC_ALPHA, GL_ONE) para que brille

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 color;

uniform sampler2D textura;
uniform float tiempo;
uniform vec2  velocidadUV;     // cuanto se mueve la textura por segundo
uniform float repeticionUV;    // cuantas veces se repite la textura
uniform vec3  colorBase;       // tinte que multiplica a la textura
uniform float transparencia;   // 0 = invisible, 1 = opaco

uniform int   usarLuz;         // 1 = recibe la luz direccional, 0 = no
uniform vec3  luzDireccion;    // hacia donde viaja la luz (igual que DirectionalLight)
uniform vec3  luzColor;
uniform float luzAmbiental;
uniform float luzDifusa;
uniform vec3  posCamara;
uniform float brillo;          // que tan concentrado es el reflejo (shininess)

void main()
{
	// Dos copias de la textura moviendose distinto: parece que el agua fluye
	vec2 uv = TexCoord * repeticionUV;
	vec4 t1 = texture(textura, uv + velocidadUV * tiempo);
	vec4 t2 = texture(textura, uv * 1.37 - velocidadUV * tiempo * 0.7);
	vec4 tex = mix(t1, t2, 0.5);

	vec3 rgb = tex.rgb * colorBase;
	float alfa = transparencia * tex.a;

	if (usarLuz == 1)
	{
		vec3 N = normalize(Normal);
		vec3 L = normalize(-luzDireccion);           // hacia la luz
		vec3 V = normalize(posCamara - FragPos);     // hacia la camara

		float difusa = max(dot(N, L), 0.0);
		float especular = pow(max(dot(V, reflect(-L, N)), 0.0), brillo);
		rgb = rgb * luzColor * (luzAmbiental + luzDifusa * difusa)
			+ luzColor * especular * luzDifusa;

		// Visto de lado el agua refleja mas y se ve menos transparente (Fresnel)
		float lado = pow(1.0 - max(dot(N, V), 0.0), 3.0);
		alfa = mix(alfa, 1.0, lado * 0.6);
	}

	if (alfa < 0.01)
		discard;
	color = vec4(rgb, alfa);
}
