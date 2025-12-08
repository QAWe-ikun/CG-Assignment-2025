// Tonemapping Fragment Shader

#version 460

#extension GL_GOOGLE_include_directive : enable
#include "agx.glsl"

layout(location = 0) in vec2 uv;
layout(location = 1) in vec2 ndc;

layout(location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D light_buffer_tex;
// TODO: Bloom texture

layout(std430, set = 2, binding = 1) readonly buffer Auto_exposure
{
    float avg_luminance;
};

layout(std140, set = 3, binding = 0) uniform Parameter
{
    float exposure;
};

void main()
{
    float exposure_mult = (1.0 / 9.6) / avg_luminance * exposure;

    vec3 hdr_color = textureLod(light_buffer_tex, uv, 0).rgb * exposure_mult;
    vec3 srgb_color = agx(hdr_color);
    out_color = vec4(srgb_color, 1.0);
}
