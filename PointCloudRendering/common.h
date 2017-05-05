#include "stdafx.h"
#pragma once

#include <math.h>			// ��ѧ������
#include <stdlib.h>			// ��׼��
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

#include	"gl/glew.h"			// �������µ�gl.h,glu.h��
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

HINSTANCE	hInstances[2];		// ��������ʵ��
HINSTANCE	hInstance;		// ��������ʵ��

bool	keys[256];			// ������̰���������
bool	active = true;		// ���ڵĻ��־��ȱʡΪtrue

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
		//MessageBox(NULL,L"����̨��ʧ��",L"����̨��ʧ��",MB_OK|MB_ICONEXCLAMATION);
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

		printf("����̨�򿪳ɹ���\n");
	}
}


LRESULT CALLBACK WndProc1(HWND	hWnd,			// ���ڵľ��
	UINT	uMsg,			// ���ڵ���Ϣ
	WPARAM	wParam,			// ���ӵ���Ϣ����
	LPARAM	lParam)			// ���ӵ���Ϣ����
{
	switch (uMsg)									// ���Windows��Ϣ
	{
	case WM_ACTIVATE:							// ���Ӵ��ڼ�����Ϣ
	{
		if (!HIWORD(wParam))					// �����С��״̬
		{
			active = true;						// �����ڼ���״̬
		}
		else
		{
			active = false;						// �����ټ���
		}

		return 0;								// ������Ϣѭ��
	}

	case WM_SYSCOMMAND:							// ϵͳ�ж�����
	{
		switch (wParam)							// ���ϵͳ����
		{
		case SC_SCREENSAVE:					// ����Ҫ����?
		case SC_MONITORPOWER:				// ��ʾ��Ҫ����ڵ�ģʽ?
			return 0;							// ��ֹ����
		}
		break;									// �˳�
	}

	case WM_CLOSE:								// �յ�Close��Ϣ?
	{
		PostQuitMessage(0);						// �����˳���Ϣ
		return 0;								// ����
	}

	case WM_KEYDOWN:							// �м�����ô?
	{
		keys[wParam] = true;					// ����ǣ���Ϊtrue
		return 0;								// ����
	}

	case WM_KEYUP:								// �м��ſ�ô?
	{
		keys[wParam] = false;					// ����ǣ���Ϊfalse
		return 0;								// ����
	}
	}

	// �� DefWindowProc��������δ�������Ϣ��
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool myRegisterClass()
{
	WNDCLASS	wc;						// ������ṹ

	hInstance = GetModuleHandle(NULL);				// ȡ�����Ǵ��ڵ�ʵ��
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// �ƶ�ʱ�ػ�����Ϊ����ȡ��DC
	wc.lpfnWndProc = (WNDPROC)WndProc1;					// WndProc������Ϣ
	wc.cbClsExtra = 0;									// �޶��ⴰ������
	wc.cbWndExtra = 0;									// �޶��ⴰ������
	wc.hInstance = hInstance;							// ����ʵ��
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);			// װ��ȱʡͼ��
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);			// װ�����ָ��
	wc.hbrBackground = NULL;									// GL����Ҫ����
	wc.lpszMenuName = NULL;									// ����Ҫ�˵�
	wc.lpszClassName = _T("OpenG");								// �趨������

	return RegisterClass(&wc);									// ����ע�ᴰ����
}

