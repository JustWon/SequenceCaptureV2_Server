#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

const char SERVER_IP[] = "127.0.0.1";
const unsigned short PORT_NUMBER = 31400;

boost::asio::io_service io_service;
boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), PORT_NUMBER);
boost::asio::ip::tcp::acceptor acceptor(io_service, endpoint);
boost::asio::ip::tcp::socket socket_(io_service);

QImage Mat2QImage_color(const cv::Mat3b &src) {
	QImage dest(src.cols, src.rows, QImage::Format_ARGB32);
	for (int y = 0; y < src.rows; ++y) {
		const cv::Vec3b *srcrow = src[y];
		QRgb *destrow = (QRgb*)dest.scanLine(y);
		for (int x = 0; x < src.cols; ++x) {
			destrow[x] = qRgba(srcrow[x][2], srcrow[x][1], srcrow[x][0], 255);
		}
	}
	return dest;
}
QImage Mat2QImage_depth(const cv::Mat_<double> &src)
{
	double scale = 1;
	QImage dest(src.cols, src.rows, QImage::Format_ARGB32);
	for (int y = 0; y < src.rows; ++y) {
		const double *srcrow = src[y];
		QRgb *destrow = (QRgb*)dest.scanLine(y);
		for (int x = 0; x < src.cols; ++x) {
			unsigned int color = srcrow[x] * scale;
			destrow[x] = qRgba(color, color, color, 255);
		}
	}
	return dest;
}


void MainWindow::test_thread()
{
	kinect.update();
	kinect.drawDepth();
	kinect.showDepth();

	Mat mat_img = kinect.scaleMat;
	QImage q_image;

	cv::resize(mat_img, mat_img, cv::Size(ui->label_KinectDepth->width(), ui->label_KinectDepth->height()));
	q_image = Mat2QImage_color(mat_img);
	ui->label_KinectDepth->setPixmap(QPixmap::fromImage(q_image));
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	m_thread = new ThreadTest(this);
	connect(m_thread, SIGNAL(tick(int)), this, SLOT(test_thread(void)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showEvent(QShowEvent* event) {
	QWidget::showEvent(event);

	std::cout << "Waiting for a client..." << std::endl;
	acceptor.accept(socket_);
	std::cout << "Client connected" << std::endl;
}

void MainWindow::on_pushButton_StillShotCapture_clicked()
{
	std::array<char, 128> buf; buf.assign(0);
	char szMessage[128] = { 0, }; sprintf_s(szMessage, 128 - 1, "Re:%s", &buf[0]);
	int nMsgLen = strnlen_s(szMessage, 128 - 1);

	boost::system::error_code error;
	socket_.write_some(boost::asio::buffer(szMessage, nMsgLen), error);
	std::cout << "클라이언트에 보낸 메시지: " << szMessage << std::endl;

	//std::cout << boost::chrono::system_clock::now() << '\n';
	kinect.update();
	kinect.drawDepth();
	kinect.saveDepth();
}

void MainWindow::on_pushButton_SequenceCapture_clicked()
{
	for (;;)
	{
		std::array<char, 128> buf; 
		buf.assign(0);
		boost::system::error_code error;
		size_t len = socket_.read_some(boost::asio::buffer(buf), error);

		if (error) {
			if (error == boost::asio::error::eof) {

				std::cout << "클라이언트와 연결이 끊어졌습니다" << std::endl;
			}
			else
			{
				std::cout << "error No: " << error.value() << " error Message: " << error.message() << std::endl;
			}
			break;
		}

		std::cout << "클라이언트에서 받은 메시지: " << &buf[0] << std::endl;
		char szMessage[128] = { 0, }; 
		sprintf_s(szMessage, 128 - 1, "Re:%s", &buf[0]);
		int nMsgLen = strnlen_s(szMessage, 128 - 1);

		std::cout << boost::chrono::system_clock::now() << '\n';
		kinect.update();
		kinect.drawDepth();
		kinect.showDepth();
		cv::waitKey(30);

		boost::system::error_code ignored_error;
		socket_.write_some(boost::asio::buffer(szMessage, nMsgLen), ignored_error);
		std::cout << "클라이언트에 보낸 메시지: " << szMessage << std::endl;
	}
}

void MainWindow::on_pushButton_StreamOn_clicked()
{
	m_thread->start();

	char szMessage[128] = { 0, };
	sprintf_s(szMessage, 128 - 1, "stream_on");
	int nMsgLen = strnlen_s(szMessage, 128 - 1);
	boost::system::error_code error;
	socket_.write_some(boost::asio::buffer(szMessage, nMsgLen), error);
	std::cout << "클라이언트에 보낸 메시지: " << szMessage << std::endl;
}

void MainWindow::on_pushButton_StreamOff_clicked()
{
	m_thread->terminate();

	char szMessage[128] = { 0, };
	sprintf_s(szMessage, 128 - 1, "stream_off");
	int nMsgLen = strnlen_s(szMessage, 128 - 1);
	boost::system::error_code error;
	socket_.write_some(boost::asio::buffer(szMessage, nMsgLen), error);
	std::cout << "클라이언트에 보낸 메시지: " << szMessage << std::endl;

	Mat mat_img = Mat::zeros(640, 480, CV_32FC1);
	QImage q_image;
	cv::resize(mat_img, mat_img, cv::Size(ui->label_KinectDepth->width(), ui->label_KinectDepth->height()));
	q_image = Mat2QImage_depth(mat_img);

	ui->label_KinectDepth->setPixmap(QPixmap::fromImage(q_image));
}
