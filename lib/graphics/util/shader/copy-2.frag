#version 460

layout(location = 0) out vec2 out_color;

layout(set = 2, binding = 0) uniform sampler2D in_texture;

void main()
{
    out_color = texelFetch(in_texture, ivec2(floor(gl_FragCoord.xy)), 0).rg;
}
