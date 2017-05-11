#pragma once

#include "common.h"

// OpenNI Header
#include <OpenNI.h>
//#include "myKinect.h"
using namespace openni;

#include "LeapMotion.h"

HDC		hDC0 = NULL;	HGLRC	hRC0 = NULL;	HWND	hWnd0 = NULL;

//rs::context ctx;
//rs::device * dev;
cv::Mat MatRGB(480, 640, CV_8UC3, cv::Scalar(0, 0, 0));
cv::Mat MatBGR(480, 640, CV_8UC3, cv::Scalar(0, 0, 0));
cv::Mat MatBGR_map(480, 640, CV_8UC3, cv::Scalar(0, 0, 0));
cv::Mat MatDepth_map(480, 640, CV_16UC1, cv::Scalar(0, 0, 0));

//获取Kinect数据 
void CaptureFunc_Kinect1()
{
	VideoStream mColorStream, mDepthStream;
	OpenNI::initialize();
	Array<DeviceInfo> aDeviceList;
	OpenNI::enumerateDevices(&aDeviceList);
	const DeviceInfo& rDevInfo = aDeviceList[0];
	Device mDevice;
	mDevice.open(rDevInfo.getUri());

	// 创建颜色数据量
	VideoMode colorMode;	colorMode.setResolution(640, 480);	colorMode.setFps(30);	colorMode.setPixelFormat(PIXEL_FORMAT_RGB888);
	// 创建深度数据量
	VideoMode depthMode;	depthMode.setResolution(640, 480);	depthMode.setFps(30);	depthMode.setPixelFormat(PIXEL_FORMAT_DEPTH_1_MM);

	mColorStream.create(mDevice, SENSOR_COLOR);	mColorStream.setVideoMode(colorMode);
	mDepthStream.create(mDevice, SENSOR_DEPTH);	mDepthStream.setVideoMode(depthMode);
	mDevice.setImageRegistrationMode(IMAGE_REGISTRATION_DEPTH_TO_COLOR);

	mColorStream.start();
	mDepthStream.start();

	while (1)
	{
		VideoFrameRef mFrame;
		mColorStream.readFrame(&mFrame);		//读彩色数据 并转化成Mat
		Mat mImageRGB1(mFrame.getHeight(), mFrame.getWidth(), CV_8UC3, const_cast<void*>(mFrame.getData()));
		cvtColor(mImageRGB1, colorImg, CV_RGB2BGR);

		mDepthStream.readFrame(&mFrame);		//读深度数据 short
		Mat mImageDepth1(mFrame.getHeight(), mFrame.getWidth(), CV_16UC1, const_cast<void*>(mFrame.getData()));
		mImageDepth1.copyTo(depthImgRaw);
		depthImgRaw.convertTo(depthImg, CV_8U, 255.0 / mDepthStream.getMaxPixelValue());

		imshow("color", colorImg);
		if (waitKey(1) == 27)
		{
			break;
		}
	}
	OpenNI::shutdown();
}

void CaptureFunc_Kinect2()
{
	/*
	CBodyBasics myKinect;
	HRESULT hr = myKinect.InitializeDefaultSensor();
	
	Mat colorImgROI(COLOR_HIGHT, COLOR_WIDTH, CV_8UC4, Scalar(0));

	if (SUCCEEDED(hr)) 
	{
		while (1)
		{
			myKinect.Update();
			Rect roi_rect((1920-1440)/2, 0, 1440, 1080);
			resize(myKinect.ColorImgRaw(roi_rect), colorImgROI, cvSize(COLOR_WIDTH, COLOR_HIGHT));
			cvtColor(colorImgROI, colorImg, CV_BGRA2BGR);
			resize(myKinect.depthImg_Map_Raw_short(roi_rect), depthImgRaw, cvSize(COLOR_WIDTH, COLOR_HIGHT));

			imshow("color", colorImg);
			if (waitKey(1) == 27)
			{
				break;
			}

		}
	}
	*/
  
}


