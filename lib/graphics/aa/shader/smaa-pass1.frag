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
	const float 
		lumaM = luma(get_color(ivec2(0, 0))),
		lumaN = luma(get_color(ivec2(0, -1))),
		lumaN2 = luma(get_color(ivec2(0, -2))),
		lumaW = luma(get_color(ivec2(-1, 0))),
		lumaW2 = luma(get_color(ivec2(-2, 0))),
		lumaS = luma(get_color(ivec2(0, 1))),
		lumaE = luma(get_color(ivec2(1, 0)));

	const float
		c_l = abs(lumaM - lumaW),
		c_r = abs(lumaM - lumaE),
		c_t = abs(lumaM - lumaN),
		c_b = abs(lumaM - lumaS),
		c_2l = abs(lumaW - lumaW2),
		c_2t = abs(lumaN - lumaN2);

	const float max_c = max(max(max(c_l, c_r), max(c_t, c_b)), max(c_2l, c_2t));
	const float thres = max_c * 0.5;

	return vec2(
		step(LUMA_THRESHOLD, c_l) * step(thres, c_l),
		step(LUMA_THRESHOLD, c_t) * step(thres, c_t)
	);
}

void main()
{
	vec2 edge = find_edge();

	if(edge.x + edge.y < 0.0001)
		discard;

	out_color = edge;
}