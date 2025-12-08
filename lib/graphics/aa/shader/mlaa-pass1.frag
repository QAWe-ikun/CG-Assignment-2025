#version 460

layout (set = 2, binding = 0) uniform sampler2D color_tex;

layout (location = 0) out vec2 out_color;

/*==========*/

#define LUMA_THRESHOLD 0.1

ivec2 pixcoord = ivec2(floor(gl_FragCoord.xy));

/*==========*/

float luma(in vec3 color)
{
	return dot(color, vec3(0.21, 0.72, 0.07));
}

#define get_color(offset) texelFetchOffset(color_tex, pixcoord, 0, offset).rgb

vec2 find_edge()
{
	const vec3 colM = get_color(ivec2(0, 0)), 
		colN = get_color(ivec2(0, -1)),
		colW = get_color(ivec2(-1, 0));

	const float lumaM = luma(colM),
		lumaN = luma(colN),
		lumaW = luma(colW);

	const float left_luma_grad = abs(lumaW - lumaM);
	const float top_luma_grad = abs(lumaN - lumaM);

	return vec2(
		step(LUMA_THRESHOLD, left_luma_grad), 
		step(LUMA_THRESHOLD, top_luma_grad)
	);
}

void main()
{
	vec2 edge = find_edge();

	if(edge.x + edge.y < 0.0001)
		discard;

	out_color = edge;
}