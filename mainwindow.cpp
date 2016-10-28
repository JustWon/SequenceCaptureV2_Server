#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

const char SERVER_IP[] = "127.0.0.1";
const unsigned short PORT_NUMBER = 31400;


class Session
{
public:

	Session(boost::asio::io_service& io_service)
		: m_Socket(io_service)
	{
	}

	boost::asio::ip::tcp::socket& Socket()
	{
		return m_Socket;
	}

	void PostReceive()
	{
		memset(&m_ReceiveBuffer, '\0', sizeof(m_ReceiveBuffer));

		m_Socket.async_read_some
		(
			boost::asio::buffer(m_ReceiveBuffer),
			boost::bind(&Session::handle_receive, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred)
		);
	}

	void PostQuery(const string query)
	{
		char szMessage[128] = { 0, };
		sprintf_s(szMessage, 128 - 1, query.c_str());
		m_WriteMessage = szMessage;

		boost::asio::async_write(m_Socket, boost::asio::buffer(m_WriteMessage),
			boost::bind(&Session::handle_write, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred)
		);
	}

private:
	void handle_write(const boost::system::error_code& /*error*/, size_t /*bytes_transferred*/)
	{
	}

	void handle_receive(const boost::system::error_code& error, size_t bytes_transferred)
	{
		if (error)
		{
			if (error == boost::asio::error::eof)
			{
				std::cout << "클라이언트와 연결이 끊어졌습니다" << std::endl;
			}
			else
			{
				std::cout << "error No: " << error.value() << " error Message: " << error.message() << std::endl;
			}
		}
		else
		{
			const std::string strRecvMessage = m_ReceiveBuffer.data();
			std::cout << "클라이언트에서 받은 메시지: " << strRecvMessage << ", 받은 크기: " << bytes_transferred << std::endl;

			PostReceive();
		}
	}

	boost::asio::ip::tcp::socket m_Socket;
	std::string m_WriteMessage;
	std::array<char, 128> m_ReceiveBuffer;
};

class TCP_Server
{
public:
	TCP_Server(boost::asio::io_service& io_service)
		: m_acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT_NUMBER))
	{
		m_pSession = nullptr;
		StartAccept();
	}

	~TCP_Server()
	{
		if (m_pSession != nullptr)
		{
			delete m_pSession;
		}
	}

public:
	void StartAccept()
	{
		std::cout << "클라이언트 접속 대기....." << std::endl;

		m_pSession = new Session(m_acceptor.get_io_service());

		m_acceptor.async_accept(m_pSession->Socket(),
			boost::bind(&TCP_Server::handle_accept,
				this,
				m_pSession,
				boost::asio::placeholders::error)
		);
	}

	void handle_accept(Session* pSession, const boost::system::error_code& error)
	{
		if (!error)
		{
			std::cout << "클라이언트 접속 성공" << std::endl;

			pSession->PostReceive();
		}
	}

	int m_nSeqNumber;
	boost::asio::ip::tcp::acceptor m_acceptor;
	Session* m_pSession;
};

boost::asio::io_service io_service;
TCP_Server server(io_service);

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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showEvent(QShowEvent* event) {
	QWidget::showEvent(event);

	boost::thread thread(boost::bind(&boost::asio::io_service::run, &io_service));
}

void MainWindow::on_pushButton_StillShotCapture_clicked()
{
	server.m_pSession->PostQuery("still_capture");

	kinect.update();
	kinect.drawDepth();
	kinect.saveDepth();
}

void MainWindow::on_pushButton_SequenceCapture_clicked()
{
	//for (;;)
	//{
	//	std::array<char, 128> buf; 
	//	buf.assign(0);
	//	boost::system::error_code error;
	//	size_t len = socket_.read_some(boost::asio::buffer(buf), error);
	//
	//	if (error) {
	//		if (error == boost::asio::error::eof) {
	//
	//			std::cout << "클라이언트와 연결이 끊어졌습니다" << std::endl;
	//		}
	//		else
	//		{
	//			std::cout << "error No: " << error.value() << " error Message: " << error.message() << std::endl;
	//		}
	//		break;
	//	}
	//
	//	std::cout << "클라이언트에서 받은 메시지: " << &buf[0] << std::endl;
	//	char szMessage[128] = { 0, }; 
	//	sprintf_s(szMessage, 128 - 1, "Re:%s", &buf[0]);
	//	int nMsgLen = strnlen_s(szMessage, 128 - 1);
	//
	//	std::cout << boost::chrono::system_clock::now() << '\n';
	//	kinect.update();
	//	kinect.drawDepth();
	//	kinect.showDepth();
	//	cv::waitKey(30);
	//
	//	boost::system::error_code ignored_error;
	//	socket_.write_some(boost::asio::buffer(szMessage, nMsgLen), ignored_error);
	//	std::cout << "클라이언트에 보낸 메시지: " << szMessage << std::endl;
	//}
}

void MainWindow::on_pushButton_StreamOn_clicked()
{
	server.m_pSession->PostQuery("stream_on");
	stream_on_flag = true;
	boost::thread thread(boost::bind(&MainWindow::stream_on, this));
}

void MainWindow::on_pushButton_StreamOff_clicked()
{
	server.m_pSession->PostQuery("stream_off");
	stream_on_flag = false;
}
