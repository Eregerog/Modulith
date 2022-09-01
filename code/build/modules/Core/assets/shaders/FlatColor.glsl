#type vertex

#version 330 core

layout(location = 0)in vec3 a_Position;

uniform mat4 u_P;
uniform mat4 u_V;
uniform mat4 u_M;

void main(){
    gl_Position = u_P * u_V * u_M * vec4(a_Position, 1.0);
}


    #type fragment

    #version 330 core

layout(location = 0) out vec4 color;

uniform vec4 u_Color;

void main(){
    color = u_Color;
}