#version 460

layout(location = 0) in vec2 uv;
layout(location = 1) in vec2 ndc;

layout(location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D input_texture;

void main()
{
    vec3 color = textureLod(input_texture, uv, 0).rrr;
    out_color = vec4(color, 1.0);
}
