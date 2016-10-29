#include "BaslerCameraCapture.h"


CBaslerCameraCapture::CBaslerCameraCapture()
{
}


CBaslerCameraCapture::~CBaslerCameraCapture()
{
	WaitForThreadExit();

	for (int i = 0; i < m_VectorCameras.size(); i++)
	{
		m_VectorCameras[i]->DestroyDevice();
		m_VectorCameras[i]->Close();
		delete m_VectorCameras[i];
	}

	m_VectorCameras.clear();
}


size_t CBaslerCameraCapture::Initialize()
{
	CTlFactory& tlFactory = CTlFactory::GetInstance();
	DeviceInfoList_t devices;

	tlFactory.EnumerateDevices(devices);

	for (int i = 0; i < devices.size(); i++)
	{
		CBaslerGigEInstantCamera* camera = new CBaslerGigEInstantCamera;

		cout << devices[i].GetSerialNumber() << endl;
		camera->Attach(tlFactory.CreateDevice(devices[i]));
		camera->StartGrabbing(GrabStrategy_LatestImageOnly);

		camera->UserSetSelector.SetValue(UserSetSelector_UserSet1);
		camera->TriggerMode.SetValue(TriggerMode_On);

		camera->StopGrabbing();

		m_VectorCameras.push_back(camera);
	}

	return m_VectorCameras.size();
}


void CBaslerCameraCapture::SequenceCapture()
{
	bThread = true;

	for (int i = 0; i < m_VectorCameras.size(); i++)
	{
		m_VectorCameras[i]->StartGrabbing();

		thread t(CaptureThreadParent, this, m_VectorCameras[i]);
		t.detach();
		m_VectorThreadHandle.push_back(t.native_handle());
	}
}


void CBaslerCameraCapture::CalibrationCapture(vector<Mat>& vectorImages)
{
	CGrabResultPtr ptrGrabResult;
	vectorImages.assign(m_VectorCameras.size(), Mat());

	for (int i = 0; i < m_VectorCameras.size(); i++)
	{
		m_VectorCameras[i]->GrabOne(1000, ptrGrabResult, TimeoutHandling_ThrowException);

		CDeviceInfo info = m_VectorCameras[i]->GetDeviceInfo();
		String_t serial = info.GetSerialNumber();

		if (ptrGrabResult->GrabSucceeded())
		{
			int index;

			switch (atoi(serial))
			{
			case 21041255:
			case 20823683:
				// cam1 or cam5
				index = 0;
				break;

			case 21041650:
			case 20829222:
				// cam2 or cam6
				index = 1;
				break;
			case 21041654:
				// cam3
				index = 2;
				break;
			case 21041254:
				// cam4
				index = 3;
				break;
			}


			Mat matImageBayer(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC1, ptrGrabResult->GetBuffer());
			cvtColor(matImageBayer, vectorImages[index], CV_BayerGB2RGB);
		}
	}
}


void CBaslerCameraCapture::CaptureThreadParent(void* arg, CBaslerGigEInstantCamera* pCamera)
{
	CBaslerCameraCapture* pClass = (CBaslerCameraCapture*)arg;
	pClass->CaptureThread(pCamera);
}


void CBaslerCameraCapture::CaptureThread(CBaslerGigEInstantCamera* pCamera)
{
	CGrabResultPtr ptrGrabResult;
	CDeviceInfo info = pCamera->GetDeviceInfo();
	String_t serial = info.GetSerialNumber();

	string strImageFolderPath;
	string strImageFilePath;
	DWORD dwFrameNumber = 0;

	while (bThread)
	{
		try
		{
			if (!pCamera->RetrieveResult(1000, ptrGrabResult, TimeoutHandling_ThrowException))
				continue;

			switch (atoi(serial))
			{
			case 20823683:
			case 21041255:
				// cam5 or cam 1
				strImageFolderPath = "D:\\image\\";
				break;
			case 20829222:
			case 21041650:
				// cam6 or cam2
				strImageFolderPath = "E:\\image\\";
				break;
			case 21041654:
				// cam3
				strImageFolderPath = "F:\\image\\";
				break;
			case 21041254:
				// cam4
				strImageFolderPath = "G:\\image\\";
				break;
			}

			dwFrameNumber = ptrGrabResult->GetBlockID() - 1;
			Mat matImageBayer(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC1, ptrGrabResult->GetBuffer());
			strImageFilePath = strImageFolderPath + serial.c_str() + "_" + to_string(dwFrameNumber) + ".png";
			imwrite(strImageFilePath, matImageBayer);
		}
		catch (TimeoutException& e)
		{
			cout << e.what();
		}
	}

	cout << dwFrameNumber << endl;
}


void CBaslerCameraCapture::WaitForThreadExit()
{
	bThread = false;

	for (int i = 0; i < m_VectorCameras.size(); i++)
		m_VectorCameras[i]->StopGrabbing();

	WaitForMultipleObjects(m_VectorThreadHandle.size(), m_VectorThreadHandle.data(), TRUE, INFINITE);
	m_VectorThreadHandle.clear();
}
