#pragma once
#include "PointCloud_process.h"

HDC		hDC1 = NULL;	HGLRC	hRC1 = NULL;	HWND	hWnd1 = NULL;
static const int ThreadNum = 3;
HDC			hDC[3];
HGLRC		hRC[3];
HWND		hWnd[3];
bool thread0_flag, thread_flag[ThreadNum];
HANDLE hEvent[ThreadNum];
HANDLE hSwap0, hSwap[ThreadNum];
int ThreadParam[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

GLuint indexFBO;
GLuint FBOTex[3];

GLuint quadVAO[2];	//顶点缓存对象

float quadVBO[8 * 768];	//储存每个矩形的顶点坐标。注意只提供了xy，z坐标需要在shader里另外加上
float quadUV[8 * 768];	//储存每个矩形的纹理坐标

GLuint ShaderCompressID, ShaderProjID;

float Vradius = 500, Vheight = 800, Pheight = 200;

int mask331[3 * 3 * 1];

void RandDither(int *a, int M, int T)
{
	/*int i, j;
	srand(6543785);
	for (i = 0; i<T*M*M; i++)
	{
		a[i] = rand() % (T*M*M);
		int flag = 1;
		while (flag == 1)//只要有重复的就再随机新的数
		{
			for (j = 0; j<i; j++)
				if (a[i] == a[j])
					break;
			if (j<i)
				a[i] = rand() % (T*M*M);
			if (j == i)
				flag = 0;
		}

	}
	for (i = 0; i<T*M*M; i++)
		a[i] = 256.0 / M / M / T*a[i];*/


	for (int i = 0; i<T*M*M; i++)
		a[i] = 256.0 / M / M / T*i;
}


void initFBO()
{
	glGenFramebuffersEXT(1, &indexFBO);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, indexFBO);
	GLuint rboId;
	glGenRenderbuffersEXT(1, &rboId);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rboId);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, 768, 768);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, rboId);
	//GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	//if (status == GL_FRAMEBUFFER_COMPLETE_EXT)
	//	MessageBox(NULL, _T("FBO success"), _T("FBO success"), MB_OK | MB_ICONEXCLAMATION);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glGenTextures(3, FBOTex);          //创建纹理i
	for (int i = 0; i<3; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, FBOTex[i]);        // 绑定纹理
														 //纹理的显示方式
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 768, 768, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	}

	//生成顶点和纹理坐标。每两个数字表示一个点的坐标，注意shader里要将z坐标补上0.0
	for (int i = 0; i < 768; ++i)
	{
		float* vert = quadVBO + 8 * i;
		vert[0] = (1024 * ((767 - i) % 24) + 128) / 8; vert[1] = i / 24;	//要水平翻转纹理，可以将此处vert[0]和vert[2]调换一下
		vert[2] = (1024 * ((767 - i) % 24) + 128 + 768) / 8; vert[3] = vert[1];
		vert[4] = vert[2]; vert[5] = vert[1] + 1;
		vert[6] = vert[0]; vert[7] = vert[5];

		float* tex = quadUV + 8 * i;	//纹理坐标不要轻易修改
		tex[0] = 0.0; tex[1] = i / 768.0;
		tex[2] = 1.0 / 8; tex[3] = tex[1];
		tex[4] = tex[2]; tex[5] = tex[1] + 1 / 768.0;
		tex[6] = tex[0]; tex[7] = tex[5];
	}

	//启用顶点和纹理坐标
	glGenBuffers(2, quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVAO[0]);
	glBufferData(GL_ARRAY_BUFFER, 8 * 768 * sizeof(float), quadVBO, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, quadVAO[1]);
	glBufferData(GL_ARRAY_BUFFER, 8 * 768 * sizeof(float), quadUV, GL_STATIC_DRAW);

}


