#include "stdafx.h"

#include "myKinect.h"
#include <iostream>
using namespace std;

/// Initializes the default Kinect sensor
HRESULT CBodyBasics::InitializeDefaultSensor()
{
	//�����ж�ÿ�ζ�ȡ�����ĳɹ����
	HRESULT hr;

	//����kinect
	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	if (FAILED(hr)){
		return hr;
	}

	//�ҵ�kinect�豸
	if (m_pKinectSensor)
	{
		// Initialize the Kinect and get coordinate mapper and the body reader
		IColorFrameSource* pColorFrameSource = NULL;//��ȡ��ɫ
		IBodyFrameSource* pBodyFrameSource = NULL;//��ȡ�Ǽ�
		IDepthFrameSource* pDepthFrameSource = NULL;//��ȡ�����Ϣ
		IBodyIndexFrameSource* pBodyIndexFrameSource = NULL;//��ȡ������ֵͼ

		//��kinect
		hr = m_pKinectSensor->Open();

		//coordinatemapper
		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
		}

		//colorframe
		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);
		}

		//bodyframe
		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
		}

		//depth frame
		if (SUCCEEDED(hr)){
			hr = m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);
		}

		if (SUCCEEDED(hr)){
			hr = pDepthFrameSource->OpenReader(&m_pDepthFrameReader);
		}

		//body index frame
		if (SUCCEEDED(hr)){
			hr = m_pKinectSensor->get_BodyIndexFrameSource(&pBodyIndexFrameSource);
		}

		if (SUCCEEDED(hr)){
			hr = pBodyIndexFrameSource->OpenReader(&m_pBodyIndexFrameReader);
		}

		SafeRelease(pBodyFrameSource);
		SafeRelease(pDepthFrameSource);
		SafeRelease(pBodyIndexFrameSource);
	}

	if (!m_pKinectSensor || FAILED(hr))
	{
		std::cout << "Kinect initialization failed!" << std::endl;
		return E_FAIL;
	}

	//ColorImg,���ڻ���ɫ��Ϣ��MAT
	ColorImgRaw.create(cColorHeight, cColorWidth, CV_8UC4);
	ColorImgRaw.setTo(0);
	ColorImg.create(cColorHeight/4, cColorWidth/4, CV_8UC4);
	ColorImg.setTo(0);
	depthImg_Map.create(ColorImg.rows, ColorImg.cols, CV_8UC1);		
	depthImg_Map.setTo(0);
	depthImg_Map_Raw.create(cColorHeight, cColorWidth, CV_8UC1);
	depthImg_Map_Raw.setTo(0);
	depthImg_Map_short.create(ColorImg.rows, ColorImg.cols, CV_16UC1);
	depthImg_Map_short.setTo(0);
	depthImg_Map_Raw_short.create(cColorHeight, cColorWidth, CV_16UC1);
	depthImg_Map_Raw_short.setTo(0);
	
	//skeletonImg,���ڻ��Ǽܡ�������ֵͼ��MAT
	skeletonImg.create(cDepthHeight, cDepthWidth, CV_8UC3);
	skeletonImg.setTo(0);

	//depthImg,���ڻ������Ϣ��MAT
	depthImg.create(cDepthHeight, cDepthWidth, CV_8UC1);
	depthImg.setTo(0);

	return hr;
}

