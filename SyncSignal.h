#pragma once

#include "BaslerCameraCapture.h"

#include <NIDAQmx.h>
#pragma comment(lib, "NIDAQmx.lib")

class CSyncSignal
{
public:
	CSyncSignal();
	~CSyncSignal();

private:
	DWORD m_DwCalibrationFrameCount;
	DWORD m_DwSamplingCount;
	TaskHandle hTaskTest;
	TaskHandle hTaskSeq;
	CBaslerCameraCapture* pClassColorCameraCapture;

protected:
	static int32 CVICALLBACK CounterCallback(TaskHandle taskHandle, int32 signalID, void *callbackData);
	void CallbackFunction();

public:
	BOOL Initialize();
	void Clear();
	void Start();
	void Stop();
	void Snapshot();

	void ImageGrab(vector<Mat>&, vector<Mat>&);
};