void initModel()
{
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//bool res = loadAssImp("model\\earth_small\\earth_small.obj", PointCloudIndices, PointCloudVertex, PointCloudUV, PointCloudNormal);

	bool res = loadAssImp("model\\baymax\\baymax.obj", PointCloudIndices, PointCloudVertex, PointCloudUV, PointCloudNormal);

	//cv::Mat img_lp = cv::imread("model\\earth_small\\earth.png");

	cv::Mat img_lp = cv::imread("model\\baymax\\bm.jpg");
	//cv::flip(img_lp, img_lp, -1);
	cv::flip(img_lp, img_lp, 0);
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_2D, mTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_lp.cols, img_lp.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img_lp.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D, 0);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	glGenBuffers(1, &vertex_buffer);	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, PointCloudVertex.size() * sizeof(glm::vec3), PointCloudVertex.data(), GL_DYNAMIC_DRAW);	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &uv_buffer);	glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
	glBufferData(GL_ARRAY_BUFFER, PointCloudUV.size() * sizeof(glm::vec2), PointCloudUV.data(), GL_DYNAMIC_DRAW);	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &index_buffer);	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, PointCloudIndices.size() * sizeof(unsigned short), PointCloudIndices.data(), GL_DYNAMIC_DRAW);	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void StaticVBO_inFBO(int view, int ThreadFlag, int id)
{
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, FBOTex[id], 0); //将纹理1绑定到FBO的3个颜色位

	glViewport(0, 0, 768, 768);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	glUseProgram(ShaderProjID);

	setAdvanceMVP();

	glUniformMatrix4fv(glGetUniformLocation(ShaderProjID, "ProjectionMatrix"), 1, GL_FALSE, &mProjectionMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(ShaderProjID, "ModelMatrix"), 1, GL_FALSE, &mModelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(ShaderProjID, "ViewMatrix"), 1, GL_FALSE, &mViewMatrix[0][0]);

	glUniform1i(glGetUniformLocation(ShaderProjID, "view"), view + 300 + 0);	//传入投影机距离
	glUniform1i(glGetUniformLocation(ShaderProjID, "thread"), ThreadFlag);
	glUniform1f(glGetUniformLocation(ShaderProjID, "Pheight"), Pheight);	//传入投影机距离
	glUniform1f(glGetUniformLocation(ShaderProjID, "Vradius"), Vradius);	//传入视点半径
	glUniform1f(glGetUniformLocation(ShaderProjID, "Vheight"), Vheight);	//传入视点高度
	glUniform1iv(glGetUniformLocation(ShaderProjID, "mask331"), 3 * 3 * 1, mask331);
	glUniform1i(glGetUniformLocation(ShaderProjID, "mode331"), 0);

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);		glBindTexture(GL_TEXTURE_2D, mTexture);
	glUniform1i(glGetUniformLocation(ShaderProjID, "sampler0"), 0);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);		glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	//glDrawElements(GL_TRIANGLES, PointCloudIndices.size(), GL_UNSIGNED_SHORT, (void*)0);
	if (which_device == -1)
		glDrawElements(GL_TRIANGLES, PointCloudIndices3[ThreadFlag].size(), GL_UNSIGNED_SHORT, (void*)0);
	else
		glDrawElements(GL_QUADS, PointCloudIndices3[ThreadFlag].size(), GL_UNSIGNED_SHORT, (void*)0);

	glDisableVertexAttribArray(0);		glDisableVertexAttribArray(1);
}

void drawFBO_forFPGA(int state)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 3072, 0, 32);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//int state=cs.color;
	glUseProgram(ShaderCompressID);
	{
		glUniform1i(glGetUniformLocation(ShaderCompressID, "ColorState"), state);
		//绑定顶点数组
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, quadVAO[0]);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);	

																
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, quadVAO[1]);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);	

		glUniform1i(glGetUniformLocation(ShaderCompressID, "FBOTex0"), 0);
		glUniform1i(glGetUniformLocation(ShaderCompressID, "FBOTex1"), 1);
		glUniform1i(glGetUniformLocation(ShaderCompressID, "FBOTex2"), 2);

		glUniform1iv(glGetUniformLocation(ShaderCompressID, "mask331"), 3 * 3 * 1, mask331);
		glUniform1i(glGetUniformLocation(ShaderCompressID, "mode331"), 0);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, FBOTex[2]);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, FBOTex[1]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, FBOTex[0]);

		glDrawArrays(GL_QUADS, 0, 4 * 768);	//在shader里根据顶点数组绘制
	}
	glUseProgram(0);
}

