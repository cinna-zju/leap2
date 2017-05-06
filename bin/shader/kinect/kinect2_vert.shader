#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexRGB;

// Output data ; will be interpolated for each fragment.
out vec3 vertexColor;

// Values that stay constant for the whole mesh.
uniform mat4 ProjectionMatrix;
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;

void main(){

	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(vertexPosition_modelspace,1);
	
	// UV of the vertex. No special space for this one.
	vertexColor = vertexRGB;
}

