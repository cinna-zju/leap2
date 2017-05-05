#include "stdafx.h"
#pragma once

#include <math.h>			// 数学函数库
#include <stdlib.h>			// 标准库
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

#include	"gl/glew.h"			// 包含最新的gl.h,glu.h库
#include	"gl/wglew.h"
#include	"glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

//#include <librealsense/rs.hpp>     // real sense 

#include "leapmotion/Leap.h"

using namespace cv;
using namespace std;


int which_device = 0;
bool use_leapmotion = false;

#define COLOR_WIDTH	 640
#define COLOR_HIGHT	 480
#define DEPTH_WIDTH	 640
#define DEPTH_HIGHT	 480

HINSTANCE	hInstances[2];		// 保存程序的实例
HINSTANCE	hInstance;		// 保存程序的实例

bool	keys[256];			// 保存键盘按键的数组
bool	active = true;		// 窗口的活动标志，缺省为true

BOOL run_flag;
bool snapshot_cloud = false;
CRITICAL_SECTION cs;

vector<glm::vec3> PointCloudVertex;
vector<glm::vec3> PointCloudColor;
vector<glm::vec3> PointCloudNormal;
vector<glm::vec2> PointCloudUV;
vector<unsigned short> PointCloudIndices;

vector<vector<glm::vec3>> PointCloudVertex3(3);
vector<vector<glm::vec3>> PointCloudColor3(3);
vector<vector<glm::vec3>> PointCloudNormal3(3);
vector<vector<glm::vec2>> PointCloudUV3(3);
vector<vector<unsigned short>> PointCloudIndices3(3);

GLuint vertex_buffer, color_buffer, uv_buffer, index_buffer, mTexture;

float m_viewer_distance = -1800.0;
float m_viewer_angle_H = 0.0;
float m_viewer_angle_V = 0;
float m_model_scale = 1.0;
glm::vec3 m_translate_vec = glm::vec3(0.0, 0.0, 0.0); //-500
glm::vec3 m_rotate_vec = glm::vec3(0.0);

glm::mat4 mViewMatrix;
glm::mat4 mProjectionMatrix;
glm::mat4 mModelMatrix;

glm::mat4 mScaleMatrix;
glm::mat4 mTranslateMatrix;
glm::mat4 mRotateMatrix_x;
glm::mat4 mRotateMatrix_y;
glm::mat4 mRotateMatrix_z;

int upv = -1000;
int downv = 10000;
int d_near = 500;
int d_far = 1500;
Mat tex_test = imread("D:\\test.bmp");


Mat colorImg(COLOR_HIGHT, COLOR_WIDTH, CV_8UC3, Scalar(0));
Mat depthImgRaw(COLOR_HIGHT, COLOR_WIDTH, CV_16UC1, Scalar(0));
Mat depthImg(COLOR_HIGHT, COLOR_WIDTH, CV_8UC3, Scalar(0));
Mat depthMatIndex(COLOR_HIGHT, COLOR_WIDTH, CV_32SC1, Scalar(0));
Mat texMat(colorImg.cols, colorImg.rows, CV_8UC3, Scalar(0, 0, 0));


GLuint ShaderMonitorID;


