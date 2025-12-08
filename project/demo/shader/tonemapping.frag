// Tonemapping Fragment Shader

#version 460

#extension GL_GOOGLE_include_directive : enable
#include "include/tonemap/agx.glsl"

layout(location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D light_buffer_tex;
// TODO: Bloom texture

layout(std140, set = 3, binding = 0) uniform Parameter
{
    float exposure_multiplier;
} param;

void main()
{
    ivec2 fragcoord = ivec2(floor(gl_FragCoord.xy));
    vec3 hdr_color = texelFetch(light_buffer_tex, fragcoord, 0).rgb * param.exposure_multiplier;
    vec3 ldr_color = agx(hdr_color);
    vec3 srgb_color = pow(ldr_color, vec3(1.0 / 2.2));
    out_color = vec4(srgb_color, 1.0);
}
