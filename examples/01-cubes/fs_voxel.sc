$input v_color0, facedata, amb_occ, v_normal
//$out vec4 fragColor;

#include "../common/common.sh"

// stbvox_default_ambient
uniform mat4 u_ambient;


uniform mat3 u_normal_table_0;
uniform mat3 u_normal_table_1;


void main()
{
	vec4 color;
	color.xyz = vec3(facedata.xyz) / 255.0;
	bool emissive = false;
	vec3 albedo = color.xyz;
	float fragment_alpha = 1.0;

	vec3 normal = v_normal;

	// TODO
	// vec3 ambient_color = dot(normal, ambient[0].xyz) * ambient[1].xyz + ambient[2].xyz;\n"
	vec3 ambient_color = dot(normal, u_ambient[0].xyz) * u_ambient[1].xyz + u_ambient[2].xyz;
	ambient_color = clamp(ambient_color, 0.0, 1.0);
	ambient_color *= amb_occ;

	vec3 lit_color;
	lit_color = albedo * ambient_color;

	gl_FragColor = vec4(lit_color, fragment_alpha);
	//fragColor = v_color0;
}