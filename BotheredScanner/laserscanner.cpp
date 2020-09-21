#include "laserscanner.h"

#include <QDebug>
#include <cmath>

#define PI 3.14159265359
#define D2R(x) ((x) * PI / 180.0)

LaserScanner::LaserScanner(QObject *parent) : QObject(parent)
{
    blurSize = 3;
    threshold = 20;

    steps = 200;
    microsteps = 16;
    gearing = 8;

    dist = 13.6;
    hfov = 70.58;

    fastDelay = 400;
    slowDelay = 2000;

    curPosition = 0;

    hasBase = false;
    active = false;

    homePos = 0;
    leftPos = 0;
    rightPos = 0;

    laserPort = new QSerialPort();

    this->moveToThread(&scannerThread);
    scannerThread.start();

    connect(&positionTimer, &QTimer::timeout, this, &LaserScanner::updatePosition);
    positionTimer.start(100);
}

LaserScanner::~LaserScanner()
{
    positionTimer.stop();

    closeLaserPort();

    scannerThread.quit();
    scannerThread.wait();
}

void LaserScanner::setBlurSize(int size)
{
    blurSize = size * 2 - 1;
}

void LaserScanner::setThreshold(int thresh)
{
    threshold = thresh;
}

bool LaserScanner::isOpened()
{
    return laserPort != 0 && laserPort->isOpen();
}

void LaserScanner::updatePosition()
{
    if (isOpened()) {
        portMutex.lock();
        //laserPort->clear();
        laserPort->write("c", 1);
        //laserPort->flush();
        //thread()->msleep(1);

        char buf[10];
        laserPort->readLine(buf, 10);
        QString sVal(buf);
        curPosition = sVal.toInt();

        emit currentPosition(curPosition);

        if (steps > 0 && gearing > 0 && microsteps > 0) {
            double degPerStep = 360.0 / steps / gearing / microsteps;
            int stepsFromHome = curPosition - homePos;
            angle = 90 - stepsFromHome * degPerStep;

            emit currentAngle(angle);
        }
        portMutex.unlock();

        positionTimer.stop();
        positionTimer.start(100);
    }
}

void LaserScanner::write(QString s)
{
    if (isOpened()) {
        laserPort->write(s.toLocal8Bit());
    }
}

int LaserScanner::getBlurSize()
{
    return blurSize;
}

int LaserScanner::getThreshold()
{
    return threshold;
}

int LaserScanner::getHomePos()
{
    return homePos;
}

int LaserScanner::getLeftPos()
{
    return leftPos;
}

int LaserScanner::getRightPos()
{
    return rightPos;
}

int LaserScanner::getCurrentPos()
{
    return curPosition;
}

bool LaserScanner::wait()
{
    return scannerThread.wait();
}

void LaserScanner::setAngle(double a)
{
    angle = a;
}

void LaserScanner::setDist(double d)
{
    dist = d;
}

void LaserScanner::setViewAngle(double a)
{
    hfov = a;
}

void LaserScanner::setStepsPerRev(int s)
{
    steps = s;
}

void LaserScanner::setMicrosteps(int s)
{
    microsteps = s;
}

void LaserScanner::setGearing(double g)
{
    gearing = g;
}

