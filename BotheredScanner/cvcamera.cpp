#include "cvcamera.h"

#include <QCamera>
#include <QCameraInfo>

CVCamera::CVCamera()
{
    cap = 0;
    close = false;
    state = Idle;
    width = 0;
    height = 0;
}

CVCamera::~CVCamera()
{
    if (isRunning()) {
        stop();
        wait();
    }

    if (cap != 0) {
        if (cap->isOpened()) {
            cap->release();
        }
    }
}

bool CVCamera::setResolution(int width, int height)
{
    if (cap == 0) {
        return false;
    }

    if (width == this->width && height == this->height) {
        return true;
    }

    cap->set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap->set(cv::CAP_PROP_FRAME_HEIGHT, height);

    if (cap->get(cv::CAP_PROP_FRAME_WIDTH) == width &&
            cap->get(cv::CAP_PROP_FRAME_HEIGHT) == height) {
        this->width = width;
        this->height = height;

        return true;
    }

    //unsuccessful, revert resolution
    cap->set(cv::CAP_PROP_FRAME_WIDTH, 0);
    cap->set(cv::CAP_PROP_FRAME_HEIGHT, 0);

    width = cap->get(cv::CAP_PROP_FRAME_WIDTH);
    height = cap->get(cv::CAP_PROP_FRAME_HEIGHT);

    return false;
}

void CVCamera::getResolution(int &width, int &height)
{
    width = this->width;
    height = this->height;
}

bool CVCamera::setProp(int prop, double val)
{
    if (cap == 0) {
        return false;
    }

    return cap->set(prop, val);
}

double CVCamera::getProp(int prop)
{
    return cap->get(prop);
}

void CVCamera::showProps()
{
    cap->set(cv::CAP_PROP_SETTINGS, 1);
}

void CVCamera::stop()
{
    threadMutex.lock();
    state = Closing;
    close = true;
    threadMutex.unlock();
}

bool CVCamera::openCam(int index)
{
    bool res = true;

    capMutex.lock();
    state = Opening;

    if (cap != 0) {
        if (cap->isOpened()) {
            cap->release();
        }

        delete cap;
        cap = 0;
    }

    cap = new cv::VideoCapture(index, cv::CAP_DSHOW);

    if (!cap->isOpened()) {
        qDebug() << "Unable to open camera" << index << "!";
        cap->release();
        delete cap;
        cap = 0;
        res = false;
    }

    capMutex.unlock();

    width = cap->get(cv::CAP_PROP_FRAME_WIDTH);
    height = cap->get(cv::CAP_PROP_FRAME_HEIGHT);

    emit cameraReady();

    if (res && !isRunning()) {
        start(HighPriority);
    }

    return res;
}

void CVCamera::run()
{
    cv::Mat frame, colorFixed;
    bool captured = false;
    bool shutdown = false;

    state = Capture;

    while (true) {
        threadMutex.lock();
        shutdown = close;
        threadMutex.unlock();

        if (shutdown) {
            break;
        }

        capMutex.lock();
        captured = cap->read(frame);
        capMutex.unlock();

        //emit the original image
        if (captured && !frame.empty()) {
            emit newFrame(QMat(frame));
        }
    }
}

void CVCamera::updateProps()
{

}

void CVCamera::applyResolution()
{
    //Try 1080p
    cap->set(cv::CAP_PROP_FRAME_WIDTH, 1920);
    cap->set(cv::CAP_PROP_FRAME_HEIGHT, 1080);

    if (cap->get(cv::CAP_PROP_FRAME_WIDTH) == 1920) {
        return;
    }

    //try 720p
    cap->set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    cap->set(cv::CAP_PROP_FRAME_HEIGHT, 720);

    if (cap->get(cv::CAP_PROP_FRAME_WIDTH) == 1280) {
        return;
    }

    //640x480
    cap->set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap->set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    if (cap->get(cv::CAP_PROP_FRAME_WIDTH) == 640) {
        return;
    }

    //320x240
    cap->set(cv::CAP_PROP_FRAME_WIDTH, 320);
    cap->set(cv::CAP_PROP_FRAME_HEIGHT, 240);

    if (cap->get(cv::CAP_PROP_FRAME_WIDTH) == 320) {
        return;
    }
}

void enumerateCameras(QStringList *list)
{
    if (list == 0) {
        return;
    }

    list->clear();
    QList<QCameraInfo> cams = QCameraInfo::availableCameras();

    for (int i = 0; i < cams.count(); ++i) {
        list->append(cams[i].description());
    }
}
