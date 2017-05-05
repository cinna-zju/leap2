#version 400 compatibility

//此处两个location应当与程序当中绑定VBO时的参数对应
layout (location = 0) in vec2 VertexPosition;
layout (location = 1) in vec2 VertexTexcoord;

out vec2 vTexCoord;

void main()
{
	vTexCoord = VertexTexcoord;

	gl_Position = ftransform();		//不需要做任何变换，这个内置函数最简便

}