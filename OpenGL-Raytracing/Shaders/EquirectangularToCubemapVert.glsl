#version 450
layout (location = 0) in vec3 position;

out vec3 vs_position;

uniform mat4 u_projection;
uniform mat4 u_view;

void main()
{
    vs_position = position;  
    gl_Position =  u_projection * u_view * vec4(vs_position, 1.0);
}