bool loadAssImp(
	const char * path,
	std::vector<unsigned short> & indices,
	std::vector<glm::vec3> & vertices,
	std::vector<glm::vec2> & uvs,
	std::vector<glm::vec3> & normals
	) {

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, 0/*aiProcess_JoinIdenticalVertices | aiProcess_SortByPType*/);
	if (!scene) {
		fprintf(stderr, importer.GetErrorString());
		getchar();
		return false;
	}
	const aiMesh* mesh = scene->mMeshes[0]; // In this simple example code we always use the 1rst mesh (in OBJ files there is often only one anyway)

											// Fill vertices positions
	vertices.reserve(mesh->mNumVertices);
	for (unsigned int i = 0; i<mesh->mNumVertices; i++) {
		aiVector3D pos = mesh->mVertices[i];
		vertices.push_back(glm::vec3(pos.x, pos.y, pos.z));
	}

	// Fill vertices texture coordinates
	uvs.reserve(mesh->mNumVertices);
	for (unsigned int i = 0; i<mesh->mNumVertices; i++) {
		aiVector3D UVW = mesh->mTextureCoords[0][i]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
		uvs.push_back(glm::vec2(UVW.x, UVW.y));
	}

	// Fill vertices normals
	normals.reserve(mesh->mNumVertices);
	for (unsigned int i = 0; i<mesh->mNumVertices; i++) {
		aiVector3D n = mesh->mNormals[i];
		normals.push_back(glm::vec3(n.x, n.y, n.z));
	}


	// Fill face indices
	indices.reserve(3 * mesh->mNumFaces);
	for (unsigned int i = 0; i<mesh->mNumFaces; i++) {
		// Assume the model has only triangles.
		indices.push_back(mesh->mFaces[i].mIndices[0]);
		indices.push_back(mesh->mFaces[i].mIndices[1]);
		indices.push_back(mesh->mFaces[i].mIndices[2]);
	}

	// The "scene" pointer will be deleted automatically by "importer"
	return true;
}

void CreateConsole()
{
	if (!AllocConsole())
	{
		//MessageBox(NULL,L"控制台打开失败",L"控制台打开失败",MB_OK|MB_ICONEXCLAMATION);
	}
	else
	{
		HANDLE lStdHandle;
		int hConHandle;
		FILE* fp;

		lStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		hConHandle = _open_osfhandle((long)lStdHandle, _O_TEXT);
		fp = _fdopen(hConHandle, "w");
		*stdout = *fp;
		setvbuf(stdout, NULL, _IONBF, 0);

		lStdHandle = GetStdHandle(STD_INPUT_HANDLE);
		hConHandle = _open_osfhandle((long)lStdHandle, _O_TEXT);
		fp = _fdopen(hConHandle, "r");
		*stdin = *fp;
		setvbuf(stdin, NULL, _IONBF, 0);

		lStdHandle = GetStdHandle(STD_ERROR_HANDLE);
		hConHandle = _open_osfhandle((long)lStdHandle, _O_TEXT);
		fp = _fdopen(hConHandle, "w");
		*stderr = *fp;
		setvbuf(stderr, NULL, _IONBF, 0);

		printf("控制台打开成功。\n");
	}
}


LRESULT CALLBACK WndProc1(HWND	hWnd,			// 窗口的句柄
	UINT	uMsg,			// 窗口的消息
	WPARAM	wParam,			// 附加的消息内容
	LPARAM	lParam)			// 附加的消息内容
{
	switch (uMsg)									// 检查Windows消息
	{
	case WM_ACTIVATE:							// 监视窗口激活消息
	{
		if (!HIWORD(wParam))					// 检查最小化状态
		{
			active = true;						// 程序处于激活状态
		}
		else
		{
			active = false;						// 程序不再激活
		}

		return 0;								// 返回消息循环
	}

	case WM_SYSCOMMAND:							// 系统中断命令
	{
		switch (wParam)							// 检查系统调用
		{
		case SC_SCREENSAVE:					// 屏保要运行?
		case SC_MONITORPOWER:				// 显示器要进入节电模式?
			return 0;							// 阻止发生
		}
		break;									// 退出
	}

	case WM_CLOSE:								// 收到Close消息?
	{
		PostQuitMessage(0);						// 发出退出消息
		return 0;								// 返回
	}

	case WM_KEYDOWN:							// 有键按下么?
	{
		keys[wParam] = true;					// 如果是，设为true
		return 0;								// 返回
	}

	case WM_KEYUP:								// 有键放开么?
	{
		keys[wParam] = false;					// 如果是，设为false
		return 0;								// 返回
	}
	}

	// 向 DefWindowProc传递所有未处理的消息。
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool myRegisterClass()
{
	WNDCLASS	wc;						// 窗口类结构

	hInstance = GetModuleHandle(NULL);				// 取得我们窗口的实例
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// 移动时重画，并为窗口取得DC
	wc.lpfnWndProc = (WNDPROC)WndProc1;					// WndProc处理消息
	wc.cbClsExtra = 0;									// 无额外窗口数据
	wc.cbWndExtra = 0;									// 无额外窗口数据
	wc.hInstance = hInstance;							// 设置实例
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);			// 装入缺省图标
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);			// 装入鼠标指针
	wc.hbrBackground = NULL;									// GL不需要背景
	wc.lpszMenuName = NULL;									// 不需要菜单
	wc.lpszClassName = _T("OpenG");								// 设定类名字

	return RegisterClass(&wc);									// 尝试注册窗口类
}

