#version 400

uniform sampler2D sampler0;

uniform int thread;
uniform int mode331;uniform int mask331[3*3*1];
//uniform int mode116;uniform int mask116[1*1*6];
//uniform int mode225;uniform int mask225[2*2*5];
//uniform int mode334;uniform int mask334[3*3*4];
//uniform int mode443;uniform int mask443[4*4*3];
//uniform int mode552;uniform int mask552[5*5*2];
//uniform int mode661;uniform int mask661[6*6*1];

in vec2 vTexCoord;
in vec2 vPosCoord;

out vec4 FragColor;
void main()
{
	int x = int(384*(vPosCoord.x + 1));
	int y = int(384*(vPosCoord.y + 1));

	vec4 tex=texture2D(sampler0, vTexCoord);

	vec3 rgb;
	float t;
	//t=(mask223[2 * (y % 2) + x % 2 + 2*2*mode223] + 1);
	t=(mask331[3 * (y % 3) + x % 3 + 3*3*mode331] + 1);
	//t=(mask116[1 * (y % 1) + x % 1 + 1*1*mode116] + 1);
	//t=(mask225[2 * (y % 2) + x % 2 + 2*2*mode225] + 1);
	//t=(mask334[3 * (y % 3) + x % 3 + 3*3*mode334] + 1);
	//t=(mask443[4 * (y % 4) + x % 4 + 4*4*mode443] + 1);
	//t=(mask552[5 * (y % 5) + x % 5 + 5*5*mode552] + 1);
	//t=(mask661[6 * (y % 6) + x % 6 + 6*6*mode661] + 1);
	t /= 255.0;
	rgb=step(t, tex.rgb);

	switch(thread)
	{
		case 0: FragColor=vec4(rgb.r, 0.0, 0.0, 1.0); break;
		case 1: FragColor=vec4(rgb.g, 0.0, 0.0, 1.0); break;
		case 2: FragColor=vec4(rgb.b, 0.0, 0.0, 1.0); break;
		default: FragColor=vec4(rgb, 1.0); break;
	}
	//FragColor=vec4(rgb, 1.0);//vec4(1.0, 0.0, 0.0, 1.0);
}