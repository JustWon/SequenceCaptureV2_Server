#pragma once

#include <vector>
#include <opencv2/highgui/highgui.hpp>
#include <pylon/PylonIncludes.h>
#include <pylon/gige/BaslerGigEInstantCamera.h>

using namespace std;
using namespace cv;

using namespace Pylon;
using namespace Basler_GigECameraParams;

class CBaslerCameraCapture
{
public:
	CBaslerCameraCapture();
	~CBaslerCameraCapture();

protected:
	bool bThread;

	PylonAutoInitTerm autoInitTerm;
	vector<CBaslerGigEInstantCamera*> m_VectorCameras;
	vector<HANDLE> m_VectorThreadHandle;

public:
	size_t Initialize();
	void SequenceCapture();
	void CalibrationCapture(vector<Mat>& vectorImages);
	static void CaptureThreadParent(void* arg, CBaslerGigEInstantCamera* pCamera);
	void WaitForThreadExit();

protected:
	void CaptureThread(CBaslerGigEInstantCamera* pCamera);
};

