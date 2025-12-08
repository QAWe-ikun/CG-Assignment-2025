#version 460

layout (set = 2, binding = 0) uniform sampler2D color_tex;
layout (set = 2, binding = 1) uniform sampler2D blend_tex;

layout (location = 0) out vec4 out_color;

/*==========*/

ivec2 fragcoord_int = ivec2(floor(gl_FragCoord.xy));
vec2 texture_size = vec2(textureSize(blend_tex, 0));

/*==========*/

vec4 get_color(vec2 offset)
{
	return textureLod(color_tex, (gl_FragCoord.xy + offset) / texture_size, 0.0);
}

void main()
{
	vec4 current_pixel_weight = texelFetch(blend_tex, fragcoord_int, 0);
	float right_weight = texelFetchOffset(blend_tex, fragcoord_int, 0, ivec2(0, 1)).g;
	float bottom_weight = texelFetchOffset(blend_tex, fragcoord_int, 0, ivec2(1, 0)).a;

	vec4 weights = vec4(current_pixel_weight.r, right_weight, current_pixel_weight.b, bottom_weight);
	float sum = dot(weights, vec4(1.0));

	if(sum > 0.0001)
	{
		vec4 color = vec4(0.0);
		color += get_color(vec2(0.0, -weights.r)) * weights.r;
		color += get_color(vec2(0.0, weights.g)) * weights.g;
		color += get_color(vec2(-weights.b, 0.0)) * weights.b;
		color += get_color(vec2(weights.a, 0.0)) * weights.a;

		out_color = color / sum;
	}
	else
		out_color = texelFetch(color_tex, fragcoord_int, 0);
}