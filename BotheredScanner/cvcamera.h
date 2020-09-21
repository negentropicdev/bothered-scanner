#ifndef CVCAMERA_H
#define CVCAMERA_H

#include <QThread>
#include <QMutex>

#include "qmat.h"

#include <opencv2/opencv.hpp>

void enumerateCameras(QStringList *list);

class CVCamera : public QThread
{
    Q_OBJECT

public:
    CVCamera();
    ~CVCamera();

    enum State {
        Idle,
        Opening,
        Preview,
        Capture,
        Closing
    };

    struct CameraProps {
        double width;
        double height;
        double fps;
        double brightness;
        double contrast;
        double saturation;
        double hue;
        double gain;
        double exposure;
        double sharpness;
        double gamma;
        double temperature;
        double zoom;
        double focus;
        double iso;
        double backlight;
    };

    struct ControlCaps {
        bool width;
        bool height;
        bool fps;
        bool brightness;
        bool contrast;
        bool saturation;
        bool hue;
        bool gain;
        bool exposure;
        bool sharpness;
        bool gamma;
        bool temperature;
        bool zoom;
        bool focus;
        bool iso;
        bool backlight;
    };

    bool setResolution(int width, int height);
    void getResolution(int &width, int &height);

    bool setProp(int prop, double val);
    double getProp(int prop);

    void showProps();

signals:
    void newFrame(QMat img);
    void cameraReady();

public slots:
    void stop();
    bool openCam(int index);

protected:
    void run();

    void updateProps();

private:
    void applyResolution();

    //protects access to thread control values
    QMutex threadMutex;
    bool close;

    //protects camera access while
    QMutex capMutex;

    cv::VideoCapture *cap;

    ControlCaps controlCaps;
    CameraProps cameraProps;

    State state;

    int width;
    int height;
};

#endif // CVCAMERA_H