glm::vec3 m_rotate_vec_tmp = m_rotate_vec;
void setBasicMVP()
{
	glm::vec3 CameraPos = glm::vec3(m_viewer_distance * cos(1.0*m_viewer_angle_V*3.14 / 180) * sin(1.0*m_viewer_angle_H*3.14 / 180),
		m_viewer_distance * sin(1.0*m_viewer_angle_V*3.14 / 180),
		m_viewer_distance * cos(1.0*m_viewer_angle_V*3.14 / 180) * cos(1.0*m_viewer_angle_H*3.14 / 180));
	glm::vec3 CameraTarget = glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 CameraUp = glm::vec3(0.0, 1.0, 0.0);
	float FoV = 45;//initialFoV;
	mProjectionMatrix = glm::perspective(FoV, 1.0f, 0.1f, 10000.0f);
	mViewMatrix = glm::lookAt(CameraPos, CameraTarget, CameraUp);

	/*if (use_leapmotion)
	{
		if (rot_byLP)
		{
			m_rotate_vec_tmp.y = m_rotate_vec.y + (finger_rot.x - finger_rot_base.x);
			m_rotate_vec_tmp.x = m_rotate_vec.z + (finger_rot.z - finger_rot_base.z);
		}
		else
			m_rotate_vec = m_rotate_vec_tmp;
			;
	}
	else
		m_rotate_vec_tmp = m_rotate_vec;*/
	mScaleMatrix = glm::scale(glm::mat4(1.0), glm::vec3(m_model_scale, m_model_scale, m_model_scale));
	mTranslateMatrix = glm::translate(glm::mat4(1.0), m_translate_vec);
	mRotateMatrix_x = glm::rotate(glm::mat4(1.0), m_rotate_vec.x, glm::vec3(1.0, 0.0, 0.0));
	mRotateMatrix_y = glm::rotate(glm::mat4(1.0), m_rotate_vec.y, glm::vec3(0.0, 1.0, 0.0));
	mRotateMatrix_z = glm::rotate(glm::mat4(1.0), m_rotate_vec.z, glm::vec3(0.0, 0.0, 1.0));
	mModelMatrix = glm::mat4(1.0) * mScaleMatrix * mRotateMatrix_x * mRotateMatrix_y * mRotateMatrix_z * mTranslateMatrix;
}

void PointCloudProcess()
{
	int width = COLOR_WIDTH;	// depthImg.cols;
	int height = COLOR_HIGHT;	// depthImg.rows;
	int step = 2;

	float delta = 50;
	depthMatIndex.setTo(0);
	PointCloudVertex.clear();
	//PointCloudColor.clear();
	PointCloudUV.clear();
	PointCloudIndices.clear();
	int cnt = 0;

	for (int i = step; i < height - step; i += step)
		for (int j = step; j < width - step; j += step)
		{
			if (
				depthImgRaw.at<unsigned short int>(i, j) > d_near && depthImgRaw.at<unsigned short int>(i, j) < d_far
				&& abs(depthImgRaw.at<unsigned short int>(i, j) - depthImgRaw.at<unsigned short int>(i - step, j - step)) < delta
				&& abs(depthImgRaw.at<unsigned short int>(i, j) - depthImgRaw.at<unsigned short int>(i, j - step)) < delta
				&& abs(depthImgRaw.at<unsigned short int>(i, j) - depthImgRaw.at<unsigned short int>(i - step, j)) < delta
				)
			{
				depthMatIndex.at<unsigned long int>(i, j) = cnt++;

				glm::vec3 pos00(1.0*(j - width / 2)*0.0017*depthImgRaw.at<unsigned short int>(i, j), 1.0*(height / 2 - i)*0.0017*depthImgRaw.at<unsigned short int>(i, j), depthImgRaw.at<unsigned short int>(i, j) - 700);
				//glm::vec3 color00(1.0 * colorImg.data[i*width * 3 + j * 3 + 2] / 255, 1.0 * colorImg.data[i*width * 3 + j * 3 + 1] / 255, 1.0 * colorImg.data[i*width * 3 + j * 3 + 0] / 255);
				//glm::vec3 pos00(1.0*(j - width / 2), 1.0*(height / 2 - i), 0/* depthImgRaw.at<unsigned short int>(i, j)*/);
				//glm::vec3 color00(1.0, 1.0, 1.0);
				if (pos00.z > upv)	upv = pos00.z;
				if (pos00.z < downv)	downv = pos00.z;
				PointCloudVertex.push_back(pos00);
				//PointCloudColor.push_back(color00);

				glm::vec2 uv00(1 - 1.0*j / width, 1.0*i / height);   // the (0, 0) locates at the lower-left point
				PointCloudUV.push_back(uv00);

				if (
					depthMatIndex.at<unsigned long int>(i, j) > 0
					&& depthMatIndex.at<unsigned long int>(i, j - step) > 0
					&& depthMatIndex.at<unsigned long int>(i - step, j - step) > 0
					&& depthMatIndex.at<unsigned long int>(i - step, j) > 0
					)
				{
					//QUADS
					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i, j));
					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i, j - step));
					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i - step, j - step));
					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i - step, j));
					//TRIANGLES
					/*PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i, j));
					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i, j + step));
					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i + step, j + step));
					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i + step, j + step));
					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i + step, j));
					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i, j));*/
				}
			}
		}
	

}