void LaserScanner::newImage(QMat mat)
{
    if (active) {
        if (curPosition < rightPos) {
            stepRight();
        } else {
            active = false;
        }

        emit requestCapture();
    }

    cv::Mat chan;

    cv::cvtColor(*mat.mat(), chan, cv::COLOR_BGR2GRAY);

    emit channelImage(QMat(chan));

    if (chan.size != baseMat.size) {
        hasBase = false;
    }

    cv::Size size(blurSize, blurSize);
    cv::GaussianBlur(chan, chan, size, 0);

    emit blurredImage(QMat(chan));

    if (saveNextAsBase) {
        baseRaw = mat.mat()->clone();
        baseMat = chan.clone();
        saveNextAsBase = false;
        hasBase = true;
    }

    if (hasBase) {
        int nR = chan.rows;
        int nC = chan.cols;
        uchar *row, *base;

        for (int r = 0; r < nR; ++r) {
            row = chan.ptr(r);
            base = baseMat.ptr(r);

            for (int c = 0; c < nC; ++c) {
                int v = row[c] - base[c];

                if (v < 0) row[c] = 0;
                else       row[c] = v;
            }
        }
    } else {
        chan = cv::Mat(chan.rows, chan.cols, CV_8UC1);
    }

    emit diffedImage(QMat(chan));

    cv::threshold(chan, chan, threshold, 255, cv::THRESH_TOZERO);

    emit threshImage(QMat(chan));

    int nR = chan.rows;
    int nC = chan.cols;
    uchar max;
    int maxI;
    uchar *row;
    float_t *depthRow;

    for (int r = 0; r < nR; ++r) {
        row = chan.ptr(r);

        max = 0;
        maxI = -1;

        for (int c = 0; c < nC; ++c) {
            if (row[c] > max) {
                max = row[c];
                maxI = c;
            }
            row[c] = 0;
        }

        if (maxI >= 0) {
            row[maxI] = 255;

            if (hasBase) {
                if (active) {
                    depthRow = depthMap.ptr<float_t>(r);

                    double c_2 = nC / 2; // center pixel index
                    double dA = hfov / nC; // angle per pixel
                    double theta = (maxI - c_2) * dA;
                    double aR = angle * 3.14159265359 / 180.0;
                    double tR = theta * 3.14159265359 / 180.0;

                    double sa = sin(aR);
                    //double cat = cos(aR - tR);
                    double sat = sin((PI / 2) - aR - tR);

                    double x = dist * sa / sat;

                    double H = x * cos(tR);

                    depthRow[maxI] = H;
                }
            }
        }
    }

    emit scanImage(QMat(chan));

    emit depthImage(QMat(depthMap));
}

void LaserScanner::resolutionChanged(int width, int height)
{
    baseMat = cv::Mat::zeros(height, width, CV_8UC1);
    depthMap = cv::Mat::zeros(height, width, CV_32FC1);
}

void LaserScanner::saveBase()
{
    saveNextAsBase = true;
}

void LaserScanner::clearDepthMap()
{
    depthMap = cv::Mat::zeros(baseMat.rows, baseMat.cols, CV_32FC1);
}

void LaserScanner::startScan()
{
    depthMap = cv::Mat::zeros(baseMat.rows, baseMat.cols, CV_32FC1);
    active = true;
}

void LaserScanner::stopScan()
{
    active = false;
}

void LaserScanner::openLaserPort(QString portName)
{
    laserPort->setPortName(portName);
    laserPort->setBaudRate(19200);
    laserPort->setParity(QSerialPort::NoParity);
    laserPort->setDataBits(QSerialPort::Data8);
    laserPort->setStopBits(QSerialPort::OneStop);
    laserPort->setFlowControl(QSerialPort::NoFlowControl);

    if (laserPort->open(QIODevice::ReadWrite)) {
        updatePosition();

        emit laserReady();
    } else {
        emit laserOpenError("Failed to open laser.");
    }
}

void LaserScanner::closeLaserPort()
{
    if (isOpened()) {
        portMutex.lock();
        laserPort->close();
        portMutex.unlock();
    }
}

void LaserScanner::stepLeft()
{
    portMutex.lock();
    write("<");
    portMutex.unlock();
    //updatePosition();
}

void LaserScanner::stepRight()
{
    portMutex.lock();
    write(">");
    portMutex.unlock();
    //updatePosition();
}

void LaserScanner::setLeftExtent()
{
    leftPos = curPosition;
}

void LaserScanner::setRightExtent()
{
    rightPos = curPosition;
}

void LaserScanner::setHome()
{
    homePos = curPosition;
    //updatePosition();
}

void LaserScanner::goLeftExtent()
{
    gotoPos(leftPos);
}

void LaserScanner::goRightExtent()
{
    gotoPos(rightPos);
}

void LaserScanner::goHome()
{
    gotoPos(homePos);
}

void LaserScanner::gotoPos(int pos)
{
    QString s = "G";
    s.append(QString::number(pos));
    write(s);
}

void LaserScanner::setFastSpeed(int delay)
{
    fastDelay = delay;
}

void LaserScanner::setSlowSpeed(int delay)
{
    slowDelay = delay;
}

void LaserScanner::setDelay(int delay)
{
    QString s = "p";
    s.append(QString::number(delay));
    write(s);
}

void LaserScanner::jogLeft()
{
    write("-");
}

void LaserScanner::jogRight()
{
    write("+");
}

void LaserScanner::stop()
{
    write("s");
}

void LaserScanner::laserOn()
{
    write("L");
}

void LaserScanner::laserOff()
{
    write("l");
}