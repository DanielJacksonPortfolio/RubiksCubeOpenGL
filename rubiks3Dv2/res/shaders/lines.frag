#version 330 core
out vec4 FragColor;

in vec3 FragPos;

uniform vec3 lineColor;

void main()
{    
    // properties
    FragColor = vec4(lineColor, 1.0);
}