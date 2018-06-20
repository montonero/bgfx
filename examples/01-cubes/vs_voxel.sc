//$input a_position, a_color1
$input a_color1, a_color2

$output v_color0

/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "../common/common.sh"

void main()
{
	//gl_Position =  vec4(a_position, 1.0) ;

	vec3 col = vec3(0,0,0);
	col.x = float((a_color1.x) & 255u);
	col.y = float((a_color1.y ) & 255u) ;
	col.z = float((a_color1.z ) & 255u);

    
    // reconstruct uint32 from components
    uint posU = a_color2.x + (a_color2.y << 8u) + (a_color2.z << 16u) + (a_color2.w << 24u);
    //uint posU = uint(a_color2.xyzw);
    vec3 offset;
    offset.x = float( (posU) & 127u );
    offset.y = float( (posU >> 7u) & 127u);
    offset.z = float( (posU >> 14u) & 511u) * 0.5;

    col = col / 256.0;
    v_color0 = vec4(col, 1.0);

    gl_Position = mul(u_modelViewProj, vec4(offset, 1.0) );
 
}
