#version 400 compatibility

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
out vec2 UV;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 ProjectionMatrix;
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;

uniform int view;
uniform float Pheight;
uniform float Vradius;
uniform float Vheight;


const float	PI=3.14159265;	// Ô²ÖÜÂÊ
const int  Nv=600;


out vec2 vTexCoord;
out vec2 vPosCoord;


vec3 transform_proj(vec3 Q, vec3 P)
{
	//float Vheight=530;

	vec3 A;
	vec3 V;
	vec3 S;

	A.x = Q.x - 1.0*(P.x-Q.x)/(P.y-Q.y)*Q.y;
	A.y = 0;
	A.z = Q.z - 1.0*(P.z-Q.z)/(P.y-Q.y)*Q.y;

	float alpha = 6.28/600*view;
	float m1 = Q.y*sin(alpha);
	float m2 = 1.0*(P.z-Q.z)/(P.y-Q.y)*Q.y*cos(alpha) - 1.0*(P.x-Q.x)/(P.y-Q.y)*Q.y*sin(alpha);
	float m3 = -Q.y*cos(alpha);
	float m4 = -m1*Q.x - m2*Q.y - m3*Q.z + m2*Vheight;

	V.y = Vheight;
	
	/////////ÈôÊÓµãVµÄ¹ì¼£ÊÇÒ»¸öÔ²/////////////////////////////////////////////////////

	float tmp1x = 1.0*(-m1*m4 + sqrt(m1*m1*m4*m4 - (m1*m1+m3*m3)*(m4*m4-m3*m3*Vradius*Vradius)))/(m1*m1+m3*m3);
	float tmp2x = 1.0*(-m1*m4 - sqrt(m1*m1*m4*m4 - (m1*m1+m3*m3)*(m4*m4-m3*m3*Vradius*Vradius)))/(m1*m1+m3*m3);
	float tmp1z = -1.0*(m1*tmp1x+m4)/m3;
	float tmp2z = -1.0*(m1*tmp2x+m4)/m3;
	if( (tmp1x-Q.x)*cos(alpha) + (tmp1z-Q.z)*sin(alpha)>0)
	{
		V.x = tmp1x;
		V.z = tmp1z;
	}
	else
	{
		V.x = tmp2x;
		V.z = tmp2z;
	}


	/////////¼ÆËãÆÁÄ»µã/////////////////////////////////////////////////////
	S.x = Q.x - 1.0*(V.x - Q.x) / (V.y - Q.y)*Q.y;
	S.y = Q.y;
	S.z = Q.z - 1.0*(V.z - Q.z) / (V.y - Q.y)*Q.y;

	/*S.x = Q.x ;
	S.y = Q.y;
	S.z = Q.z ;*/

//
	return S;
}


void main()		
{
	vTexCoord = vertexUV;

	vec3 Q = vec3(ModelMatrix * vec4(vertexPosition_modelspace, 1));

	vec3 P;
	P.x=0;	P.y=-Pheight;	P.z=0;
	Q=transform_proj(Q, P);		

	gl_Position = ProjectionMatrix * ViewMatrix * vec4(Q, 1.0);
	
	vPosCoord = gl_Position.xy;
}