void drawEntire_forFPGA(int ThreadFlag)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	
	if (which_device == 0 || which_device == 1)
	{
		PointCloudProcess3(ThreadFlag);
	}
	else if (which_device == 2)
	{
		PointCloudProcess_RS3(ThreadFlag);
	}

	if (which_device != -1)
	{
		//想实时更新图像时，前面加model_update，并Dynamic，后面glDelete
		glDeleteBuffers(1, &vertex_buffer);	glDeleteBuffers(1, &uv_buffer);	glDeleteBuffers(1, &index_buffer);		glDeleteTextures(1, &mTexture);
	
		glGenBuffers(1, &vertex_buffer);
		glGenBuffers(1, &uv_buffer);
		glGenBuffers(1, &index_buffer);

		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, (PointCloudVertex3[ThreadFlag].size())*sizeof(glm::vec3), PointCloudVertex3[ThreadFlag].data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
		glBufferData(GL_ARRAY_BUFFER, (PointCloudUV3[ThreadFlag].size())*sizeof(glm::vec2), PointCloudUV3[ThreadFlag].data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (PointCloudIndices3[ThreadFlag].size())*sizeof(unsigned short), PointCloudIndices3[ThreadFlag].data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		//colorImg.copyTo(img_lp);
		flip(colorImg, texMat, 1);
		////imshow("colorImg_seg", img_lp);
		glGenTextures(1, &mTexture);
		glBindTexture(GL_TEXTURE_2D, mTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texMat.cols, texMat.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, texMat.data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D, 0);
	}
	//else
	//{
	//	glDeleteBuffers(1, &vertex_buffer);	glDeleteBuffers(1, &uv_buffer);	glDeleteBuffers(1, &index_buffer);		glDeleteTextures(1, &mTexture);
	//	bool res = loadAssImp("model\\earth_small\\earth_small.obj", PointCloudIndices3[ThreadFlag], PointCloudVertex3[ThreadFlag], PointCloudUV3[ThreadFlag], PointCloudNormal3[ThreadFlag]);
	//	cv::Mat img_lp = cv::imread("model\\earth_small\\earth.png");
	//	//cv::flip(img_lp, img_lp, -1);
	//	cv::flip(img_lp, img_lp, 0);
	//	glGenTextures(1, &mTexture);
	//	glBindTexture(GL_TEXTURE_2D, mTexture);
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_lp.cols, img_lp.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img_lp.data);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//	glGenerateMipmap(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D, 0);

	//	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	glGenBuffers(1, &vertex_buffer);	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	//	glBufferData(GL_ARRAY_BUFFER, PointCloudVertex3[ThreadFlag].size() * sizeof(glm::vec3), PointCloudVertex3[ThreadFlag].data(), GL_DYNAMIC_DRAW);	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//	glGenBuffers(1, &uv_buffer);	glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
	//	glBufferData(GL_ARRAY_BUFFER, PointCloudUV3[ThreadFlag].size() * sizeof(glm::vec2), PointCloudUV3[ThreadFlag].data(), GL_DYNAMIC_DRAW);	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//	glGenBuffers(1, &index_buffer);	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, PointCloudIndices3[ThreadFlag].size() * sizeof(unsigned short), PointCloudIndices3[ThreadFlag].data(), GL_DYNAMIC_DRAW);	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//}

	for (int i = 0; i<100; i++)
	{
		glEnable(GL_DEPTH_TEST);	glDisable(GL_BLEND);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, indexFBO);	//绑定FBO
		{
			glEnable(GL_DEPTH_TEST);
			StaticVBO_inFBO(200 + i, ThreadFlag, 0); StaticVBO_inFBO(0 + i, ThreadFlag, 1);	StaticVBO_inFBO(100 + i, ThreadFlag, 2);
		}
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);	//关闭FBO
		glEnable(GL_BLEND);			glBlendFunc(GL_ONE, GL_ONE);		glDisable(GL_DEPTH_TEST);
		glViewport(0, i * 32, 3072, 32);	//
		drawFBO_forFPGA(ThreadFlag);  //
		glViewport(3072, i * 32, 3072, 32);	//
		drawFBO_forFPGA(ThreadFlag);  //	
	}
	glFinish();

	
}

