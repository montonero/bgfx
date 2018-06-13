$input a_position, a_normal, a_positionUint
$output v_color0

/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "../common/common.sh"

void main()
{
	vec3 pos_;
	pos_.x = float((a_positionUint) & 255u);
	pos_.y = float((a_positionUint >> 8u) & 255u);
	pos_.z = float((a_positionUint >> 16u) & 255u);
	
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
	v_color0 = vec4(a_normal.xyz, 0.5);
}
