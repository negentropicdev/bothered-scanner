#ifndef LASERSCANNER_H
#define LASERSCANNER_H

#include <QThread>
#include <QSerialPort>
#include <QTimer>
#include <QMutex>

#include "qmat.h"

typedef QVector<int> Scan;

class LaserScanner : public QObject
{
    Q_OBJECT
public:
    explicit LaserScanner(QObject *parent = nullptr);
    ~LaserScanner();

    int getBlurSize();
    int getThreshold();

    int getHomePos();
    int getLeftPos();
    int getRightPos();

    int getCurrentPos();

    bool wait();

public slots:
    void newImage(QMat mat);
    void resolutionChanged(int width, int height);

    void saveBase();

    void clearDepthMap();

    void startScan();
    void stopScan();

    void openLaserPort(QString portName);
    void closeLaserPort();

    void stepLeft();
    void stepRight();

    void setLeftExtent();
    void setRightExtent();
    void setHome();

    void goLeftExtent();
    void goRightExtent();
    void goHome();
    void gotoPos(int pos);

    void setFastSpeed(int delay);
    void setSlowSpeed(int delay);
    void setDelay(int delay);

    void jogLeft();
    void jogRight();
    void stop();

    void laserOn();
    void laserOff();

    void setAngle(double a);
    void setDist(double d);
    void setViewAngle(double a);

    void setStepsPerRev(int s);
    void setMicrosteps(int s);
    void setGearing(double g);

    void setBlurSize(int size);
    void setThreshold(int thresh);

    void updatePosition();

signals:
    void channelImage(QMat mat);
    void blurredImage(QMat mat);
    void diffedImage(QMat mat);
    void threshImage(QMat mat);

    void scanImage(QMat mat);
    void depthImage(QMat mat);

    void requestCapture();

    void laserReady();
    void laserOpenError(QString message);

    void currentPosition(int pos);
    void currentAngle(double angle);

private:
    bool isOpened();
    void timerElapsed();

    void write(QString s);

    QThread scannerThread;

    QSerialPort *laserPort;

    QTimer positionTimer;
    QMutex portMutex;

    int blurSize;
    int threshold;

    bool saveNextAsBase;
    bool hasBase;

    bool active;

    cv::Mat baseMat;
    cv::Mat baseRaw;
    cv::Mat depthMap;

    int fastDelay;
    int slowDelay;

    int leftPos;
    int rightPos;
    int homePos;

    int curPosition;

    double angle;
    double dist;
    double hfov;

    int steps;
    int microsteps;
    double gearing;
};

#endif // LASERSCANNER_H
