//---------------------------------------------------------------------------
//  worker.cpp
//---------------------------------------------------------------------------
#include "worker.h"
#include <QTimer>
#include <QEventLoop>

#include <VX/vx.h>
#include <NVX/nvx_timer.hpp>

#include "NVXIO/FrameSource.hpp"
#include "NVXIO/Render.hpp"
#include "NVXIO/Application.hpp"
#include "NVXIO/SyncTimer.hpp"
#include "NVXIO/Utility.hpp"

#include "NvUtils.h"

//---------------------------------------------------------------------------
//  Worker
//---------------------------------------------------------------------------
Worker::Worker(QObject *parent) : QObject(parent)
{
    _device     = 0;
    _path       = "";
    _working    = false;
    _abort      = false;
    _isVideoFile= false;
}

//---------------------------------------------------------------------------
//  requestWork
//---------------------------------------------------------------------------
void Worker::requestWork(int device, QString path)
{
    mutex.lock();
    _device  = device;
    _path    = path;
    _working = true;
    _abort = false;
    qDebug() << "Request worker start in Thread " << thread()->currentThreadId();
    mutex.unlock();

    emit workRequested();
}

//---------------------------------------------------------------------------
//  abort
//---------------------------------------------------------------------------
void Worker::abort()
{
    mutex.lock();
    if (_working) {
        _abort = true;
        qDebug() << "Request worker aborting in Thread " << thread()->currentThreadId();
    }
    mutex.unlock();
}

//---------------------------------------------------------------------------
//  VideoCapture
//---------------------------------------------------------------------------
void Worker::VideoCapture(std::string input)
{
    nvxio::Application &app = nvxio::Application::get();
    nvxio::FrameSource::Parameters config;

    int argc = 1;
    char* argv = "";
    app.init(argc, &argv);
    app.setDescription("ZimonLabs Tegra");
    app.addOption('s', "source", "Input URI", nvxio::OptionHandler::string(&input));

    //-----------------------------------------------------------------------
    // Create OpenVX context
    //-----------------------------------------------------------------------
    nvxio::ContextGuard context;

    //-----------------------------------------------------------------------
    // Create a Frame Source
    //-----------------------------------------------------------------------
    std::unique_ptr<nvxio::FrameSource> source(nvxio::createDefaultFrameSource(context, input));
    if (!source)
    {
        std::cout << "Error: cannot open source!" << std::endl;
    }

    if (_device == 1) {
        source->open();
        config = source->getConfiguration();
    }

    vx_image frame = vxCreateImage(context, config.frameWidth,
                                   config.frameHeight, config.format);
    NVXIO_CHECK_REFERENCE(frame);

    std::unique_ptr<nvxio::SyncTimer> syncTimer = nvxio::createSyncTimer();
    syncTimer->arm(1. / app.getFPSLimit());

    nvxio::FrameSource::FrameStatus status = nvxio::FrameSource::OK;

    while (1)
    {
        mTimer.start();

        //-------------------------------------------------------------------
        //  Checks if the process should be aborted
        //-------------------------------------------------------------------
        mutex.lock();
        bool abort = _abort;
        mutex.unlock();

        if (abort) {
            qDebug() << "Aborting worker process in Thread " << thread()->currentThreadId();
            break;
        }

        //-------------------------------------------------------------------
        //  Frame is updated
        //-------------------------------------------------------------------
        status = source->fetch(frame);

        switch(status)
        {
        case nvxio::FrameSource::OK:
            {
                syncTimer->synchronize();

                vx_status ret = VX_SUCCESS;
                vx_uint32 width  = 1280;
                vx_uint32 height = 720;
                vx_uint8 *ptr = NULL;
                vx_rectangle_t rect;
                vxGetValidRegionImage(frame, &rect);

                int padding = 0;
                int scanlinebytes = width * 3;
                while ((scanlinebytes + padding) % 4 != 0)     // DWORD = 4 bytes
                    padding++;

                vx_imagepatch_addressing_t addr = {
                    width, height, sizeof(vx_uint8),
                    width * sizeof(vx_uint8),
                    VX_SCALE_UNITY, VX_SCALE_UNITY, 1, 1 };

                ret = vxAccessImagePatch(frame, &rect, padding, &addr, (void **)&ptr, VX_READ_ONLY);
                if (ret == VX_SUCCESS)
                {
                    const QImage image(ptr, width + padding, height, QImage::Format_RGBX8888);
                    emit frameCaptured(QPixmap::fromImage(image));
                }
                vxCommitImagePatch (frame, &rect, 0, &addr, ptr);
            }
            break;
        case nvxio::FrameSource::TIMEOUT:
            {
                // Do nothing
            }
            break;
        case nvxio::FrameSource::CLOSED:
            break;
        }
    }
}

