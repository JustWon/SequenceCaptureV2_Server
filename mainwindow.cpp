#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

const char SERVER_IP[] = "127.0.0.1";
const unsigned short PORT_NUMBER = 31400;

string g_sandbox_dir_path;

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

public:
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
			m_pSession->PostQuery(g_sandbox_dir_path);
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

void MainWindow::stream_on()
{
	while (stream_on_flag)
	{
		// Kinect v2 depth
		kinect.update();
		kinect.drawDepth();
		kinect.showDepth();

		Mat mat_kinect_stream = kinect.scaleMat;
		QImage q_image;

		cv::resize(mat_kinect_stream, mat_kinect_stream, cv::Size(ui->label_KinectDepth->width(), ui->label_KinectDepth->height()));
		q_image = Mat2QImage_color(mat_kinect_stream);
		ui->label_KinectDepth->setPixmap(QPixmap::fromImage(q_image));

		//// Basler colors
		vec_color.clear();
		signal->ImageGrab(vec_color);
		
		// color
		Mat mat_basler_stream = vec_color.at(0);
		Mat resized_img;
		cv::resize(mat_basler_stream, resized_img, cv::Size(ui->label_BaslerColor1->width(), ui->label_BaslerColor1->height()));
		q_image = Mat2QImage_color(resized_img);
		ui->label_BaslerColor1->setPixmap(QPixmap::fromImage(q_image));
			
		mat_basler_stream = vec_color.at(1);
		cv::resize(mat_basler_stream, resized_img, cv::Size(ui->label_BaslerColor2->width(), ui->label_BaslerColor2->height()));
		q_image = Mat2QImage_color(resized_img);
		ui->label_BaslerColor2->setPixmap(QPixmap::fromImage(q_image));
			
		mat_basler_stream = vec_color.at(2);
		cv::resize(mat_basler_stream, resized_img, cv::Size(ui->label_BaslerColor3->width(), ui->label_BaslerColor3->height()));
		q_image = Mat2QImage_color(resized_img);
		ui->label_BaslerColor3->setPixmap(QPixmap::fromImage(q_image));
			
		mat_basler_stream = vec_color.at(3);
		cv::resize(mat_basler_stream, resized_img, cv::Size(ui->label_BaslerColor4->width(), ui->label_BaslerColor4->height()));
		q_image = Mat2QImage_color(resized_img);
		ui->label_BaslerColor4->setPixmap(QPixmap::fromImage(q_image));
		

		if (stream_save_flag) {
			kinect.saveDepth(sequence_save_dir_path, sequence_cnt);
			for (int i = 0; i < vec_color.size(); i++)
			{
				imwrite(sequence_save_dir_path + "color" + to_string(i) + "_" + to_string(sequence_cnt) + ".bmp", vec_color[i]);
			}
			std::cout << sequence_save_dir_path + " seqeucne " + to_string(sequence_cnt) + " saved" << endl;
			sequence_cnt++;
		}
		if (still_save_flag) {
			kinect.saveDepth(still_save_dir_path, still_cnt);
			for (int i = 0; i < vec_color.size(); i++)
			{
				imwrite(still_save_dir_path + "color" + to_string(i) + "_" + to_string(still_cnt) + ".bmp", vec_color[i]);
			}
			std::cout << still_save_dir_path + " still shot " + to_string(still_cnt) + " saved" << endl;
			still_cnt++;
			still_save_flag = false;
		}		
	}

	// stream off
	{
		Mat mat_img = Mat::zeros(640, 480, CV_32FC1);
		QImage q_image;
		cv::resize(mat_img, mat_img, cv::Size(ui->label_KinectDepth->width(), ui->label_KinectDepth->height()));
		q_image = Mat2QImage_depth(mat_img);
		ui->label_KinectDepth->setPixmap(QPixmap::fromImage(q_image));

		cv::resize(mat_img, mat_img, cv::Size(ui->label_BaslerColor1->width(), ui->label_BaslerColor1->height()));
		q_image = Mat2QImage_depth(mat_img);
		ui->label_BaslerColor1->setPixmap(QPixmap::fromImage(q_image));
		ui->label_BaslerColor2->setPixmap(QPixmap::fromImage(q_image));
		ui->label_BaslerColor3->setPixmap(QPixmap::fromImage(q_image));
		ui->label_BaslerColor4->setPixmap(QPixmap::fromImage(q_image));
	}
}

std::wstring FormatTime(boost::posix_time::ptime now)
{
	using namespace boost::posix_time;
	static std::locale loc(std::wcout.getloc(),	new wtime_facet(L"%Y%m%d_%H%M%S"));
	std::basic_stringstream<wchar_t> wss;
	wss.imbue(loc);
	wss << now;
	return wss.str();
}

void MainWindow::showEvent(QShowEvent* event) {
	QWidget::showEvent(event);

	boost::thread thread(boost::bind(&boost::asio::io_service::run, &io_service));

	signal = new CSyncSignal;
	signal->Initialize();

	boost::filesystem::create_directory("result");
	using namespace boost::posix_time;
	ptime now = second_clock::local_time();

	std::wstring ws(FormatTime(now));
	std::wcout << "Sandbox name: " << ws << std::endl;

	sandbox_dir_path = "result/" + std::string(ws.begin(), ws.end());
	boost::filesystem::create_directory(sandbox_dir_path);

	still_save_dir_path = sandbox_dir_path + "/still/";
	boost::filesystem::create_directory(still_save_dir_path);

	sequence_save_dir_path = sandbox_dir_path + "/sequence/";
	boost::filesystem::create_directory(sequence_save_dir_path);

	g_sandbox_dir_path = sandbox_dir_path;
}

void MainWindow::on_pushButton_StillShotCapture_clicked()
{
	server.m_pSession->PostQuery("still_capture");
	still_save_flag = true;
}

void MainWindow::on_pushButton_SequenceCapture_clicked()
{
	if (!stream_save_flag)
	{
		stream_save_flag = true;
		server.m_pSession->PostQuery("sequence_capture_start");
	}
	else
	{
		stream_save_flag = false;
		server.m_pSession->PostQuery("sequence_capture_stop");
	}
}

void MainWindow::on_pushButton_StreamOn_clicked()
{
	server.m_pSession->PostQuery("stream_on");
	stream_on_flag = true;
	boost::thread thread(boost::bind(&MainWindow::stream_on, this));

	ui->pushButton_StreamOn->setEnabled(false);
	ui->pushButton_StreamOff->setEnabled(true);
	ui->pushButton_StillShotCapture->setEnabled(true);
	ui->pushButton_SequenceCapture->setEnabled(true);
}

void MainWindow::on_pushButton_StreamOff_clicked()
{
	server.m_pSession->PostQuery("stream_off");
	stream_on_flag = false;
	ui->pushButton_StreamOn->setEnabled(true);
	ui->pushButton_StreamOff->setEnabled(false);
	ui->pushButton_StillShotCapture->setEnabled(false);
	ui->pushButton_SequenceCapture->setEnabled(false);
}

void MainWindow::on_pushButton_Quit_clicked()
{
	server.m_pSession->PostQuery("stream_off");
	stream_on_flag = false;

	server.m_pSession->PostQuery("program_quit");
	server.m_pSession->m_Socket.close();
	close();
}
