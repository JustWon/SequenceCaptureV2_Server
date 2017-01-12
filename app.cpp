#include "app.h"
#include "util.h"

#include <thread>
#include <chrono>

// Constructor
Kinect::Kinect()
{
    // Initialize
    initialize();
}

// Destructor
Kinect::~Kinect()
{
    // Finalize
    finalize();
}

// Processing
void Kinect::run()
{
    // Main Loop
    while( true ){
        // Update Data
        update();

        // Draw Data
        draw();

        // Show Data
        show();

        // Key Check
        const int key = cv::waitKey( 10 );
        if( key == VK_ESCAPE ){
            break;
        }
    }
}

// Initialize
void Kinect::initialize()
{
    cv::setUseOptimized( true );

    // Initialize Sensor
    initializeSensor();

    // Initialize Depth
    initializeDepth();

	// Initialize Infrared
	initializeInfrared();

	// Initialize Color
	initializeColor();

    // Wait a Few Seconds until begins to Retrieve Data from Sensor ( about 2000-[ms] )
    std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
}

// Initialize Sensor
inline void Kinect::initializeSensor()
{
    // Open Sensor
    ERROR_CHECK( GetDefaultKinectSensor( &kinect ) );

    ERROR_CHECK( kinect->Open() );

    // Check Open
    BOOLEAN isOpen = FALSE;
    ERROR_CHECK( kinect->get_IsOpen( &isOpen ) );
    if( !isOpen ){
        throw std::runtime_error( "failed IKinectSensor::get_IsOpen( &isOpen )" );
    }
}

// Initialize Depth
inline void Kinect::initializeDepth()
{
    // Open Depth Reader
    ComPtr<IDepthFrameSource> depthFrameSource;
    ERROR_CHECK( kinect->get_DepthFrameSource( &depthFrameSource ) );
    ERROR_CHECK( depthFrameSource->OpenReader( &depthFrameReader ) );

    // Retrieve Depth Description
    ComPtr<IFrameDescription> depthFrameDescription;
    ERROR_CHECK( depthFrameSource->get_FrameDescription( &depthFrameDescription ) );
    ERROR_CHECK( depthFrameDescription->get_Width( &depthWidth ) ); // 512
    ERROR_CHECK( depthFrameDescription->get_Height( &depthHeight ) ); // 424
    ERROR_CHECK( depthFrameDescription->get_BytesPerPixel( &depthBytesPerPixel ) ); // 2

    // Retrieve Depth Reliable Range
    UINT16 minReliableDistance;
    UINT16 maxReliableDistance;
    ERROR_CHECK( depthFrameSource->get_DepthMinReliableDistance( &minReliableDistance ) ); // 500
    ERROR_CHECK( depthFrameSource->get_DepthMaxReliableDistance( &maxReliableDistance ) ); // 4500
    //std::cout << "Depth Reliable Range : " << minReliableDistance << " - " << maxReliableDistance << std::endl;

    // Allocation Depth Buffer
    depthBuffer.resize( depthWidth * depthHeight );
}

inline void Kinect::initializeColor()
{
	// Open Color Reader
	ComPtr<IColorFrameSource> colorFrameSource;
	ERROR_CHECK(kinect->get_ColorFrameSource(&colorFrameSource));
	ERROR_CHECK(colorFrameSource->OpenReader(&colorFrameReader));

	// Retrieve Color Description
	ComPtr<IFrameDescription> colorFrameDescription;
	ERROR_CHECK(colorFrameSource->CreateFrameDescription(ColorImageFormat::ColorImageFormat_Bgra, &colorFrameDescription));
	ERROR_CHECK(colorFrameDescription->get_Width(&colorWidth)); // 1920
	ERROR_CHECK(colorFrameDescription->get_Height(&colorHeight)); // 1080
	ERROR_CHECK(colorFrameDescription->get_BytesPerPixel(&colorBytesPerPixel)); // 4
																				// Allocation Color Buffer
	colorBuffer.resize(colorWidth * colorHeight * colorBytesPerPixel);
}

// Initialize Infrared
inline void Kinect::initializeInfrared()
{
	// Open Infrared Reader
	ComPtr<IInfraredFrameSource> infraredFrameSource;
	ERROR_CHECK(kinect->get_InfraredFrameSource(&infraredFrameSource));
	ERROR_CHECK(infraredFrameSource->OpenReader(&infraredFrameReader));

	// Retrieve Infrared Description
	ComPtr<IFrameDescription> infraredFrameDescription;
	ERROR_CHECK(infraredFrameSource->get_FrameDescription(&infraredFrameDescription));
	ERROR_CHECK(infraredFrameDescription->get_Width(&infraredWidth)); // 512
	ERROR_CHECK(infraredFrameDescription->get_Height(&infraredHeight)); // 424
	ERROR_CHECK(infraredFrameDescription->get_BytesPerPixel(&infraredBytesPerPixel)); // 2

																					  // Allocation Depth Buffer
	infraredBuffer.resize(infraredWidth * infraredHeight);
}