/// Main processing function
void CBodyBasics::Update()
{
	//ÿ�������skeletonImg
	skeletonImg.setTo(0);

	//�����ʧ��kinect���򲻼�������
	if (!m_pBodyFrameReader)
	{
		return;
	}

	//��¼ÿ�β����ĳɹ����
	HRESULT hr = S_OK;

	//---------------------------------------��ȡ������ֵͼ����ʾ---------------------------------
	//if (SUCCEEDED(hr)){
	//	hr = m_pBodyIndexFrameReader->AcquireLatestFrame(&pBodyIndexFrame);//��ñ�����ֵͼ��Ϣ
	//}
	//if (SUCCEEDED(hr)){
	//	BYTE *bodyIndexArray = new BYTE[cDepthHeight * cDepthWidth];//������ֵͼ��8Ϊuchar�������Ǻ�ɫ��û���ǰ�ɫ
	//	pBodyIndexFrame->CopyFrameDataToArray(cDepthHeight * cDepthWidth, bodyIndexArray);

	//	//�ѱ�����ֵͼ����MAT��
	//	uchar* skeletonData = (uchar*)skeletonImg.data;
	//	for (int j = 0; j < cDepthHeight * cDepthWidth; ++j){
	//		*skeletonData = bodyIndexArray[j]; ++skeletonData;
	//		*skeletonData = bodyIndexArray[j]; ++skeletonData;
	//		*skeletonData = bodyIndexArray[j]; ++skeletonData;
	//	}
	//	delete[] bodyIndexArray;
	//}
	//SafeRelease(pBodyIndexFrame);//����Ҫ�ͷţ�����֮���޷�����µ�frame����


	//-----------------------��ȡ��ɫ���ݲ���ʾ--------------------------
	if (SUCCEEDED(hr)){
		hr = m_pColorFrameReader->AcquireLatestFrame(&pColorFrame);//��ò�ɫ����
	}
	if (SUCCEEDED(hr)){
		pColorFrame->CopyConvertedFrameDataToArray(cColorHeight * cColorWidth * 4, ColorImgRaw.data, ColorImageFormat_Bgra);

	}
	SafeRelease(pColorFrame);//����Ҫ�ͷţ�����֮���޷�����µ�frame����
	cv::resize(ColorImgRaw, ColorImg, cvSize(ColorImg.cols, ColorImg.rows));
	//imshow("ColorImg", ColorImg);
	//cv::waitKey(5);

	//-----------------------��ȡ������ݲ���ʾ--------------------------
	if (SUCCEEDED(hr)){
		hr = m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);//����������
	}
	if (SUCCEEDED(hr)){
		//UINT16 *depthArray = new UINT16[cDepthHeight * cDepthWidth];//���������16λunsigned int
		pDepthFrame->CopyFrameDataToArray(cDepthHeight * cDepthWidth, depthArray);
		UINT nDepthBufferSize = cDepthHeight * cDepthWidth;
		//pDepthFrame->AccessUnderlyingBuffer(&nDepthBufferSize, &depthArray);

		HRESULT hr = m_pCoordinateMapper->MapColorFrameToDepthSpace(cDepthHeight * cDepthWidth, (UINT16*)depthArray, cColorHeight * cColorWidth , cDepthSpacePoint);
		
		depthImg_Map_Raw.setTo(0);
		depthImg_Map_Raw_short.setTo(0);

		//for (int k = 0; k < cColorHeight * cColorWidth; k++)
		for (int i = 0; i < cColorHeight; i++)
		for (int j = 0; j < cColorWidth; j++)
		{
			int k = i*cColorWidth + j;
			int x = static_cast<int>(cDepthSpacePoint[k].X + 0.5);
			int y = static_cast<int>(cDepthSpacePoint[k].Y + 0.5);
			if (x >= 0 && x < cColorWidth && y >= 0 && y < cColorHeight)
			{
				depthImg_Map_Raw.data[k] = depthArray[y*cDepthWidth + x] /10;
				depthImg_Map_Raw_short.at<unsigned short int>(i, j) = depthArray[y*cDepthWidth + x];
			}
		}
		//cv::Mat depthImg_Map(cColorHeight / 4, cColorWidth / 4, CV_8UC1);		depthImg_Map.setTo(0);
		cv::resize(depthImg_Map_Raw, depthImg_Map, cvSize(depthImg_Map.cols, depthImg_Map.rows));
		cv::resize(depthImg_Map_Raw_short, depthImg_Map_short, cvSize(depthImg_Map_short.cols, depthImg_Map_short.rows));
		//imshow("depthImg_Map", depthImg_Map);
		//cv::waitKey(5);
		
	}

	SafeRelease(pDepthFrame);//����Ҫ�ͷţ�����֮���޷�����µ�frame����



	//-----------------------------��ȡ�Ǽܲ���ʾ----------------------------
	if (SUCCEEDED(hr)){
		hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);//��ȡ�Ǽ���Ϣ
	}
	if (SUCCEEDED(hr))
	{
		IBody* ppBodies[BODY_COUNT] = { 0 };//ÿһ��IBody����׷��һ���ˣ��ܹ�����׷��������

		if (SUCCEEDED(hr))
		{
			//��kinect׷�ٵ����˵���Ϣ���ֱ�浽ÿһ��IBody��
			hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
		}

		if (SUCCEEDED(hr))
		{
			//��ÿһ��IBody�������ҵ����ĹǼ���Ϣ�����һ�����
			//ProcessBody(BODY_COUNT, ppBodies);
		}

		for (int i = 0; i < _countof(ppBodies); ++i)
		{
			SafeRelease(ppBodies[i]);//�ͷ�����
		}
	}
	SafeRelease(pBodyFrame);//����Ҫ�ͷţ�����֮���޷�����µ�frame����
	
}