void KillGLWindow(HWND hWnd, HDC& hDC, HGLRC& hRC)								// 正常销毁窗口
{
	wglMakeCurrent(hDC, hRC);
	if (hRC)											//我们拥有OpenGL描述表吗?
	{
		if (!wglMakeCurrent(NULL, NULL))					// 我们能否释放DC和RC描述表?
		{
			MessageBox(NULL, _T("释放DC或RC失败。"), _T("关闭错误"), MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// 我们能否删除RC?
		{
			MessageBox(NULL, _T("释放RC失败。"), _T("关闭错误"), MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;										// 将RC设为 NULL
	}

	if (hDC && !ReleaseDC(hWnd, hDC))					// 我们能否释放 DC?
	{
		MessageBox(NULL, _T("释放DC失败。"), _T("关闭错误"), MB_OK | MB_ICONINFORMATION);
		hDC = NULL;										// 将 DC 设为 NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// 能否销毁窗口?
	{
		MessageBox(NULL, _T("释放窗口句柄失败。"), _T("关闭错误"), MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;										// 将 hWnd 设为 NULL
	}
}

void KillInstance()
{
	if (!UnregisterClass(_T("OpenG"), hInstance))			// 能否注销类?
	{
		MessageBox(NULL, _T("不能注销窗口类。"), _T("关闭错误"), MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;									// 将 hInstance 设为 NULL
	}
}

bool CreateGLWindow(LPCWSTR title,
	int pos_x, int pos_y, int width, int height, int bits,
	HWND& hWnd, HDC& hDC, HGLRC& hRC, int RCnum)
{
	bool caption = false;
	GLuint		PixelFormat;			// 保存查找匹配的结果
	DWORD		dwExStyle;				// 扩展窗口风格
	DWORD		dwStyle;				// 窗口风格
	RECT		WindowRect;				// 取得矩形的左上角和右下角的坐标值
	WindowRect.left = (long)0;			// 将Left   设为 0
	WindowRect.right = (long)width;		// 将Right  设为要求的宽度
	WindowRect.top = (long)0;				// 将Top    设为 0
	WindowRect.bottom = (long)height;		// 将Bottom 设为要求的高度

	dwExStyle = WS_EX_APPWINDOW;			// 扩展窗体风格
	dwStyle = WS_POPUP;							// 窗体风格

	if (caption)	//要求标题栏
	{
		dwStyle = WS_CAPTION;//WS_OVERLAPPEDWINDOW;							// 窗体风格
	}
	AdjustWindowRectEx(&WindowRect, dwStyle, false, dwExStyle);		// 调整窗口达到真正要求的大小

																	// 创建窗口
	if (!(hWnd = CreateWindowEx(dwExStyle,							// 扩展窗体风格
		_T("OpenG"),							// 类名字
		(title),								// 窗口标题
		dwStyle |							// 必须的窗体风格属性
		WS_CLIPSIBLINGS |					// 必须的窗体风格属性
		WS_CLIPCHILDREN,					// 必须的窗体风格属性
		pos_x, pos_y,								// 窗口位置
		WindowRect.right - WindowRect.left,	// 计算调整好的窗口宽度
		WindowRect.bottom - WindowRect.top,	// 计算调整好的窗口高度
		NULL,								// 无父窗口
		NULL,								// 无菜单
		hInstance,							// 实例
		NULL)))								// 不向WM_CREATE传递任何东东
	{
		KillGLWindow(hWnd, hDC, hRC);								// 重置显示区
		KillInstance();
		MessageBox(NULL, _T("窗口创建错误"), _T("错误"), MB_OK | MB_ICONEXCLAMATION);
		return false;								// 返回 false
	}

	static	PIXELFORMATDESCRIPTOR pfd =				//pfd 告诉窗口我们所希望的东东，即窗口使用的像素格式
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// 上述格式描述符的大小
		1,											// 版本号
		PFD_DRAW_TO_WINDOW |						// 格式支持窗口
		PFD_SUPPORT_OPENGL |						// 格式必须支持OpenGL
		PFD_DOUBLEBUFFER,							// 必须支持双缓冲
		PFD_TYPE_RGBA,								// 申请 RGBA 格式
		bits,										// 选定色彩深度
		0, 0, 0, 0, 0, 0,							// 忽略的色彩位
		0,											// 无Alpha缓存
		0,											// 忽略Shift Bit
		0,											// 无累加缓存
		0, 0, 0, 0,									// 忽略聚集位
		16,											// 16位 Z-缓存 (深度缓存)
		0,											// 无蒙板缓存
		0,											// 无辅助缓存
		PFD_MAIN_PLANE,								// 主绘图层
		0,											// 不使用重叠层
		0, 0, 0										// 忽略层遮罩
	};

	if (!(hDC = GetDC(hWnd)))							// 取得设备描述表了么?
	{
		KillGLWindow(hWnd, hDC, hRC);							// 重置显示区
		KillInstance();
		MessageBox(NULL, _T("不能创建一个窗口设备描述表"), _T("错误"), MB_OK | MB_ICONEXCLAMATION);
		return false;								// 返回 false
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))	// Windows 找到相应的象素格式了吗?
	{
		KillGLWindow(hWnd, hDC, hRC);							// 重置显示区
		KillInstance();
		MessageBox(NULL, _T("不能创建一种相匹配的像素格式"), _T("错误"), MB_OK | MB_ICONEXCLAMATION);
		return false;								// 返回 false
	}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))		// 能够设置象素格式么?
	{
		KillGLWindow(hWnd, hDC, hRC);							// 重置显示区
		KillInstance();
		MessageBox(NULL, _T("不能设置像素格式"), _T("错误"), MB_OK | MB_ICONEXCLAMATION);
		return false;								// 返回 false
	}

	if (!(hRC = wglCreateContext(hDC)))				// 能否取得OpenGL渲染描述表?
	{
		KillGLWindow(hWnd, hDC, hRC);							// 重置显示区
		KillInstance();
		MessageBox(NULL, _T("不能创建OpenGL渲染描述表"), _T("错误"), MB_OK | MB_ICONEXCLAMATION);
		return false;								// 返回 false
	}

	if (!wglMakeCurrent(hDC, hRC))					// 尝试激活着色描述表
	{
		KillGLWindow(hWnd, hDC, hRC);								// 重置显示区
		KillInstance();
		MessageBox(NULL, _T("不能激活当前的OpenGL渲然描述表"), _T("错误"), MB_OK | MB_ICONEXCLAMATION);
		return false;								// 返回 false
	}

	ShowWindow(hWnd, SW_SHOW);						// 显示窗口
	SetForegroundWindow(hWnd);						// 略略提高优先级
	SetFocus(hWnd);									// 设置键盘的焦点至此窗口
													//ReSizeGLScene(width, height);					// 设置透视 GL 屏幕

	glewInit();		//glew初始化必须放在NV枚举之前

	///////////////////创建一个Affinity RC并替代原有的RC///////////////////////////////////
	//HGPUNV hGPU = NULL;
	//wglEnumGpusNV(RCnum, &hGPU);

	//HGPUNV GpuMask[2] = { hGPU, NULL };
	//HDC hDCNV = NULL;
	//hDCNV = wglCreateAffinityDCNV(GpuMask);

	//int pf = ChoosePixelFormat(hDCNV, &pfd);
	//SetPixelFormat(hDCNV, pf, &pfd);
	//DescribePixelFormat(hDCNV, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	//hRC = wglCreateContext(hDCNV);

	//printf("hGPU: 0x%p hDCNV: 0x%p\n", hGPU, hDCNV);

	//wglMakeCurrent(hDC, hRC);	//记得重新激活一次

	//////////////////////////////////////////////////////////////////////////////

	return true;									// 成功
}

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path) 
{

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

string NumToString(int d)
{
	string str;
	stringstream ss;
	ss << d;
	ss >> str;
	return str;
}
string NumToString(float d)
{
	string str;
	stringstream ss;
	ss << d;
	ss >> str;
	return str;
}
string NumToString(double d)
{
	string str;
	stringstream ss;
	ss << d;
	ss >> str;
	return str;
}

void keyboard_interaction()
{
	if (keys['0'])				m_model_scale *= 1.1;
	else if (keys['9'])			m_model_scale /= 1.1;
	else if (keys['1'])			m_rotate_vec.x += 10;
	else if (keys['2'])			m_rotate_vec.x -= 10;
	else if (keys['3'])			m_rotate_vec.y += 10;
	else if (keys['4'])			m_rotate_vec.y -= 10;
	else if (keys['5'])			m_rotate_vec.z += 10;
	else if (keys['6'])			m_rotate_vec.z -= 10;
	else if (keys['P'])			d_far += 10;
	else if (keys['O'])			d_far -= 10;
	else if (keys['I'])			d_near += 10;
	else if (keys['U'])			d_near -= 10;
	else if (keys['Z'])			m_translate_vec.z += 10;
	else if (keys['C'])			m_translate_vec.z -= 10;
	else if (keys['A'])			m_translate_vec.x += 10;
	else if (keys['D'])			m_translate_vec.x -= 10;
	else if (keys['W'])			m_translate_vec.y += 10;
	else if (keys['S'])			m_translate_vec.y -= 10;
	else if (keys[' '])			snapshot_cloud = true;
	else if (keys['M'])			snapshot_cloud = false;
}

void setAdvanceMVP()
{
	glm::vec3 CameraPos = glm::vec3(0.0, 100.0, 0.0);
	glm::vec3 CameraTarget = glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 CameraUp = glm::vec3(0.0, 0.0, 1.0);

	float edge = 100;
	mProjectionMatrix = glm::ortho(-edge, edge, -edge, edge, -1000.0f, 1000.0f);
	//ProjectionMatrix = glm::perspective(30.0f, 1.0f, 0.1f, 10000.0f);
	mViewMatrix = glm::lookAt(CameraPos, CameraTarget, CameraUp);

	mScaleMatrix = glm::scale(glm::mat4(1.0), glm::vec3(m_model_scale, m_model_scale, m_model_scale));
	mTranslateMatrix = glm::translate(glm::mat4(1.0), m_translate_vec);
	mRotateMatrix_x = glm::rotate(glm::mat4(1.0), m_rotate_vec.x, glm::vec3(1.0, 0.0, 0.0));
	mRotateMatrix_y = glm::rotate(glm::mat4(1.0), m_rotate_vec.y, glm::vec3(0.0, 1.0, 0.0));
	mRotateMatrix_z = glm::rotate(glm::mat4(1.0), m_rotate_vec.z, glm::vec3(0.0, 0.0, 1.0));
	mModelMatrix = glm::mat4(1.0) * mScaleMatrix * mRotateMatrix_x * mRotateMatrix_y * mRotateMatrix_z * mTranslateMatrix;
}
