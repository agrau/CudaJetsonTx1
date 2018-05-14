#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "vectorAdd.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <memory>

#include <VX/vx.h>
#include <NVX/nvx_timer.hpp>

#include "NVXIO/FrameSource.hpp"
#include "NVXIO/Render.hpp"
#include "NVXIO/Application.hpp"
#include "NVXIO/SyncTimer.hpp"
#include "NVXIO/Utility.hpp"

#define PATCH_DIM 16

struct EventData
{
    EventData(): alive(true), pause(false) {}

    bool alive;
    bool pause;
};

//---------------------------------------------------------------------------
//  MainWindow
//---------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->lineEdit->setText("./data/cars.mp4");

    //-----------------------------------------------------------------------
    //  Video input Thread
    //-----------------------------------------------------------------------
    thread = new QThread();
    worker = new Worker();

    worker->moveToThread(thread);
    connect(worker, SIGNAL(workRequested()), thread, SLOT(start()));
    connect(thread, SIGNAL(started()), worker, SLOT(doWork()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()), Qt::DirectConnection);
    connect(worker, SIGNAL(frameCaptured(QPixmap)),this, SLOT(pricessFrameAndUpdateGUI(QPixmap)), Qt::AutoConnection);
}

//---------------------------------------------------------------------------
//  ~MainWindow
//---------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    worker->abort();
    thread->wait();
    delete thread;
    delete worker;
    delete ui;
}

//---------------------------------------------------------------------------
//  pricessFrameAndUpdateGUI
//---------------------------------------------------------------------------
void MainWindow::pricessFrameAndUpdateGUI(QPixmap imageOriginal)
{
    m_Image = imageOriginal;

    ui->m_videoFrame->setPixmap(imageOriginal);
    ui->m_videoFrame->show();
}

//---------------------------------------------------------------------------
//  on_pushButton_2_clicked
//---------------------------------------------------------------------------
void MainWindow::on_pushButtonCamera_clicked()
{
    worker->abort();
    thread->wait();
    worker->requestWork(0, "");
}

//---------------------------------------------------------------------------
//  on_pushButton_3_clicked
//---------------------------------------------------------------------------
void MainWindow::on_pushButtonVideo_clicked()
{
    worker->abort();
    thread->wait();

    QString input = ui->lineEdit->text();
    worker->requestWork(1, input);
}

//---------------------------------------------------------------------------
//  on_pushButtonStop
//---------------------------------------------------------------------------
void MainWindow::on_pushButtonStop_clicked()
{
    worker->abort();
    thread->exit();
}

//---------------------------------------------------------------------------
//  on_pushButtonCUDA
//---------------------------------------------------------------------------
void MainWindow::on_pushButtonCUDA_clicked()
{
    int ret;
    ret = vectorAdd();
}
