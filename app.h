#ifndef __APP__
#define __APP__

#include <Windows.h>
#include <Kinect.h>
#include <opencv2/opencv.hpp>

#include <vector>
#include <string>

#include <wrl/client.h>
using namespace Microsoft::WRL;
using namespace std;

class Kinect
{
public:
    // Sensor
    ComPtr<IKinectSensor> kinect;

    // Reader
    ComPtr<IDepthFrameReader> depthFrameReader;
	ComPtr<IInfraredFrameReader> infraredFrameReader;
	ComPtr<IColorFrameReader> colorFrameReader;

    // Depth Buffer
    std::vector<UINT16> depthBuffer;
    int depthWidth;
    int depthHeight;
    unsigned int depthBytesPerPixel;
    cv::Mat depthMat;
	cv::Mat scaleMat;

	// Infrared Buffer
	std::vector<UINT16> infraredBuffer;
	int infraredWidth;
	int infraredHeight;
	unsigned int infraredBytesPerPixel;
	cv::Mat infraredMat;
	cv::Mat scaleInfraredMat;

	// Color Buffer
	std::vector<BYTE> colorBuffer;
	int colorWidth;
	int colorHeight;
	unsigned int colorBytesPerPixel;
	cv::Mat colorMat;

public:
    // Constructor
    Kinect();

    // Destructor
    ~Kinect();

    // Processing
    void run();

public:
    // Initialize
    void initialize();

    // Initialize Sensor
    inline void initializeSensor();

    // Initialize Depth
    inline void initializeDepth();

	// Initialize Color
	inline void initializeColor();

	// Initialize Infrared
	inline void initializeInfrared();

    // Finalize
    void finalize();

    // Update Data
    void update();

    // Update Depth
    inline void updateDepth();

	// Update Infrared
	inline void updateInfrared();
	
	// Update Color
	inline void updateColor();

    // Draw Data
    void draw();

    // Draw Depth
    inline void drawDepth();

	// Draw Infrared
	inline void drawInfrared();

	// Draw Color
	inline void drawColor();

    // Show Data
    void show();

    // Show Depth
    inline void showDepth();

	// Show Infrared
	inline void showInfrared();

	// Show Color
	inline void showColor();

	void saveDepth(string dir_path, int cnt);
	void saveInfrared(string dir_path, int cnt);
};

#endif // __APP__