/// Handle new body data
void CBodyBasics::ProcessBody(int nBodyCount, IBody** ppBodies)
{
	//��¼��������Ƿ�ɹ�
	HRESULT hr;

	TrackedUserCount = 0;
	ID_temp = 0;
	UserID[6] = { 0 };

	//����ÿһ��IBody
	for (int i = 0; i < nBodyCount; ++i)
	{
		IBody* pBody = ppBodies[i];
		if (pBody)//��û�и���������pBody�������bTracked��ʲô����
		{
			BOOLEAN bTracked = false;
			hr = pBody->get_IsTracked(&bTracked);

			if (SUCCEEDED(hr) && bTracked)
			{
				Joint joints[JointType_Count];//�洢�ؽڵ���
				HandState leftHandState = HandState_Unknown;//����״̬
				HandState rightHandState = HandState_Unknown;//����״̬

				//��ȡ������״̬
				pBody->get_HandLeftState(&leftHandState);
				pBody->get_HandRightState(&rightHandState);

				//�洢�������ϵ�еĹؽڵ�λ��
				DepthSpacePoint *depthSpacePosition = new DepthSpacePoint[_countof(joints)];

				//��ùؽڵ���
				hr = pBody->GetJoints(_countof(joints), joints);
				if (SUCCEEDED(hr))
				{
					UserHeadPos[i].x = joints[JointType_Head].Position.X;
					UserHeadPos[i].y = joints[JointType_Head].Position.Y;
					UserHeadPos[i].z = joints[JointType_Head].Position.Z;

					//�жϸ��ٵ����û�id
					if (/*abs(UserHeadPos[i].x) > 0.01 && abs(UserHeadPos[i].y) > 0.01 && */abs(UserHeadPos[i].z) > 0.01)
					{
						UserID[ID_temp++] = i;
					}
					/*printf(  "%f  ,  %f  ,  %f   \r" , joints[JointType_Head].Position.X , 
						joints[JointType_Head].Position.Y ,joints[JointType_Head].Position.Z );*/

					for (int j = 0; j < _countof(joints); ++j)
					{
						//���ؽڵ���������������ϵ��-1~1��ת���������ϵ��424*512��
						m_pCoordinateMapper->MapCameraPointToDepthSpace(joints[j].Position, &depthSpacePosition[j]);
					}

					//------------------------hand state left-------------------------------
					DrawHandState(depthSpacePosition[JointType_HandLeft], leftHandState);
					DrawHandState(depthSpacePosition[JointType_HandRight], rightHandState);

					//---------------------------body-------------------------------
					DrawBone(joints, depthSpacePosition, JointType_Head, JointType_Neck);
					DrawBone(joints, depthSpacePosition, JointType_Neck, JointType_SpineShoulder);
					DrawBone(joints, depthSpacePosition, JointType_SpineShoulder, JointType_SpineMid);
					DrawBone(joints, depthSpacePosition, JointType_SpineMid, JointType_SpineBase);
					DrawBone(joints, depthSpacePosition, JointType_SpineShoulder, JointType_ShoulderRight);
					DrawBone(joints, depthSpacePosition, JointType_SpineShoulder, JointType_ShoulderLeft);
					DrawBone(joints, depthSpacePosition, JointType_SpineBase, JointType_HipRight);
					DrawBone(joints, depthSpacePosition, JointType_SpineBase, JointType_HipLeft);

					// -----------------------Right Arm ------------------------------------ 
					DrawBone(joints, depthSpacePosition, JointType_ShoulderRight, JointType_ElbowRight);
					DrawBone(joints, depthSpacePosition, JointType_ElbowRight, JointType_WristRight);
					DrawBone(joints, depthSpacePosition, JointType_WristRight, JointType_HandRight);
					DrawBone(joints, depthSpacePosition, JointType_HandRight, JointType_HandTipRight);
					DrawBone(joints, depthSpacePosition, JointType_WristRight, JointType_ThumbRight);

					//----------------------------------- Left Arm--------------------------
					DrawBone(joints, depthSpacePosition, JointType_ShoulderLeft, JointType_ElbowLeft);
					DrawBone(joints, depthSpacePosition, JointType_ElbowLeft, JointType_WristLeft);
					DrawBone(joints, depthSpacePosition, JointType_WristLeft, JointType_HandLeft);
					DrawBone(joints, depthSpacePosition, JointType_HandLeft, JointType_HandTipLeft);
					DrawBone(joints, depthSpacePosition, JointType_WristLeft, JointType_ThumbLeft);

					// ----------------------------------Right Leg--------------------------------
					DrawBone(joints, depthSpacePosition, JointType_HipRight, JointType_KneeRight);
					DrawBone(joints, depthSpacePosition, JointType_KneeRight, JointType_AnkleRight);
					DrawBone(joints, depthSpacePosition, JointType_AnkleRight, JointType_FootRight);

					// -----------------------------------Left Leg---------------------------------
					DrawBone(joints, depthSpacePosition, JointType_HipLeft, JointType_KneeLeft);
					DrawBone(joints, depthSpacePosition, JointType_KneeLeft, JointType_AnkleLeft);
					DrawBone(joints, depthSpacePosition, JointType_AnkleLeft, JointType_FootLeft);
				}
				delete[] depthSpacePosition;
			}
		}
	}

	TrackedUserCount = ID_temp;
	cv::imshow("skeletonImg", skeletonImg);
	cv::waitKey(5);
}

