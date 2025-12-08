#version 460

layout(location = 0) out vec4 out_color;
layout(set = 2, binding = 0) uniform sampler2D input_texture;

layout(set = 3, binding = 0) uniform Params
{
    uvec2 render_size;
} params;

void main()
{
    vec2 uv = (vec2(gl_FragCoord.xy) + 0.5) / vec2(params.render_size);
    vec3 color = textureLod(input_texture, uv, 0).rrr;
    out_color = vec4(color, 1.0);
}
