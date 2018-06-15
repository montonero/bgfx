$input a_position, a_color0Int
//$output v_color0

//$input a_position, a_normal, a_positionUint
$output v_color0

/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "../common/common.sh"

void main()
{
	gl_Position =  vec4(a_position, 1.0) ;

	//vec3 pos = vec3(0,0,0);
	float r = float((a_color0Int) & 255u);
	float g =float((a_color0Int >> 8u) & 255u) ;
	float b = float((a_color0Int >> 16u) & 255u);

    //pos = pos / 255.0;
	//v_color0 = a_color0;
    v_color0 = vec4(b, g,r , 1.0);
    //v_color0 = vec4(1., 0.0, 0., 1.0);
}
