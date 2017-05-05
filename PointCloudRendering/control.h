#pragma once
#include "PointCloud_process.h"
#include "RenderDMD.h"
#include "LeapMotion.h"


void point_cloud_capture()
{
	OpenNI::initialize();

	DWORD ThreadIDCap;
	if ( which_device == 0)
		HANDLE hThreadCap = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CaptureFunc_Kinect1, NULL, 0, &ThreadIDCap);
	else if (which_device == 1)
		HANDLE hThreadCap = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CaptureFunc_Kinect2, NULL, 0, &ThreadIDCap);

}

void toMonitor()
{

	if (!myRegisterClass())
	{
		MessageBox(NULL, _T("×¢²á´°¿ÚÊ§°Ü"), _T("×¢²á´°¿ÚÊ§°Ü"), MB_OK | MB_ICONEXCLAMATION);
		//return 0;
	}

	DWORD ThreadIDMonitor;
	HANDLE hThreadMonitor = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GLMonitor, NULL, 0, &ThreadIDMonitor);

	if (use_leapmotion)
	{
		DWORD ThreadIDLP;
		HANDLE hThreadLP = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunLP, NULL, 0, &ThreadIDLP);
	}
}

void toDMD()
{
	if (!myRegisterClass())
	{
		MessageBox(NULL, _T("×¢²á´°¿ÚÊ§°Ü"), _T("×¢²á´°¿ÚÊ§°Ü"), MB_OK | MB_ICONEXCLAMATION);
		//return 0;
	}

	DWORD ThreadIDDMD;
	HANDLE hThreadDMD = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GLDMD, NULL, 0, &ThreadIDDMD);
}