void KillGLWindow(HWND hWnd, HDC& hDC, HGLRC& hRC)								// �������ٴ���
{
	wglMakeCurrent(hDC, hRC);
	if (hRC)											//����ӵ��OpenGL��������?
	{
		if (!wglMakeCurrent(NULL, NULL))					// �����ܷ��ͷ�DC��RC������?
		{
			MessageBox(NULL, _T("�ͷ�DC��RCʧ�ܡ�"), _T("�رմ���"), MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// �����ܷ�ɾ��RC?
		{
			MessageBox(NULL, _T("�ͷ�RCʧ�ܡ�"), _T("�رմ���"), MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;										// ��RC��Ϊ NULL
	}

	if (hDC && !ReleaseDC(hWnd, hDC))					// �����ܷ��ͷ� DC?
	{
		MessageBox(NULL, _T("�ͷ�DCʧ�ܡ�"), _T("�رմ���"), MB_OK | MB_ICONINFORMATION);
		hDC = NULL;										// �� DC ��Ϊ NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// �ܷ����ٴ���?
	{
		MessageBox(NULL, _T("�ͷŴ��ھ��ʧ�ܡ�"), _T("�رմ���"), MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;										// �� hWnd ��Ϊ NULL
	}
}

void KillInstance()
{
	if (!UnregisterClass(_T("OpenG"), hInstance))			// �ܷ�ע����?
	{
		MessageBox(NULL, _T("����ע�������ࡣ"), _T("�رմ���"), MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;									// �� hInstance ��Ϊ NULL
	}
}

bool CreateGLWindow(LPCWSTR title,
	int pos_x, int pos_y, int width, int height, int bits,
	HWND& hWnd, HDC& hDC, HGLRC& hRC, int RCnum)
{
	bool caption = false;
	GLuint		PixelFormat;			// �������ƥ��Ľ��
	DWORD		dwExStyle;				// ��չ���ڷ��
	DWORD		dwStyle;				// ���ڷ��
	RECT		WindowRect;				// ȡ�þ��ε����ϽǺ����½ǵ�����ֵ
	WindowRect.left = (long)0;			// ��Left   ��Ϊ 0
	WindowRect.right = (long)width;		// ��Right  ��ΪҪ��Ŀ��
	WindowRect.top = (long)0;				// ��Top    ��Ϊ 0
	WindowRect.bottom = (long)height;		// ��Bottom ��ΪҪ��ĸ߶�

	dwExStyle = WS_EX_APPWINDOW;			// ��չ������
	dwStyle = WS_POPUP;							// ������

	if (caption)	//Ҫ�������
	{
		dwStyle = WS_CAPTION;//WS_OVERLAPPEDWINDOW;							// ������
	}
	AdjustWindowRectEx(&WindowRect, dwStyle, false, dwExStyle);		// �������ڴﵽ����Ҫ��Ĵ�С

																	// ��������
	if (!(hWnd = CreateWindowEx(dwExStyle,							// ��չ������
		_T("OpenG"),							// ������
		(title),								// ���ڱ���
		dwStyle |							// ����Ĵ���������
		WS_CLIPSIBLINGS |					// ����Ĵ���������
		WS_CLIPCHILDREN,					// ����Ĵ���������
		pos_x, pos_y,								// ����λ��
		WindowRect.right - WindowRect.left,	// ��������õĴ��ڿ��
		WindowRect.bottom - WindowRect.top,	// ��������õĴ��ڸ߶�
		NULL,								// �޸�����
		NULL,								// �޲˵�
		hInstance,							// ʵ��
		NULL)))								// ����WM_CREATE�����κζ���
	{
		KillGLWindow(hWnd, hDC, hRC);								// ������ʾ��
		KillInstance();
		MessageBox(NULL, _T("���ڴ�������"), _T("����"), MB_OK | MB_ICONEXCLAMATION);
		return false;								// ���� false
	}

	static	PIXELFORMATDESCRIPTOR pfd =				//pfd ���ߴ���������ϣ���Ķ�����������ʹ�õ����ظ�ʽ
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// ������ʽ�������Ĵ�С
		1,											// �汾��
		PFD_DRAW_TO_WINDOW |						// ��ʽ֧�ִ���
		PFD_SUPPORT_OPENGL |						// ��ʽ����֧��OpenGL
		PFD_DOUBLEBUFFER,							// ����֧��˫����
		PFD_TYPE_RGBA,								// ���� RGBA ��ʽ
		bits,										// ѡ��ɫ�����
		0, 0, 0, 0, 0, 0,							// ���Ե�ɫ��λ
		0,											// ��Alpha����
		0,											// ����Shift Bit
		0,											// ���ۼӻ���
		0, 0, 0, 0,									// ���Ծۼ�λ
		16,											// 16λ Z-���� (��Ȼ���)
		0,											// ���ɰ建��
		0,											// �޸�������
		PFD_MAIN_PLANE,								// ����ͼ��
		0,											// ��ʹ���ص���
		0, 0, 0										// ���Բ�����
	};

	if (!(hDC = GetDC(hWnd)))							// ȡ���豸��������ô?
	{
		KillGLWindow(hWnd, hDC, hRC);							// ������ʾ��
		KillInstance();
		MessageBox(NULL, _T("���ܴ���һ�������豸������"), _T("����"), MB_OK | MB_ICONEXCLAMATION);
		return false;								// ���� false
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))	// Windows �ҵ���Ӧ�����ظ�ʽ����?
	{
		KillGLWindow(hWnd, hDC, hRC);							// ������ʾ��
		KillInstance();
		MessageBox(NULL, _T("���ܴ���һ����ƥ������ظ�ʽ"), _T("����"), MB_OK | MB_ICONEXCLAMATION);
		return false;								// ���� false
	}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))		// �ܹ��������ظ�ʽô?
	{
		KillGLWindow(hWnd, hDC, hRC);							// ������ʾ��
		KillInstance();
		MessageBox(NULL, _T("�����������ظ�ʽ"), _T("����"), MB_OK | MB_ICONEXCLAMATION);
		return false;								// ���� false
	}

	if (!(hRC = wglCreateContext(hDC)))				// �ܷ�ȡ��OpenGL��Ⱦ������?
	{
		KillGLWindow(hWnd, hDC, hRC);							// ������ʾ��
		KillInstance();
		MessageBox(NULL, _T("���ܴ���OpenGL��Ⱦ������"), _T("����"), MB_OK | MB_ICONEXCLAMATION);
		return false;								// ���� false
	}

	if (!wglMakeCurrent(hDC, hRC))					// ���Լ�����ɫ������
	{
		KillGLWindow(hWnd, hDC, hRC);								// ������ʾ��
		KillInstance();
		MessageBox(NULL, _T("���ܼ��ǰ��OpenGL��Ȼ������"), _T("����"), MB_OK | MB_ICONEXCLAMATION);
		return false;								// ���� false
	}

	ShowWindow(hWnd, SW_SHOW);						// ��ʾ����
	SetForegroundWindow(hWnd);						// ����������ȼ�
	SetFocus(hWnd);									// ���ü��̵Ľ������˴���
													//ReSizeGLScene(width, height);					// ����͸�� GL ��Ļ

	glewInit();		//glew��ʼ���������NVö��֮ǰ

	///////////////////����һ��Affinity RC�����ԭ�е�RC///////////////////////////////////
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

	//wglMakeCurrent(hDC, hRC);	//�ǵ����¼���һ��

	//////////////////////////////////////////////////////////////////////////////

	return true;									// �ɹ�
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
