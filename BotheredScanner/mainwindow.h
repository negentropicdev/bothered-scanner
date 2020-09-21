#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "opencv2/opencv.hpp"

#include "qcvcamera.h"
#include "digitalslider.h"
#include "laserscanner.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    enum ViewMode {
        Main,
        Chan,
        Blur,
        Diff,
        Thresh,
        Scan,
        Depth
    };

signals:
    void requestCapture();
    void openCamera(int index);
    void setResolution(int width, int height);
    void showCameraProperties();

    void openLaser(QString portName);
    void closeLaser();
    void setLaserStepDelay(int delay);
    void setLaserHome();
    void setLaserLeft();
    void setLaserRight();

    void setStepsPerRev(int s);
    void setGearing(double g);
    void setMicrosteps(int m);

    void setDistance(double d);

    void setBaseImage();
    void setBlur(int r);
    void setThreshold(int t);

    void startScan();
    void stopScan();

private slots:
    void on_selectCamera_currentIndexChanged(int index);
    void on_selectResolution_currentIndexChanged(const QString &arg1);

    void on_selectLaserPort_currentIndexChanged(const QString &arg1);

    void originalFrame(QMat mat);
    void channelFrame(QMat mat);
    void blurredFrame(QMat mat);
    void diffedFrame(QMat mat);
    void threshFrame(QMat mat);
    void scanFrame(QMat mat);
    void depthFrame(QMat mat);

    void cameraReady();
    void cameraOpenFailed();

    void resolutionChanged(int width, int height);
    void resolutionError();

    void captureFinished();
    void captureSkipped(QString reason);

    void laserOpened();
    void laserOpenError(QString reason);
    void laserPosition(int pos);
    void laserAngle(double angle);

    void on_btnMain_clicked();
    void on_btnBlur_clicked();
    void on_btnDiff_clicked();
    void on_btnThresh_clicked();
    void on_btnGray_clicked();
    void on_btnScan_clicked();
    void on_btnDepth_clicked();

    void on_btnProps_clicked();

    void on_btnSetHome_clicked();

    void on_btnSetLeft_clicked();

    void on_btnSetRight_clicked();

    void on_numJogPeriod_valueChanged(int arg1);

    void on_numStepsPerRev_valueChanged(double arg1);

    void on_numMicrosteps_valueChanged(double arg1);

    void on_numGearing_valueChanged(double arg1);

    void on_numDistance_valueChanged(double arg1);

    void on_btnStartScan_toggled(bool checked);

private:
    void setMainImage(const QMat &mat);

    Ui::MainWindow *ui;

    QGraphicsPixmapItem pixMain;

    QCVCamera cam;
    LaserScanner scanner;

    int curCamIndex;
    int maxResIndex;

    ViewMode view;

    DigitalSlider *blur;
    DigitalSlider *thresh;

    bool scan;
};
#endif // MAINWINDOW_H
