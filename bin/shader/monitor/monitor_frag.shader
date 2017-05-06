#version 330 core

// Interpolated values from the vertex shaders
in vec2 color_uv;
//in vec3 color_rgb;
in vec2 vPosCoord;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D sampler0;

void main(){

	vec3 MaterialDiffuseColor = texture( sampler0, color_uv).rgb;

	color = MaterialDiffuseColor;

	//color =  color_rgb;
}