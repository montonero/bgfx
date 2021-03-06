$input a_color2, a_color1
// a_color2 -- position 
// a_color1 -- facedata

//$input a_color1, a_position
$output v_color0, facedata, amb_occ, v_normal

#include "../common/common.sh"

uniform mat3 u_normal_table_0;
uniform mat3 u_normal_table_1;

uniform mat4 u_ambient;



void main()
{
	vec3 col = vec3(0,0,0);
	col.x = float((a_color1.x) & 255u);
	col.y = float((a_color1.y ) & 255u) ;
	col.z = float((a_color1.z ) & 255u);

    
    // reconstruct uint32 from components
    //uint posU = 0u;
    uint posU = a_color2.x + (a_color2.y << 8u) + (a_color2.z << 16u) + (a_color2.w << 24u);
    //uint posU = uint(a_color2.xyzw);
    vec3 offset;
    offset.x = float( (posU) & 127u );
    offset.y = float( (posU >> 7u) & 127u);
    offset.z = float( (posU >> 14u) & 511u) * 0.5;
    amb_occ = float( (posU >> 23u) & 63u ) / 63.0;

    facedata = a_color1;
        
    int normal_face = int(facedata.w >> 2u) & 31;
    if (normal_face < 3) {
        v_normal.x = u_normal_table_0[0][normal_face];
        v_normal.y = u_normal_table_0[1][normal_face];
        v_normal.z = u_normal_table_0[2][normal_face];
    }
    else {
        //int idx = normal_face - 3;
        normal_face = normal_face - 3;
        v_normal.x = u_normal_table_1[0][normal_face];
        v_normal.y = u_normal_table_1[1][normal_face];
        v_normal.z = u_normal_table_1[2][normal_face];
    }
    
    col = col / 256.0;
    v_color0 = vec4(col, 1.0);

    
    gl_Position = mul(u_modelViewProj, vec4(offset, 1.0) );
 }