//---------------------------------------------------------------------------
//  parseResolution
//---------------------------------------------------------------------------
static void parseResolution(const std::string & resolution, nvxio::FrameSource::Parameters & config)
{
    std::istringstream stream(resolution);
    std::string item;
    vx_uint32 * frameParams[] = { &config.frameWidth, &config.frameHeight };
    vx_uint32 index = 0;

    while (std::getline(stream, item, 'x'))
    {
        std::stringstream ss(item);
        ss >> *frameParams[index++];
    }
}

//---------------------------------------------------------------------------
//  CameraCapture
//---------------------------------------------------------------------------
void Worker::CameraCapture()
{
    nvxio::Application &app = nvxio::Application::get();

    nvxio::FrameSource::Parameters config;
    config.frameWidth = 1280;
    config.frameHeight = 720;

    //-----------------------------------------------------------------------
    //  Parse command line arguments
    //-----------------------------------------------------------------------
    std::string resolution = "1280x720", input = "device:///nvcamera";

    app.setDescription("This sample captures frames from NVIDIA GStreamer camera");
    app.addOption('r', "resolution", "Input frame resolution", nvxio::OptionHandler::oneOf(&resolution,
        { "2592x1944", "2592x1458", "1280x720", "640x480" }));
    app.addOption('f', "fps", "Frames per second", nvxio::OptionHandler::unsignedInteger(&config.fps,
        nvxio::ranges::atLeast(10u) & nvxio::ranges::atMost(120u)));

    int argc = 1;
    char* argv = "";
    app.init(argc, &argv);

    parseResolution(resolution, config);

    //-----------------------------------------------------------------------
    //  Create OpenVX context
    //-----------------------------------------------------------------------
    nvxio::ContextGuard context;

    //-----------------------------------------------------------------------
    //  Create a Frame Source
    //-----------------------------------------------------------------------
    std::unique_ptr<nvxio::FrameSource> source(nvxio::createDefaultFrameSource(context, input));
    if (!source)
    {
        std::cout << "Error: cannot open source!" << std::endl;
    }

    if (!source->setConfiguration(config))
    {
        std::cout << "Error: cannot setup configuration the framesource!" << std::endl;
    }

    if (!source->open())
    {
        std::cout << "Error: cannot open source!" << std::endl;
    }

    config = source->getConfiguration();

    vx_image frame = vxCreateImage(context, config.frameWidth,
                                   config.frameHeight, config.format);
    NVXIO_CHECK_REFERENCE(frame);

    nvxio::FrameSource::FrameStatus status = nvxio::FrameSource::OK;

    while (1)
    {
        mTimer.start();

        //-------------------------------------------------------------------
        //  Checks if the process should be aborted
        //-------------------------------------------------------------------
        mutex.lock();
        bool abort = _abort;
        mutex.unlock();

        if (abort) {
            qDebug() << "Aborting worker process in Thread " << thread()->currentThreadId();
            break;
        }

        //-------------------------------------------------------------------
        //  Frame is updated
        //-------------------------------------------------------------------
        status = source->fetch(frame);

        switch(status)
        {
        case nvxio::FrameSource::OK:
            {
                vx_status ret = VX_SUCCESS;
                vx_uint32 width  = config.frameWidth;
                vx_uint32 height = config.frameHeight;
                vx_uint8 *ptr = NULL;
                vx_rectangle_t rect;
                vxGetValidRegionImage(frame, &rect);

                vx_imagepatch_addressing_t addr = {
                    width, height, sizeof(vx_uint8),
                    width * sizeof(vx_uint8),
                    VX_SCALE_UNITY, VX_SCALE_UNITY, 1, 1 };

                ret = vxAccessImagePatch(frame, &rect, 0, &addr, (void **)&ptr, VX_READ_ONLY);
                if (ret == VX_SUCCESS)
                {
                    const QImage image(ptr, width, height, QImage::Format_RGBX8888);
                    emit frameCaptured(QPixmap::fromImage(image));
                }
                vxCommitImagePatch (frame, &rect, 0, &addr, ptr);
            }
            break;
        case nvxio::FrameSource::TIMEOUT:
            {
                // Do nothing
            }
            break;
        case nvxio::FrameSource::CLOSED:
            break;
        }

    }
}

//---------------------------------------------------------------------------
//  doWork
//---------------------------------------------------------------------------
void Worker::doWork()
{
    std::string input;

    qDebug()<< "Starting worker process in Thread " << thread()->currentThreadId();
    if (_device == 0) {
        CameraCapture();
    }
    else {
        input = _path.toStdString();
        VideoCapture(input);
    }

    //-----------------------------------------------------------------------
    //  Set _working to false, meaning the process can't be aborted anymore
    //-----------------------------------------------------------------------
    mutex.lock();
    _working = false;
    mutex.unlock();

    qDebug()<< "Worker process finished in Thread " << thread()->currentThreadId();
    emit finished();
}