void PointCloudProcess3(int ThreadFlag)
{
	int width = COLOR_WIDTH;	// depthImg.cols;
	int height = COLOR_HIGHT;	// depthImg.rows;
	int step = 2;

	float delta = 50;
	depthMatIndex.setTo(0);
	PointCloudVertex3[ThreadFlag].clear();
	//PointCloudColor.clear();
	PointCloudUV3[ThreadFlag].clear();
	PointCloudIndices3[ThreadFlag].clear();
	int cnt = 0;

	for (int i = step; i < height - step; i += step)
		for (int j = step; j < width - step; j += step)
		{
			if (
				depthImgRaw.at<unsigned short int>(i, j) > d_near && depthImgRaw.at<unsigned short int>(i, j) < d_far
				&& abs(depthImgRaw.at<unsigned short int>(i, j) - depthImgRaw.at<unsigned short int>(i - step, j - step)) < delta
				&& abs(depthImgRaw.at<unsigned short int>(i, j) - depthImgRaw.at<unsigned short int>(i, j - step)) < delta
				&& abs(depthImgRaw.at<unsigned short int>(i, j) - depthImgRaw.at<unsigned short int>(i - step, j)) < delta
				)
			{
				depthMatIndex.at<unsigned long int>(i, j) = cnt++;

				glm::vec3 pos00(1.0*(j - width / 2)*0.0017*depthImgRaw.at<unsigned short int>(i, j), 1.0*(height / 2 - i)*0.0017*depthImgRaw.at<unsigned short int>(i, j), depthImgRaw.at<unsigned short int>(i, j) - 700);
				//glm::vec3 color00(1.0 * colorImg.data[i*width * 3 + j * 3 + 2] / 255, 1.0 * colorImg.data[i*width * 3 + j * 3 + 1] / 255, 1.0 * colorImg.data[i*width * 3 + j * 3 + 0] / 255);
				//glm::vec3 pos00(1.0*(j - width / 2), 1.0*(height / 2 - i), 0/* depthImgRaw.at<unsigned short int>(i, j)*/);
				//glm::vec3 color00(1.0, 1.0, 1.0);
				if (pos00.z > upv)	upv = pos00.z;
				if (pos00.z < downv)	downv = pos00.z;
				PointCloudVertex3[ThreadFlag].push_back(pos00);
				//PointCloudColor.push_back(color00);

				glm::vec2 uv00(1 - 1.0*j / width, 1.0*i / height);   // the (0, 0) locates at the lower-left point
				PointCloudUV3[ThreadFlag].push_back(uv00);

				if (
					depthMatIndex.at<unsigned long int>(i, j) > 0
					&& depthMatIndex.at<unsigned long int>(i, j - step) > 0
					&& depthMatIndex.at<unsigned long int>(i - step, j - step) > 0
					&& depthMatIndex.at<unsigned long int>(i - step, j) > 0
					)
				{
					//QUADS
					PointCloudIndices3[ThreadFlag].push_back(depthMatIndex.at<unsigned long int>(i, j));
					PointCloudIndices3[ThreadFlag].push_back(depthMatIndex.at<unsigned long int>(i, j - step));
					PointCloudIndices3[ThreadFlag].push_back(depthMatIndex.at<unsigned long int>(i - step, j - step));
					PointCloudIndices3[ThreadFlag].push_back(depthMatIndex.at<unsigned long int>(i - step, j));
					//TRIANGLES
					/*PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i, j));
					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i, j + step));
					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i + step, j + step));
					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i + step, j + step));
					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i + step, j));
					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i, j));*/
				}
			}
		}
	

}

