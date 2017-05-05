#version 400

uniform sampler2D sampler0;

uniform int thread;
uniform int mode331;uniform int mask331[3*3*1];

in vec2 vTexCoord;
in vec2 vPosCoord;

out vec4 FragColor;
void main()
{
	int x = int(384*(vPosCoord.x + 1));
	int y = int(384*(vPosCoord.y + 1));
	
	vec4 tex=texture2D(sampler0, vTexCoord);

	/*switch (thread)
	{
	case 0: FragColor = vec4(tex.r, 0.0, 0.0, 1.0); break;
	case 1: FragColor = vec4(tex.g, 0.0, 0.0, 1.0); break;
	case 2: FragColor = vec4(tex.b, 0.0, 0.0, 1.0); break;
	}*/

	//FragColor = tex;
	vec3 rgb;
	float t;
	t=(mask331[3 * (y % 3) + x % 3 + 3*3*mode331] + 1);
	t /= 255.0;
	rgb=step(t, tex.rgb);
	switch(thread)
	{
		case 0: FragColor=vec4(rgb.r, 0.0, 0.0, 1.0); break;
		case 1: FragColor=vec4(rgb.g, 0.0, 0.0, 1.0); break;
		case 2: FragColor=vec4(rgb.b, 0.0, 0.0, 1.0); break;
	}
}