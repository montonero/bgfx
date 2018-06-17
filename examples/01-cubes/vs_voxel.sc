$input a_position, a_color1

$output v_color0

/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "../common/common.sh"

void main()
{
	gl_Position =  vec4(a_position, 1.0) ;

	vec3 col = vec3(0,0,0);
	col.x = float((a_color1.x) & 255u);
	col.y = float((a_color1.y ) & 255u) ;
	col.z = float((a_color1.z ) & 255u);

    col = col / 256.0;
	//v_color0 = a_color0;
    v_color0 = vec4(col, 1.0);
    //v_color0 = vec4(1., 0.0, 0., 1.0);
}
