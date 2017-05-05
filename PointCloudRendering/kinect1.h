#pragma once

// OpenNI Header
#include <OpenNI.h>
using namespace openni;

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
using namespace cv;

class myKinect1
{
public:

	
	Mat colorImg, depthImgRaw, depthImg;

	bool init()
	{
		OpenNI::initialize();
		Array<DeviceInfo> aDeviceList;
		OpenNI::enumerateDevices(&aDeviceList);
		if (aDeviceList.getSize() <= 0)
			return false;
		const DeviceInfo& rDevInfo = aDeviceList[0];
		Device mDevice;
		mDevice.open(rDevInfo.getUri());

		// 创建颜色数据量
		VideoMode colorMode;
		colorMode.setResolution(640, 480);
		colorMode.setFps(30);
		colorMode.setPixelFormat(PIXEL_FORMAT_RGB888);
		// 创建深度数据量
		VideoMode depthMode;
		depthMode.setResolution(640, 480);
		depthMode.setFps(30);
		depthMode.setPixelFormat(PIXEL_FORMAT_DEPTH_1_MM);

		mColorStream.create(mDevice, SENSOR_COLOR);
		mColorStream.setVideoMode(colorMode);
		mDepthStream.create(mDevice, SENSOR_DEPTH);
		mDepthStream.setVideoMode(depthMode);
		mDevice.setImageRegistrationMode(IMAGE_REGISTRATION_DEPTH_TO_COLOR);
		mColorStream.start();
		mDepthStream.start();
	};

	void update()
	{
		VideoFrameRef mFrame;
		//mColorStream.readFrame(&mFrame);
		//Mat mImageRGB1(mFrame.getHeight(), mFrame.getWidth(), CV_8UC3, const_cast<void*>(mFrame.getData()));
		//cvtColor(mImageRGB1, colorImg, CV_RGB2BGR);

		/*mDepthStream.readFrame(&mFrame);
		Mat mImageDepth1(mFrame.getHeight(), mFrame.getWidth(), CV_16UC1, const_cast<void*>(mFrame.getData()));
		mImageDepth1.copyTo(depthImgRaw); 
		depthImgRaw.convertTo(depthImg, CV_8U, 255.0 / mDepthStream.getMaxPixelValue());


		imshow("color", colorImg);
		waitKey(1);*/
	};

private:
	VideoStream mColorStream, mDepthStream;
};