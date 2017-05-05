#version 400 compatibility

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
out vec2 UV;

// Values that stay constant for the whole mesh.
//uniform mat4 MVP;
uniform mat4 ProjectionMatrix;
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;

uniform int view;
uniform float Pheight;
uniform float Vradius;
uniform float Vheight;
uniform float Vr1;
uniform float Vh1;
uniform float Va1;

uniform float Vr2;
uniform float Vh2;
uniform float Va2;


const float	PI = 3.14159265;	// Ô²ÖÜÂÊ
const int  Nv = 600;


float Vheight1[2];
float Vradius1[2];
float Vangle1[2];

out vec2 vTexCoord;
out vec2 vPosCoord;

vec3 transform_proj2(vec3 Q, vec3 P)
{
	vec3 A;
	vec3 V;
	vec3 S;

	A.x = Q.x - 1.0*(P.x - Q.x) / (P.y - Q.y)*Q.y;
	A.y = 0;
	A.z = Q.z - 1.0*(P.z - Q.z) / (P.y - Q.y)*Q.y;

	float alpha = 6.28 / 600 * view;

	float m1, m2, m3, m4;
	float tmp1x, tmp2x, tmp1z, tmp2z;

	m1 = Q.y*sin(alpha);
	m2 = 1.0*(P.z - Q.z) / (P.y - Q.y)*Q.y*cos(alpha) - 1.0*(P.x - Q.x) / (P.y - Q.y)*Q.y*sin(alpha);
	m3 = -Q.y*cos(alpha);
	m4 = -m1*Q.x - m2*Q.y - m3*Q.z + m2*Vheight;

	tmp1x = 1.0*(-m1*m4 + sqrt(m1*m1*m4*m4 - (m1*m1 + m3*m3)*(m4*m4 - m3*m3*Vradius*Vradius))) / (m1*m1 + m3*m3);
	tmp2x = 1.0*(-m1*m4 - sqrt(m1*m1*m4*m4 - (m1*m1 + m3*m3)*(m4*m4 - m3*m3*Vradius*Vradius))) / (m1*m1 + m3*m3);
	tmp1z = -1.0*(m1*tmp1x + m4) / m3;
	tmp2z = -1.0*(m1*tmp2x + m4) / m3;

	if ((tmp1x - Q.x)*cos(alpha) + (tmp1z - Q.z)*sin(alpha)>0)
	{
		V.x = tmp1x;
		V.z = tmp1z;
	}
	else
	{
		V.x = tmp2x;
		V.z = tmp2z;
	}

	float base = 0;
	if (V.x>0 && V.z >= 0)				base = atan(abs(1.0*V.z / V.x));
	else if (V.x == 0 && V.z >= 0)		base = 1.57;
	else if (V.x<0 && V.z >= 0)		base = 3.14 - atan(abs(1.0*V.z / V.x));
	else if (V.x<0 && V.z<0)			base = atan(abs(1.0*V.z / V.x)) + 3.14;
	else if (V.x == 0 && V.z<0)		base = 4.71;
	else if (V.x>0 && V.z<0)			base = 6.28 - atan(abs(1.0*V.z / V.x));


	float paraH = 1.0;
	float paraR = 1.0;
	float delta = 0.4;
	if (abs(base - Vangle1[0])<delta)
	{
		paraH = 1.0*Vheight1[0] / Vheight;
		paraR = 1.0*Vradius1[0] / Vradius;
	}
	if (abs(base - Vangle1[1])<delta)
	{
		paraH = 1.0*Vheight1[1] / Vheight;
		paraR = 1.0*Vradius1[1] / Vradius;
	}
	float _Vheight = Vheight * paraH;
	float _Vradius = Vradius * paraR;// * Gau(alpha, Vradius1/Vradius);

	m1 = Q.y*sin(alpha);
	m2 = 1.0*(P.z - Q.z) / (P.y - Q.y)*Q.y*cos(alpha) - 1.0*(P.x - Q.x) / (P.y - Q.y)*Q.y*sin(alpha);
	m3 = -Q.y*cos(alpha);
	m4 = -m1*Q.x - m2*Q.y - m3*Q.z + m2*_Vheight;

	tmp1x = 1.0*(-m1*m4 + sqrt(m1*m1*m4*m4 - (m1*m1 + m3*m3)*(m4*m4 - m3*m3*_Vradius*_Vradius))) / (m1*m1 + m3*m3);
	tmp2x = 1.0*(-m1*m4 - sqrt(m1*m1*m4*m4 - (m1*m1 + m3*m3)*(m4*m4 - m3*m3*_Vradius*_Vradius))) / (m1*m1 + m3*m3);
	tmp1z = -1.0*(m1*tmp1x + m4) / m3;
	tmp2z = -1.0*(m1*tmp2x + m4) / m3;

	if ((tmp1x - Q.x)*cos(alpha) + (tmp1z - Q.z)*sin(alpha)>0)
	{
		V.x = tmp1x;
		V.z = tmp1z;
	}
	else
	{
		V.x = tmp2x;
		V.z = tmp2z;
	}
	V.y = _Vheight;

	S.x = Q.x - 1.0*(V.x - Q.x) / (V.y - Q.y)*Q.y;
	S.y = Q.y;
	S.z = Q.z - 1.0*(V.z - Q.z) / (V.y - Q.y)*Q.y;


	//
	return S;
}

void main()
{
	vTexCoord = vertexUV;

	vec3 Q = vec3(ModelMatrix * vec4(vertexPosition_modelspace, 1));

	Vheight1[0] = Vh1;			
	Vradius1[0] = Vr1;		
	Vangle1[0] = Va1;   

	Vheight1[1] = Vh2;
	Vradius1[1] = Vr2;
	Vangle1[1] = Va2;



	vec3 P;
	P.x=0;	P.y=-Pheight;	P.z=0;
	Q=transform_proj2(Q, P);		

	gl_Position = ProjectionMatrix * ViewMatrix * vec4(Q, 1.0);

	vPosCoord = gl_Position.xy;
}