void ThreadSyncFunc()
{

	if (which_device == 2)
	{
		//rs::log_to_console(rs::log_severity::warn);
		//dev = ctx.get_device(0);
		//// best_quality 640 x 480
		//dev->enable_stream(rs::stream::depth, rs::preset::best_quality);
		//dev->enable_stream(rs::stream::color, rs::preset::best_quality);
		////dev->enable_stream(rs::stream::color, rs::preset::largest_image);
		//dev->start();
	}

	thread0_flag = true;
	MSG		msg;									// Windowsx消息结构
	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	// 有消息在等待吗?
		{
			if (msg.message == WM_QUIT)				// 收到退出消息?
			{
				run_flag = false;							// 是，则done=true
			}
			else									// 不是，处理窗口消息
			{
				TranslateMessage(&msg);				// 翻译消息
				DispatchMessage(&msg);				// 发送消息
			}
		}

		else										// 如果没有消息
		{

			if (active)								// 程序激活的么?
			{
				if (keys[VK_ESCAPE])				// ESC 按下了么?
				{
					run_flag = false;
				}
				else								// 不是退出的时候，刷新屏幕
				{
					WaitForMultipleObjects(ThreadNum, hEvent, TRUE, INFINITE);
					for (int i = 0; i<ThreadNum; i++)
						SetEvent(hSwap[i]);
					keyboard_interaction();

				}
			}
		}
		
	}
	thread0_flag = false;


	// 关闭程序
	FreeConsole();
}

int Card_ID[3] = { 1, 2, 3 };
void ThreadFunc(void* lpParameter)
{
	int ThreadFlag = *(int*)lpParameter;
	thread_flag[ThreadFlag] = true;
	MSG		msg;									// Windowsx消息结构
	EnterCriticalSection(&cs);
	if (!CreateGLWindow(L"Display", 1440, 0 + 3200 * ThreadFlag, 6144, 3200, 32, hWnd[ThreadFlag], hDC[ThreadFlag], hRC[ThreadFlag], Card_ID[ThreadFlag])) //ThreadFlag+1))
	//if (!CreateGLWindow(L"Display", 0, 0 , 768, 768, 32, hWnd[ThreadFlag], hDC[ThreadFlag], hRC[ThreadFlag], Card_ID[ThreadFlag])) //ThreadFlag+1))
	{
		printf("Window%d error\n", ThreadFlag);
	}

	initFBO();
	if (which_device == -1)
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//bool res = loadAssImp("model\\earth_small\\earth_small.obj", PointCloudIndices3[ThreadFlag], PointCloudVertex3[ThreadFlag], PointCloudUV3[ThreadFlag], PointCloudNormal3[ThreadFlag]);

		bool res = loadAssImp("model\\baymax\\baymax.obj", PointCloudIndices3[ThreadFlag], PointCloudVertex3[ThreadFlag], PointCloudUV3[ThreadFlag], PointCloudNormal3[ThreadFlag]);
		cv::Mat img_lp = cv::imread("model\\baymax\\bm.jpg");
		//cv::flip(img_lp, img_lp, -1);
		cv::flip(img_lp, img_lp, 0);
		glGenTextures(1, &mTexture);
		glBindTexture(GL_TEXTURE_2D, mTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_lp.cols, img_lp.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img_lp.data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D, 0);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		glGenBuffers(1, &vertex_buffer);	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, PointCloudVertex3[ThreadFlag].size() * sizeof(glm::vec3), PointCloudVertex3[ThreadFlag].data(), GL_DYNAMIC_DRAW);	glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &uv_buffer);	glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
		glBufferData(GL_ARRAY_BUFFER, PointCloudUV3[ThreadFlag].size() * sizeof(glm::vec2), PointCloudUV3[ThreadFlag].data(), GL_DYNAMIC_DRAW);	glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &index_buffer);	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, PointCloudIndices3[ThreadFlag].size() * sizeof(unsigned short), PointCloudIndices3[ThreadFlag].data(), GL_DYNAMIC_DRAW);	glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	ShaderCompressID = LoadShaders("shader\\compress_vert.shader", "shader\\compress_frag.shader");
	ShaderProjID = LoadShaders("shader\\proj_vert.shader", "shader\\proj_frag.shader");
	//ShaderProjID = LoadShaders("shader\\monitor_vert.shader", "shader\\monitor_frag.shader");

	//ModelShaderLoading();

	LeaveCriticalSection(&cs);

	while (1)
	{

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	// 有消息在等待吗?
		{
			if (msg.message == WM_QUIT)				// 收到退出消息?
			{
				run_flag = false;							// 是，则done=true
			}
			else									// 不是，处理窗口消息
			{
				TranslateMessage(&msg);				// 翻译消息
				DispatchMessage(&msg);				// 发送消息
			}
		}
		else										// 如果没有消息
		{
			
			if (keys[VK_ESCAPE])				// ESC 按下了么?
			{
				break;
			}
			else								// 不是退出的时候，刷新屏幕
			{
				glClear(GL_COLOR_BUFFER_BIT);
				drawEntire_forFPGA(ThreadFlag);
				SetEvent(hEvent[ThreadFlag]);
				WaitForSingleObject(hSwap[ThreadFlag], INFINITE);
				SwapBuffers(hDC[ThreadFlag]);
			}
			
		}
	}
	KillGLWindow(hWnd[ThreadFlag], hDC[ThreadFlag], hRC[ThreadFlag]);
	thread_flag[ThreadFlag] = false;
}