//���ֵ�״̬
void CBodyBasics::DrawHandState(const DepthSpacePoint depthSpacePosition, HandState handState)
{
	//����ͬ�����Ʒ��䲻ͬ��ɫ
	CvScalar color;
	switch (handState){
	case HandState_Open:
		color = cvScalar(255, 0, 0);
		break;
	case HandState_Closed:
		color = cvScalar(0, 255, 0);
		break;
	case HandState_Lasso:
		color = cvScalar(0, 0, 255);
		break;
	default://���û��ȷ�������ƣ��Ͳ�Ҫ��
		return;
	}

	circle(skeletonImg,
		cvPoint(depthSpacePosition.X, depthSpacePosition.Y),
		20, color, -1);
}


/// Draws one bone of a body (joint to joint)
void CBodyBasics::DrawBone(const Joint* pJoints, const DepthSpacePoint* depthSpacePosition, JointType joint0, JointType joint1)
{
	TrackingState joint0State = pJoints[joint0].TrackingState;
	TrackingState joint1State = pJoints[joint1].TrackingState;

	// If we can't find either of these joints, exit
	if ((joint0State == TrackingState_NotTracked) || (joint1State == TrackingState_NotTracked))
	{
		return;
	}

	// Don't draw if both points are inferred
	if ((joint0State == TrackingState_Inferred) && (joint1State == TrackingState_Inferred))
	{
		return;
	}

	CvPoint p1 = cvPoint(depthSpacePosition[joint0].X, depthSpacePosition[joint0].Y),
		p2 = cvPoint(depthSpacePosition[joint1].X, depthSpacePosition[joint1].Y);

	// We assume all drawn bones are inferred unless BOTH joints are tracked
	if ((joint0State == TrackingState_Tracked) && (joint1State == TrackingState_Tracked))
	{
		//�ǳ�ȷ���ĹǼܣ��ð�ɫֱ��
	    line(skeletonImg, p1, p2, cvScalar(255, 255, 255));
	}
	else
	{
		//��ȷ���ĹǼܣ��ú�ɫֱ��
		line(skeletonImg, p1, p2, cvScalar(0, 0, 255));
	}
}


/// Constructor
CBodyBasics::CBodyBasics() :
m_pKinectSensor(NULL),
m_pCoordinateMapper(NULL),
m_pBodyFrameReader(NULL){}

/// Destructor
CBodyBasics::~CBodyBasics()
{
	delete[] depthArray;
	delete[] cDepthSpacePoint;

	SafeRelease(m_pBodyFrameReader);
	SafeRelease(m_pCoordinateMapper);

	if (m_pKinectSensor)
	{
		m_pKinectSensor->Close();
	}
	SafeRelease(m_pKinectSensor);
}
