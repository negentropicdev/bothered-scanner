#ifndef QCVCAMERA_H
#define QCVCAMERA_H

#include <QThread>
#include <QMutex>
#include <QElapsedTimer>

#include "qmat.h"

void enumerateCameraNames(QStringList *list);

class QCVCamera : public QObject
{
    Q_OBJECT
public:
    explicit QCVCamera(QObject *parent = nullptr);
    ~QCVCamera();

    bool isOpened();
    bool wait();

signals:
    void newFrame(QMat mat);
    void captureFinished();
    void captureSkipped(QString reason);

    void cameraReady();
    void cameraClosed();

    void cameraOpenError();

    void resolutionChanged(int width, int height);
    void resolutionError();

public slots:
    void openCamera(int index);
    void closeCamera();

    void setResolution(int width, int height);

    void capture();

    void showProperties();

    void shutdown();

private slots:
    void started();
    void finished();

private:
    QThread captureThread;
    QMutex mutex;

    int frameWidth, frameHeight;

    cv::VideoCapture *cap;
    int cameraIndex;

    double fps;
    qint64 msecsPerFrame;
};

#endif // QCVCAMERA_H