void PointCloudProcess_RS()
{
	//dev->wait_for_frames();

	//// Retrieve our images
	///*const uint16_t * depth_image = (const uint16_t *)dev->get_frame_data(rs::stream::depth);
	//const uint8_t * color_image = (const uint8_t *)dev->get_frame_data(rs::stream::color);*/
	//unsigned short int * depth_image = (unsigned short int *)dev->get_frame_data(rs::stream::depth);
	//uchar * color_image = (uchar *)dev->get_frame_data(rs::stream::color);

	//// Retrieve camera parameters for mapping between depth and color
	//rs::intrinsics depth_intrin = dev->get_stream_intrinsics(rs::stream::depth);
	//rs::extrinsics depth_to_color = dev->get_extrinsics(rs::stream::depth, rs::stream::color);
	//rs::intrinsics color_intrin = dev->get_stream_intrinsics(rs::stream::color);
	//float scale = dev->get_depth_scale();

	//MatRGB.data = color_image;

	//cv::cvtColor(MatRGB, MatBGR, COLOR_RGB2BGR);

	////cout << depth_intrin.height << "  " << depth_intrin.width << "   " << color_intrin.height << "  " << color_intrin.width << endl;

	//MatBGR_map.setTo(0);

	//int step = 2;
	//float delta = 50;
	//depthMatIndex.setTo(0);
	//PointCloudVertex.clear();
	////PointCloudColor.clear();
	//PointCloudUV.clear();
	//PointCloudIndices.clear();
	//int cnt = 0;

	//for (int i = step; i < DEPTH_HIGHT - step; i += 1)
	//	for (int j = step; j < DEPTH_WIDTH - step; j += 1)
	//	{
	//		// Retrieve the 16-bit depth value and map it into a depth in meters
	//		uint16_t depth_value = depth_image[i * DEPTH_WIDTH + j];
	//		float depth_in_meters = depth_value * scale;

	//		// Skip over pixels with a depth value of zero, which is used to indicate no data
	//		//if (depth_value == 0) continue;

	//		// Map from pixel coordinates in the depth image to pixel coordinates in the color image
	//		rs::float2 depth_pixel = { (float)j, (float)i };
	//		rs::float3 depth_point = depth_intrin.deproject(depth_pixel, depth_in_meters);
	//		rs::float3 color_point = depth_to_color.transform(depth_point);
	//		rs::float2 color_pixel = color_intrin.project(color_point);

	//		// Use the color from the nearest color pixel, or pure white if this point falls outside the color image
	//		const int cx = (int)std::round(color_pixel.x), cy = (int)std::round(color_pixel.y);

	//		float cr = 0, cg = 0, cb = 0;
	//		if (cx >= 0 && cy >= 0 && cx < DEPTH_WIDTH && cy < DEPTH_HIGHT)
	//		{
	//			cb = MatBGR_map.data[i * DEPTH_WIDTH * 3 + j * 3 + 0] = MatBGR.data[cy * DEPTH_WIDTH * 3 + cx * 3 + 0];
	//			cg = MatBGR_map.data[i * DEPTH_WIDTH * 3 + j * 3 + 1] = MatBGR.data[cy * DEPTH_WIDTH * 3 + cx * 3 + 1];
	//			cr = MatBGR_map.data[i * DEPTH_WIDTH * 3 + j * 3 + 2] = MatBGR.data[cy * DEPTH_WIDTH * 3 + cx * 3 + 2];
	//		}

	//		if (i%step == 0 && j%step == 0)
	//		{
	//			if (cb > 0 || cg > 0 || cr > 0)
	//			{
	//				depthMatIndex.at<unsigned long int>(i, j) = cnt++;

	//				PointCloudVertex.push_back(glm::vec3(depth_point.x * 1000, -depth_point.y * 1000, depth_point.z * 1000 - 500));
	//				glm::vec2 uv00(1 - 1.0*j / DEPTH_WIDTH, 1.0*i / DEPTH_HIGHT);   // the (0, 0) locates at the lower-left point
	//				PointCloudUV.push_back(uv00);

	//				if (
	//					depthMatIndex.at<unsigned long int>(i, j) > 0
	//					&& depthMatIndex.at<unsigned long int>(i, j - step) > 0
	//					&& depthMatIndex.at<unsigned long int>(i - step, j - step) > 0
	//					&& depthMatIndex.at<unsigned long int>(i - step, j) > 0
	//					)
	//				{
	//					//QUADS
	//					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i, j));
	//					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i, j - step));
	//					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i - step, j - step));
	//					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i - step, j));
	//					//TRIANGLES
	//					/*PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i, j));
	//					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i, j + step));
	//					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i + step, j + step));
	//					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i + step, j + step));
	//					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i + step, j));
	//					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i, j));*/
	//				}
	//			}
	//		}
	//	}
	//
	//colorImg = MatBGR_map;
	//imshow("color", colorImg);
	//cv::waitKey(1);

}


