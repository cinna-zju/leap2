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
	//kinect 2.0 的深度空间的高*宽是 424 * 512，在官网上有说明
	static const int        cDepthWidth = 512;
	static const int        cDepthHeight = 424;
	static const int        cColorWidth = 1920;
	static const int        cColorHeight = 1080;

public:
	CBodyBasics();
	~CBodyBasics();
	void                    Update();//获得骨架、背景二值图和深度信息
	HRESULT                 InitializeDefaultSensor();//用于初始化kinect

	cv::Point3f UserHeadPos[6];
	int TrackedUserCount;
	int UserID[6];

	cv::Mat depthImg_Map_short;
	cv::Mat depthImg_Map_Raw_short;
	cv::Mat ColorImg;
	cv::Mat ColorImgRaw;
	cv::Mat depthImg_Map;


private:
	IKinectSensor*          m_pKinectSensor;//kinect源
	ICoordinateMapper*      m_pCoordinateMapper;//用于坐标变换
	IColorFrameReader*       m_pColorFrameReader;//用于骨架数据读取
	IBodyFrameReader*       m_pBodyFrameReader;//用于骨架数据读取
	IDepthFrameReader*      m_pDepthFrameReader;//用于深度数据读取
	IBodyIndexFrameReader*  m_pBodyIndexFrameReader;//用于背景二值图读取

	DepthSpacePoint *		cDepthSpacePoint = new DepthSpacePoint[cColorHeight*cColorWidth];

	UINT16 *				depthArray = new UINT16[cDepthHeight * cDepthWidth];//深度数据是16位unsigned int

	IBodyFrame* pBodyFrame = NULL;//骨架信息
	IColorFrame* pColorFrame = NULL;//彩色信息
	IDepthFrame* pDepthFrame = NULL;//深度信息
	IBodyIndexFrame* pBodyIndexFrame = NULL;//背景二值图

	//通过获得到的信息，把骨架和背景二值图画出来
	void                    ProcessBody(int nBodyCount, IBody** ppBodies);
	//画骨架函数
	void DrawBone(const Joint* pJoints, const DepthSpacePoint* depthSpacePosition, JointType joint0, JointType joint1);
	//画手的状态函数
	void DrawHandState(const DepthSpacePoint depthSpacePosition, HandState handState);

	int ID_temp;

	//显示图像的Mat
	cv::Mat skeletonImg;
	cv::Mat depthImg;
	cv::Mat depthImg_Map_Raw;
};

