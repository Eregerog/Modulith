#type vertex

#version 330 core

layout(location = 0)in vec3 a_Position;

varying vec3 v_Position;

uniform mat4 u_PV;
uniform mat4 u_M;

void main(){
    v_Position = a_Position;
    gl_Position = u_PV * u_M * vec4(a_Position, 1.0);
}


    #type fragment

    #version 330 core

layout(location = 0) out vec4 color;

varying vec3 v_Position;
uniform vec4 u_Color;

void main(){
    color = vec4(v_Position + 0.5, 1.0);
}