void PointCloudProcess_RS3(int ThreadFlag)
{
	//dev->wait_for_frames();

	//// Retrieve our images
	///*const uint16_t * depth_image = (const uint16_t *)dev->get_frame_data(rs::stream::depth);
	//const uint8_t * color_image = (const uint8_t *)dev->get_frame_data(rs::stream::color);*/
	//unsigned short int * depth_image = (unsigned short int *)dev->get_frame_data(rs::stream::depth);
	//uchar * color_image = (uchar *)dev->get_frame_data(rs::stream::color);

	//// Retrieve camera parameters for mapping between depth and color
	//rs::intrinsics depth_intrin = dev->get_stream_intrinsics(rs::stream::depth);
	//rs::extrinsics depth_to_color = dev->get_extrinsics(rs::stream::depth, rs::stream::color);
	//rs::intrinsics color_intrin = dev->get_stream_intrinsics(rs::stream::color);
	//float scale = dev->get_depth_scale();

	//MatRGB.data = color_image;

	//cv::cvtColor(MatRGB, MatBGR, COLOR_RGB2BGR);

	////cout << depth_intrin.height << "  " << depth_intrin.width << "   " << color_intrin.height << "  " << color_intrin.width << endl;

	//MatBGR_map.setTo(0);

	//int step = 2;
	//float delta = 50;
	//depthMatIndex.setTo(0);
	//PointCloudVertex3[ThreadFlag].clear();
	////PointCloudColor.clear();
	//PointCloudUV3[ThreadFlag].clear();
	//PointCloudIndices3[ThreadFlag].clear();
	//int cnt = 0;

	//for (int i = step; i < DEPTH_HIGHT - step; i += 1)
	//	for (int j = step; j < DEPTH_WIDTH - step; j += 1)
	//	{
	//		// Retrieve the 16-bit depth value and map it into a depth in meters
	//		uint16_t depth_value = depth_image[i * DEPTH_WIDTH + j];
	//		float depth_in_meters = depth_value * scale;

	//		// Skip over pixels with a depth value of zero, which is used to indicate no data
	//		//if (depth_value == 0) continue;

	//		// Map from pixel coordinates in the depth image to pixel coordinates in the color image
	//		rs::float2 depth_pixel = { (float)j, (float)i };
	//		rs::float3 depth_point = depth_intrin.deproject(depth_pixel, depth_in_meters);
	//		rs::float3 color_point = depth_to_color.transform(depth_point);
	//		rs::float2 color_pixel = color_intrin.project(color_point);

	//		// Use the color from the nearest color pixel, or pure white if this point falls outside the color image
	//		const int cx = (int)std::round(color_pixel.x), cy = (int)std::round(color_pixel.y);

	//		float cr = 0, cg = 0, cb = 0;
	//		if (cx >= 0 && cy >= 0 && cx < DEPTH_WIDTH && cy < DEPTH_HIGHT)
	//		{
	//			cb = MatBGR_map.data[i * DEPTH_WIDTH * 3 + j * 3 + 0] = MatBGR.data[cy * DEPTH_WIDTH * 3 + cx * 3 + 0];
	//			cg = MatBGR_map.data[i * DEPTH_WIDTH * 3 + j * 3 + 1] = MatBGR.data[cy * DEPTH_WIDTH * 3 + cx * 3 + 1];
	//			cr = MatBGR_map.data[i * DEPTH_WIDTH * 3 + j * 3 + 2] = MatBGR.data[cy * DEPTH_WIDTH * 3 + cx * 3 + 2];
	//		}

	//		if (i%step == 0 && j%step == 0)
	//		{
	//			if (cb > 0 || cg > 0 || cr > 0)
	//			{
	//				depthMatIndex.at<unsigned long int>(i, j) = cnt++;

	//				PointCloudVertex3[ThreadFlag].push_back(glm::vec3(depth_point.x * 1000, -depth_point.y * 1000, depth_point.z * 1000 - 500));
	//				glm::vec2 uv00(1 - 1.0*j / DEPTH_WIDTH, 1.0*i / DEPTH_HIGHT);   // the (0, 0) locates at the lower-left point
	//				PointCloudUV3[ThreadFlag].push_back(uv00);

	//				if (
	//					depthMatIndex.at<unsigned long int>(i, j) > 0
	//					&& depthMatIndex.at<unsigned long int>(i, j - step) > 0
	//					&& depthMatIndex.at<unsigned long int>(i - step, j - step) > 0
	//					&& depthMatIndex.at<unsigned long int>(i - step, j) > 0
	//					)
	//				{
	//					//QUADS
	//					PointCloudIndices3[ThreadFlag].push_back(depthMatIndex.at<unsigned long int>(i, j));
	//					PointCloudIndices3[ThreadFlag].push_back(depthMatIndex.at<unsigned long int>(i, j - step));
	//					PointCloudIndices3[ThreadFlag].push_back(depthMatIndex.at<unsigned long int>(i - step, j - step));
	//					PointCloudIndices3[ThreadFlag].push_back(depthMatIndex.at<unsigned long int>(i - step, j));
	//					//TRIANGLES
	//					/*PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i, j));
	//					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i, j + step));
	//					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i + step, j + step));
	//					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i + step, j + step));
	//					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i + step, j));
	//					PointCloudIndices.push_back(depthMatIndex.at<unsigned long int>(i, j));*/
	//				}
	//			}
	//		}
	//	}

	//colorImg = MatBGR_map;
	//imshow("color", colorImg);
	//cv::waitKey(1);

}

