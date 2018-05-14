#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QMutex>
#include <QTime>
#include <QThread>
#include <QDebug>
#include <QImage>
#include <QPixmap>
/*
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
*/

//---------------------------------------------------------------------------
//  Worker
//---------------------------------------------------------------------------
class Worker : public QObject
{
    Q_OBJECT

public:
    explicit Worker(QObject *parent = 0);
    //-----------------------------------------------------------------------
    //  @brief Requests the process to start
    //  It is thread safe as it uses #mutex to protect access to #_working variable.
    //-----------------------------------------------------------------------
    void requestWork(int device, QString rtsp);

    //-----------------------------------------------------------------------
    //  @brief Requests the process to abort
    //  It is thread safe as it uses #mutex to protect access to #_abort variable.
    //-----------------------------------------------------------------------
    void abort();

private:
    //-----------------------------------------------------------------------
    //  @brief Process is aborted when @em true
    //-----------------------------------------------------------------------
    bool _abort;

    //-----------------------------------------------------------------------
    //  @brief @em true when Worker is doing work
    //-----------------------------------------------------------------------
    bool    _working;
    int     _device;
    QString _path;
    bool    _isVideoFile;

    //-----------------------------------------------------------------------
    //  @brief Protects access to #_abort
    //-----------------------------------------------------------------------
    QMutex mutex;

    //-----------------------------------------------------------------------
    //  Timer
    //-----------------------------------------------------------------------
    QTime mTimer;

    //-----------------------------------------------------------------------
    //  VX Functions
    //-----------------------------------------------------------------------
    void VideoCapture(std::string input);
    void CameraCapture();

signals:
    //-----------------------------------------------------------------------
    //  @brief This signal is emitted when the Worker request to Work
    //  @sa requestWork()
    //-----------------------------------------------------------------------
    void workRequested();

    //-----------------------------------------------------------------------
    //  @brief frameCaptured
    //-----------------------------------------------------------------------
    void frameCaptured(QPixmap/*cv::Mat*/);

    //-----------------------------------------------------------------------
    //  @brief This signal is emitted when process is finished
    //-----------------------------------------------------------------------
    void finished();

public slots:
    //-----------------------------------------------------------------------
    //  @brief Does something
    //  Counting is interrupted if #_aborted is set to true.
    //-----------------------------------------------------------------------
    void doWork();
};

#endif // WORKER_H
