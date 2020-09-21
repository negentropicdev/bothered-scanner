#include "qcvcamera.h"

#include <QCamera>
#include <QCameraInfo>

void enumerateCameraNames(QStringList *list) {
    if (list == 0) {
        return;
    }

    list->clear();
    QList<QCameraInfo> cams = QCameraInfo::availableCameras();

    for (int i = 0; i < cams.count(); ++i) {
        list->append(cams[i].description());
    }
}

QCVCamera::QCVCamera(QObject *parent) : QObject(parent)
{
    frameWidth = 0;
    frameHeight = 0;
    cameraIndex = -1;

    cap = new cv::VideoCapture();

    this->moveToThread(&captureThread);
    //connect(&captureThread, &QThread::finished, this, &QCVCamera::captureFinished);
    captureThread.start();
}

QCVCamera::~QCVCamera()
{
    captureThread.quit();
    captureThread.wait();

    if (isOpened()) {
        cap->release();
        delete cap;
    }
}

bool QCVCamera::isOpened()
{
    return cap != 0 && cap->isOpened();
}

bool QCVCamera::wait()
{
    return captureThread.wait();
}

void QCVCamera::showProperties()
{
    cap->set(cv::CAP_PROP_SETTINGS, 1);
}

void QCVCamera::openCamera(int index)
{
    if (index == cameraIndex) {
        return;
    }

    //mutex.lock();

    bool res;

    res = cap->open(index, cv::CAP_DSHOW);
    res |= cap->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    res |= cap->set(cv::CAP_PROP_FPS, 30);
    res |= cap->set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    res |= cap->set(cv::CAP_PROP_FRAME_HEIGHT, 720);

    //setResolution(1920, 1080);

    if (isOpened()) {
        cameraIndex = index;
    } else {
        delete cap;
        cap = 0;
        cameraIndex = -1;
    }

    //mutex.unlock();

    if (cap != 0) {
        frameWidth = cap->get(cv::CAP_PROP_FRAME_WIDTH);
        frameHeight = cap->get(cv::CAP_PROP_FRAME_HEIGHT);

        //updateFPS();

        //captureTimer.start();

        emit resolutionChanged(frameWidth, frameHeight);
        emit cameraReady();
    } else {
        emit cameraOpenError();
    }
}

void QCVCamera::closeCamera()
{
    //mutex.lock();

    if (cap->isOpened()) {
        cap->release();
    }

    //mutex.unlock();

    emit cameraClosed();
}

void QCVCamera::setResolution(int width, int height)
{
    if (width == frameWidth && height == frameHeight) {
        return;
    }

    bool success = true;

    //mutex.lock();

    if (isOpened()) {
        success &= cap->set(cv::CAP_PROP_FRAME_WIDTH, width);
        success &= cap->set(cv::CAP_PROP_FRAME_HEIGHT, height);
        cap->set(cv::CAP_PROP_FPS, 30);

        frameWidth = cap->get(cv::CAP_PROP_FRAME_WIDTH);
        frameHeight = cap->get(cv::CAP_PROP_FRAME_HEIGHT);

        if (frameWidth != width || frameHeight != height) {
            success = false;
        }
    }

    //mutex.unlock();

    if (success) {
        emit resolutionChanged(frameWidth, frameHeight);

        //Needed?
        emit cameraReady();
    } else {
        emit resolutionError();
    }
}

void QCVCamera::capture()
{
    if (isOpened()) {
        cv::Mat frame;
        bool captured = cap->read(frame);

        if (captured && !frame.empty()) {
            emit newFrame(QMat(frame));
        }

        emit captureFinished();
    } else {
        emit captureSkipped("No open camera");
    }
}