void GLMonitor()
{
	//EnterCriticalSection(&cs);
	if (!CreateGLWindow(L"Monitor", 0, 0, 768, 768, 32, hWnd0, hDC0, hRC0, 1))
	{
		printf("Window%d error\n", 0);
	}

	ShaderMonitorID = LoadShaders("shader\\monitor_vert.shader", "shader\\monitor_frag.shader");
	
	MSG		msg;

	if (which_device == -1)
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
		else
		{
			if (keys[VK_ESCAPE])
				break;

			if(which_device==0 || which_device==1)
				PointCloudProcess();
			else if(which_device == 2)
				PointCloudProcess_RS();

			if (which_device != -1)
			{
				glGenBuffers(1, &vertex_buffer);
				//glGenBuffers(1, &color_buffer);
				glGenBuffers(1, &uv_buffer);
				glGenBuffers(1, &index_buffer);

				glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
				glBufferData(GL_ARRAY_BUFFER, (PointCloudVertex.size())*sizeof(glm::vec3), PointCloudVertex.data(), GL_DYNAMIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, 0);

				glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
				glBufferData(GL_ARRAY_BUFFER, (PointCloudUV.size())*sizeof(glm::vec2), PointCloudUV.data(), GL_DYNAMIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, 0);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, (PointCloudIndices.size())*sizeof(unsigned short), PointCloudIndices.data(), GL_DYNAMIC_DRAW);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

				//colorImg.copyTo(img_lp);
				flip(colorImg, texMat, 1);
				//imshow("colorImg_seg", img_lp);
				glGenTextures(1, &mTexture);
				glBindTexture(GL_TEXTURE_2D, mTexture);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texMat.cols, texMat.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, texMat.data);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glGenerateMipmap(GL_TEXTURE_2D);	glBindTexture(GL_TEXTURE_2D, 0);
			}

			glViewport(0, 0, 768, 768);

			glEnable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glClearColor(0.0, 0.0, 0.0, 1.0);

			glUseProgram(ShaderMonitorID);

			keyboard_interaction();

			setBasicMVP();
			//setAdvanceMVP();

			glUniformMatrix4fv(glGetUniformLocation(ShaderMonitorID, "ProjectionMatrix"), 1, GL_FALSE, &mProjectionMatrix[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(ShaderMonitorID, "ModelMatrix"), 1, GL_FALSE, &mModelMatrix[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(ShaderMonitorID, "ViewMatrix"), 1, GL_FALSE, &mViewMatrix[0][0]);

			glActiveTexture(GL_TEXTURE0);		glBindTexture(GL_TEXTURE_2D, mTexture);
			glUniform1i(glGetUniformLocation(ShaderMonitorID, "sampler0"), 0);

			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
			//glDrawElements(GL_POINTS, PointCloudIndices.size(), GL_UNSIGNED_SHORT, (void*)0);
			if(which_device == -1)
				glDrawElements(GL_TRIANGLES, PointCloudIndices.size(), GL_UNSIGNED_SHORT, (void*)0);
			else
				glDrawElements(GL_QUADS, PointCloudIndices.size(), GL_UNSIGNED_SHORT, (void*)0);
			//glDrawArrays(GL_POINTS, 0, PointCloudVertex.size());

			glDisableVertexAttribArray(0);		glDisableVertexAttribArray(1);	

			glFinish();

			SwapBuffers(hDC0);

			if (which_device != -1)
			{
				glDeleteBuffers(1, &vertex_buffer);
				glDeleteBuffers(1, &uv_buffer);
				glDeleteBuffers(1, &index_buffer);
			}
		}
	}
}
