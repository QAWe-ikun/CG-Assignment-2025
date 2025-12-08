#version 460

layout (set = 2, binding = 0) uniform sampler2D tex;


layout (location = 0) out vec4 out_color;

/* FXAA 参数 */

#define FXAA_MAX_SEARCH_STEP 5
const float[] search_step = float[](0.75, 1.5, 2, 4, 12);

#define FXAA_ABSOLUTE_LUMA_THRESHOLD 0.05
#define FXAA_SEARCH_END_THRESHOLD 0.07

/* FXAA 实现 */

ivec2 fragcoord_int = ivec2(floor(gl_FragCoord.xy));
vec2 texture_size = vec2(textureSize(tex, 0));

// 计算亮度
float luma(in vec3 color)
{
	return dot(color, vec3(0.21, 0.72, 0.07));
}

// 获取颜色（浮点）
vec3 get_color_float(vec2 offset)
{
	return textureLod(tex, (gl_FragCoord.xy + offset) / texture_size, 0.0).rgb;
}

// 获取颜色（整数）
vec3 get_color_int(ivec2 offset)
{
	return texelFetch(tex, fragcoord_int + offset, 0).rgb;
}

vec3 fxaa()
{
	const vec3 colM = get_color_int(ivec2(0, 0)), 
		colN = get_color_int(ivec2(0, 1)),
		colE = get_color_int(ivec2(1, 0)),
		colS = get_color_int(ivec2(0, -1)),
		colW = get_color_int(ivec2(-1, 0));

	const float lumaM = luma(colM),
		lumaN = luma(colN),
		lumaE = luma(colE),
		lumaS = luma(colS),
		lumaW = luma(colW);

	const float minNS = min(lumaN, lumaS), minWE = min(lumaW, lumaE), minLuma = min(lumaM, min(minNS, minWE));
	const float maxNS = max(lumaN, lumaS), maxWE = max(lumaW, lumaE), maxLuma = max(lumaM, max(maxNS, maxWE));
	const float lumaContrast = maxLuma - minLuma;
	const float avg = (maxLuma + minLuma) / 2;

	if(lumaContrast < FXAA_ABSOLUTE_LUMA_THRESHOLD) return colM;
	
	const float lumaNW = luma(get_color_int(ivec2(-1, 1))),
		lumaNE = luma(get_color_int(ivec2(1, 1))),
		lumaSW = luma(get_color_int(ivec2(-1, -1))),
		lumaSE = luma(get_color_int(ivec2(1, -1)));

	const float lumaGradS = lumaS - lumaM,
		lumaGradN = lumaN - lumaM,
		lumaGradW = lumaW - lumaM,
		lumaGradE = lumaE - lumaM;

	const float lumaGradH = abs(lumaNW + lumaNE - 2 * lumaN) 
		+ 2 * abs(lumaW + lumaE - 2 * lumaM) 
		+ abs(lumaSW + lumaSE - 2 * lumaS); 

	const float lumaGradV = abs(lumaNW + lumaSW - 2 * lumaW)
		+ 2 * abs(lumaN + lumaS - 2 * lumaM) 
		+ abs(lumaNE + lumaSE - 2 * lumaE); 

	bool isHorz = lumaGradV > lumaGradH; 

	vec2 normal = vec2(0, 0);
	vec3 blend_src;

	const float delta_luma_NS = abs(lumaGradN) - abs(lumaGradS),
		delta_luma_WE = abs(lumaGradE) - abs(lumaGradW);

	if(isHorz)
	{
		normal.y = sign(delta_luma_NS);
		blend_src = mix(colS, colN, step(0, delta_luma_NS));
	}
	else
	{
		normal.x = sign(delta_luma_WE);
		blend_src = mix(colW, colE, step(0, delta_luma_WE));
	}

	const vec2 offset = normal / 2;
	const vec2 search_dir = normal.yx;

	float luma_pos, luma_neg;
	float pos, neg;
	const float luma_blend_src = luma(blend_src);
	const float luma_start = (lumaM + luma_blend_src) / 2;
	const float end_thres = FXAA_SEARCH_END_THRESHOLD;

	// positive direction search
	for(uint pos_search = 0; pos_search < FXAA_MAX_SEARCH_STEP; pos_search ++)
	{
		pos = search_step[pos_search];
		luma_pos = luma(get_color_float(offset + search_dir * pos));
		if(abs(luma_pos - luma_start) > end_thres) break;
	}

	for(uint neg_search = 0; neg_search < FXAA_MAX_SEARCH_STEP; neg_search ++)
	{
		neg = search_step[neg_search];
		luma_neg = luma(get_color_float(offset - search_dir * neg));
		if(abs(luma_neg - luma_start) > end_thres) break;
	}

	const float edge_length = pos + neg;
	const bool pos_closer = pos < neg;

	const float dst = pos_closer ? pos : neg;
	const float dst_luma = pos_closer ? luma_pos : luma_neg;

	float blend = 0;
	
	if((dst_luma - luma_start) * (lumaM - luma_start) <= 0)
		blend = abs(0.5 - dst / edge_length);

	return mix(colM, blend_src, blend);
}

void main()
{
	out_color = vec4(fxaa(), 1.0);
}