void GLDMD()
{
	for (int i = 0; i< ThreadNum; i++)
	{
		hEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		hSwap[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
	}
	HANDLE hThread0, hThread[ThreadNum];
	DWORD ThreadID0, ThreadID[ThreadNum];

	RandDither(mask331, 3, 1);

	/*RandDither(mask112, 1, 2);	RandDither(mask113, 1, 3);	RandDither(mask114, 1, 4);	RandDither(mask115, 1, 5);	RandDither(mask116, 1, 6);
	RandDither(mask221, 2, 1);	RandDither(mask222, 2, 2);	RandDither(mask223, 2, 3);	RandDither(mask224, 2, 4);	RandDither(mask225, 2, 5);	RandDither(mask226, 2, 6);
	RandDither(mask331, 3, 1);	RandDither(mask332, 3, 2);	RandDither(mask333, 3, 3);	RandDither(mask334, 3, 4);	RandDither(mask335, 3, 5);	RandDither(mask336, 3, 6);
	RandDither(mask441, 4, 1);	RandDither(mask442, 4, 2);	RandDither(mask443, 4, 3);	RandDither(mask444, 4, 4);	RandDither(mask445, 4, 5);	RandDither(mask446, 4, 6);
	RandDither(mask551, 5, 1);	RandDither(mask552, 5, 2);	RandDither(mask553, 5, 3);	RandDither(mask554, 5, 4);	RandDither(mask555, 5, 5);	RandDither(mask556, 5, 6);
	RandDither(mask661, 6, 1);	RandDither(mask662, 6, 2);	RandDither(mask663, 6, 3);	RandDither(mask664, 6, 4);	RandDither(mask665, 6, 5);	RandDither(mask666, 6, 6);*/

	if(which_device != -1)
		m_model_scale = 0.1;

	InitializeCriticalSection(&cs);
	hThread0 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadSyncFunc, NULL, 0, &ThreadID0);

	for (int i = 0; i< ThreadNum; i++)
		hThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFunc, (void*)(&ThreadParam[i]), 0, &ThreadID[i]);

}