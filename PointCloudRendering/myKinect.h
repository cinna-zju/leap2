#include "stdafx.h"

#pragma once
//#include <Kinect.h>
#include <opencv2\opencv.hpp>

// Safe release for interfaces
template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

class CBodyBasics
{
	//kinect 2.0 ����ȿռ�ĸ�*���� 424 * 512���ڹ�������˵��
	static const int        cDepthWidth = 512;
	static const int        cDepthHeight = 424;
	static const int        cColorWidth = 1920;
	static const int        cColorHeight = 1080;

public:
	CBodyBasics();
	~CBodyBasics();
	void                    Update();//��ùǼܡ�������ֵͼ�������Ϣ
	HRESULT                 InitializeDefaultSensor();//���ڳ�ʼ��kinect

	cv::Point3f UserHeadPos[6];
	int TrackedUserCount;
	int UserID[6];

	cv::Mat depthImg_Map_short;
	cv::Mat depthImg_Map_Raw_short;
	cv::Mat ColorImg;
	cv::Mat ColorImgRaw;
	cv::Mat depthImg_Map;


private:
	IKinectSensor*          m_pKinectSensor;//kinectԴ
	ICoordinateMapper*      m_pCoordinateMapper;//��������任
	IColorFrameReader*       m_pColorFrameReader;//���ڹǼ����ݶ�ȡ
	IBodyFrameReader*       m_pBodyFrameReader;//���ڹǼ����ݶ�ȡ
	IDepthFrameReader*      m_pDepthFrameReader;//����������ݶ�ȡ
	IBodyIndexFrameReader*  m_pBodyIndexFrameReader;//���ڱ�����ֵͼ��ȡ

	DepthSpacePoint *		cDepthSpacePoint = new DepthSpacePoint[cColorHeight*cColorWidth];

	UINT16 *				depthArray = new UINT16[cDepthHeight * cDepthWidth];//���������16λunsigned int

	IBodyFrame* pBodyFrame = NULL;//�Ǽ���Ϣ
	IColorFrame* pColorFrame = NULL;//��ɫ��Ϣ
	IDepthFrame* pDepthFrame = NULL;//�����Ϣ
	IBodyIndexFrame* pBodyIndexFrame = NULL;//������ֵͼ

	//ͨ����õ�����Ϣ���ѹǼܺͱ�����ֵͼ������
	void                    ProcessBody(int nBodyCount, IBody** ppBodies);
	//���Ǽܺ���
	void DrawBone(const Joint* pJoints, const DepthSpacePoint* depthSpacePosition, JointType joint0, JointType joint1);
	//���ֵ�״̬����
	void DrawHandState(const DepthSpacePoint depthSpacePosition, HandState handState);

	int ID_temp;

	//��ʾͼ���Mat
	cv::Mat skeletonImg;
	cv::Mat depthImg;
	cv::Mat depthImg_Map_Raw;
};

