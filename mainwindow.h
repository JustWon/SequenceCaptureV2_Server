#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// I don't know what it is... but it works..
#define _WIN32_WINNT 0x0600

#include <QMainWindow>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include "app.h"
#include "threadtest.h"
#include <opencv2\imgproc\imgproc.hpp>
#include <vector>

using namespace std;
using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
	void showEvent(QShowEvent *ev);
	bool stream_on_flag = false;
	void stream_on();

private slots:
	void on_pushButton_StillShotCapture_clicked();
    void on_pushButton_SequenceCapture_clicked();
    void on_pushButton_StreamOn_clicked();
    void on_pushButton_StreamOff_clicked();

    void on_pushButton_Quit_clicked();

private:
    Ui::MainWindow *ui;

	Kinect kinect;
};

#endif // MAINWINDOW_H
