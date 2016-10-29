//#include "stdafx.h"
#include "SyncSignal.h"


CSyncSignal::CSyncSignal()
{
	m_DwCalibrationFrameCount = 0;
	m_DwSamplingCount = 0;
	pClassColorCameraCapture = new CBaslerCameraCapture;
}


CSyncSignal::~CSyncSignal()
{
#if defined(HOST)
	DAQmxStopTask(hTaskTest);
	DAQmxClearTask(hTaskTest);

	DAQmxStopTask(hTaskSeq);
	DAQmxClearTask(hTaskSeq);
#endif

	delete pClassColorCameraCapture;
}


int32 CSyncSignal::CounterCallback(TaskHandle taskHandle, int32 signalID, void *callbackData)
{
	CSyncSignal* pClass = (CSyncSignal*) callbackData;

	if ((pClass->m_DwSamplingCount - 1) % 2 == 0)
	{
		pClass->CallbackFunction();
	}

	pClass->m_DwSamplingCount++;
	//cout << pClass->m_DwSamplingCount << endl;

	return 0;
}


void CSyncSignal::CallbackFunction()
{
#if defined(KINECT)
	pClassKinectCapture->GrabDepthFrame();
#endif
}


BOOL CSyncSignal::Initialize()
{
	int retval = 0;

#if defined(HOST)
	retval += DAQmxCreateTask("test", &hTaskTest);
	retval += DAQmxCreateTask("seq", &hTaskSeq);

	if (retval != 0)
		return FALSE;

	retval += DAQmxCreateCOPulseChanFreq(hTaskTest, "Dev1/ctr0", "", DAQmx_Val_Hz, DAQmx_Val_Low, 0, 30, 0.5);
	//retval += DAQmxCreateCOPulseChanFreq(hTaskTest, "Dev1/ctr1", "", DAQmx_Val_Hz, DAQmx_Val_Low, 0, 30, 0.5);
	retval += DAQmxCreateCOPulseChanFreq(hTaskSeq, "Dev1/ctr0", "", DAQmx_Val_Hz, DAQmx_Val_Low, 0, 30, 0.5);
	//retval += DAQmxCreateCOPulseChanFreq(hTaskSeq, "Dev1/ctr1", "", DAQmx_Val_Hz, DAQmx_Val_Low, 0, 30, 0.5);

	if (retval != 0)
		return FALSE;

	retval += DAQmxCfgImplicitTiming(hTaskTest, DAQmx_Val_ContSamps, 1000);
	retval += DAQmxCfgImplicitTiming(hTaskSeq, DAQmx_Val_ContSamps, 1000);

	if (retval != 0)
		return FALSE;

	retval += DAQmxRegisterSignalEvent(hTaskSeq, DAQmx_Val_CounterOutputEvent, 0, CounterCallback, this);
	retval += DAQmxStartTask(hTaskTest);

	if (retval != 0)
		return FALSE;
#endif


	retval = pClassColorCameraCapture->Initialize();
	cout << "Color Cameras: " << retval << endl;


#if defined(KINECT)
	pClassKinectCapture->Initialize();
#endif


	return TRUE;
}


void CSyncSignal::Clear()
{
	m_DwCalibrationFrameCount = 0;
}


void CSyncSignal::Start()
{
	DAQmxStopTask(hTaskTest);
	DAQmxStartTask(hTaskSeq);

	pClassColorCameraCapture->SequenceCapture();
}


void CSyncSignal::Stop()
{
	DAQmxStopTask(hTaskSeq);
	DAQmxStartTask(hTaskTest);

	pClassColorCameraCapture->WaitForThreadExit();
}


void CSyncSignal::Snapshot()
{
	vector<Mat> vectorColorCalibration;

	pClassColorCameraCapture->CalibrationCapture(vectorColorCalibration);

	for (int i = 0; i < vectorColorCalibration.size(); i++)
	{
		imwrite("result/color" + to_string(i) + "_" + to_string(m_DwCalibrationFrameCount) + ".bmp", vectorColorCalibration[i]);
	}

	waitKey(10);
	cout << "Snapshot: " << m_DwCalibrationFrameCount++ << endl;
}

void CSyncSignal::ImageGrab(vector<Mat> &vec_color)
{
	pClassColorCameraCapture->CalibrationCapture(vec_color);
}
