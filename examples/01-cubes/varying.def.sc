vec4 v_color0    : COLOR0    = vec4(1.0, 0.0, 0.0, 1.0);
flat uvec4 facedata: COLOR0;
vec3 v_normal: NORMAL = vec3(0.0, 0.0, 1.0);

float amb_occ : FOG;


vec3 a_position  : POSITION;


uvec4 a_color2  : COLOR2;


vec4 a_color0    : COLOR0;
uvec4 a_color1 : COLOR1;
vec4 a_normal    : NORMAL;

// uvec4 a_position: POSITION;
// This is a position in voxel
