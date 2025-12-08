#version 460

layout(location = 0) in vec2 in_pos;

layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec2 out_ndc;

vec2 ndc_to_uv(vec2 ndc)
{
    return fma(ndc, vec2(0.5, -0.5), vec2(0.5));
}

void main()
{
    out_ndc = in_pos;
    out_uv = ndc_to_uv(in_pos);

    gl_Position = vec4(in_pos, 0.0, 1.0);
}
