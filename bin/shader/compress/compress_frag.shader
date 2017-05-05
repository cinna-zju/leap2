#version 400

//传入的三张纹理
uniform sampler2D FBOTex0;
uniform sampler2D FBOTex1;
uniform sampler2D FBOTex2;


//传入的颜色标记
uniform int ColorState;

in  vec2 vTexCoord;

out vec4 FragColor;

void main()
{
	vec2 cTexCoord;
	vec4 tempColor;
	int i;
	int po[8];	//按位压缩
	//po[0]=128; po[1]=64; po[2]=32; po[3]=16; po[4]=8; po[5]=4; po[6]=2; po[7]=1;

	FragColor = vec4(0.0);


	if(ColorState==1)	//绿色通道
	{
		cTexCoord=vec2(1.0/8-vTexCoord.x, 1.0-vTexCoord.y);
		po[7]=128; po[6]=64; po[5]=32; po[4]=16; po[3]=8; po[2]=4; po[1]=2; po[0]=1;
	}
	else	//红蓝通道，上下翻转
	{
		cTexCoord=vec2(vTexCoord.x, 1.0-vTexCoord.y);
		po[0]=128; po[1]=64; po[2]=32; po[3]=16; po[4]=8; po[5]=4; po[6]=2; po[7]=1;
	}

	float offset = (cTexCoord.x*768-0.5)*8+0.5;		//计算压缩时的纹理偏移量

	vec4 vec4_tmp0, vec4_tmp1, vec4_tmp2;

	for(i=0; i<8; i++)
	{
		vec4_tmp0 = texture2D(FBOTex0, vec2((offset + i) / 768, cTexCoord.y));
		tempColor = vec4_tmp0 * vec4(po[i])/255.0;
		FragColor.r += tempColor.r;

		vec4_tmp2 = texture2D(FBOTex2, vec2((offset + i) / 768, cTexCoord.y));
		tempColor = vec4_tmp2 * vec4(po[i])/255.0;
		FragColor.g += tempColor.r;

		vec4_tmp1 = texture2D(FBOTex1, vec2((offset + i) / 768, cTexCoord.y));
		tempColor = vec4_tmp1 * vec4(po[i])/255.0;
		FragColor.b += tempColor.r;	
		
	}

	FragColor.a=1.0;	//alpha通道姑且设为1.0（其实没有意义，因为在程序里是按GL_RGB存储的纹理，压根就没有alpha通道）
}