// Finalize
void Kinect::finalize()
{
    cv::destroyAllWindows();

    // Close Sensor
    if( kinect != nullptr ){
        kinect->Close();
    }
}

// Update Data
void Kinect::update()
{
    // Update Depth
    updateDepth();

	// Update Infrared
	updateInfrared();

	// Update Color
	updateColor();
}

// Update Depth
inline void Kinect::updateDepth()
{
    // Retrieve Depth Frame
    ComPtr<IDepthFrame> depthFrame;
    const HRESULT ret = depthFrameReader->AcquireLatestFrame( &depthFrame );
    if( FAILED( ret ) ){
        return;
    }

    // Retrieve Depth Data
    ERROR_CHECK( depthFrame->CopyFrameDataToArray( static_cast<UINT>( depthBuffer.size() ), &depthBuffer[0] ) );
}
// Update Infrared
inline void Kinect::updateInfrared()
{
	// Retrieve Infrared Frame
	ComPtr<IInfraredFrame> infraredFrame;
	const HRESULT ret = infraredFrameReader->AcquireLatestFrame(&infraredFrame);
	if (FAILED(ret)) {
		return;
	}

	// Retrieve Infrared Data
	ERROR_CHECK(infraredFrame->CopyFrameDataToArray(static_cast<UINT>(infraredBuffer.size()), &infraredBuffer[0]));
}

// Update Color
inline void Kinect::updateColor()
{
	// Retrieve Color Frame
	ComPtr<IColorFrame> colorFrame;
	const HRESULT ret = colorFrameReader->AcquireLatestFrame(&colorFrame);
	if (FAILED(ret)) {
		return;
	}

	// Convert Format ( YUY2 -> BGRA )
	ERROR_CHECK(colorFrame->CopyConvertedFrameDataToArray(static_cast<UINT>(colorBuffer.size()), &colorBuffer[0], ColorImageFormat::ColorImageFormat_Bgra));
}

// Draw Data
void Kinect::draw()
{
    // Draw Depth
    drawDepth();

	// Draw Infrared
	drawInfrared();

	// Draw Color
	drawColor();
}

// Draw Depth
inline void Kinect::drawDepth()
{
    // Create cv::Mat from Depth Buffer
    depthMat = cv::Mat( depthHeight, depthWidth, CV_16UC1, &depthBuffer[0] );
}

// Draw Infrared
inline void Kinect::drawInfrared()
{
	// Create cv::Mat from Infrared Buffer
	infraredMat = cv::Mat(infraredHeight, infraredWidth, CV_16UC1, &infraredBuffer[0]);
}

// Draw Color
inline void Kinect::drawColor()
{
	// Create cv::Mat from Color Buffer
	colorMat = cv::Mat(colorHeight, colorWidth, CV_8UC4, &colorBuffer[0]);
}

// Show Data
void Kinect::show()
{
    // Show Depth
    showDepth();

	// Show Infrared
	showInfrared();

	// Show Color
	showColor();
}

// Show Depth
inline void Kinect::showDepth()
{
    if( depthMat.empty() ){
        return;
    }

    // Scaling ( 0-8000 -> 255-0 )
    depthMat.convertTo( scaleMat, CV_8U, -255.0 / 8000.0, 255.0 );
    cv::applyColorMap(scaleMat, scaleMat, cv::COLORMAP_BONE );
}

// Show Infrared
inline void Kinect::showInfrared()
{
	if (infraredMat.empty()) {
		return;
	}

	// Scaling ( 0b1111'1111'0000'0000 -> 0b1111'1111 )
	cv::Mat infraredScaleMat(infraredHeight, infraredWidth, CV_8UC1);
	infraredScaleMat.forEach<uchar>([&](uchar &p, const int* position) {
		p = infraredMat.at<ushort>(position[0], position[1]) >> 8;
	});

	this->scaleInfraredMat = infraredScaleMat;

	// Show Image
	//cv::imshow("Infrared", infraredScaleMat); 
	//cv::waitKey();
}

// Show Color
inline void Kinect::showColor()
{
	if (colorMat.empty()) {
		return;
	}

	// Resize Image
	cv::Mat resizeMat;
	const double scale = 0.5;
	cv::resize(colorMat, resizeMat, cv::Size(), scale, scale);

	// Show Image
	cv::imshow("Color", resizeMat);
	cv::waitKey();
}

// Save Depth
void Kinect::saveDepth(string dir_path, int cnt)
{
	if (depthMat.empty()) {
		return;
	}

	// Save Image 
	flip(depthMat, depthMat, 1);
	cvSaveImage((dir_path + "kinect_depth" + to_string(cnt) + ".png").c_str() , &IplImage(depthMat));
}

// Save Depth
void Kinect::saveInfrared(string dir_path, int cnt)
{
	if (scaleInfraredMat.empty()) {
		return;
	}

	// Save Image 
	flip(scaleInfraredMat, scaleInfraredMat, 1);
	cvSaveImage((dir_path + "kinect_infrared" + to_string(cnt) + ".png").c_str(), &IplImage(scaleInfraredMat));
}