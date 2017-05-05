#version 330 core

// Interpolated values from the vertex shaders
in vec3 vertexColor;

// Ouput data
out vec3 color;


void main(){

	color = vertexColor;

}