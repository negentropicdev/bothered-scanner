#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>

#include <QTimer>
#include <QGraphicsPixmapItem>

#include <cmath>

#include "opencv2/opencv.hpp"

#include "digitalslider.h"

#define SLIDER(slider, slot) connect(slider, &DigitalSlider::valueChanged, this, &MainWindow::slot)

QStringList defaultRes {"1920x1080", "1280x720", "640x480", "320x240"};



DigitalSlider *createDS(MainWindow *window, QString label, int min, int max, QLayout *layout) {
    DigitalSlider *s = new DigitalSlider(window);
    s->setLabel(label);
    s->setRange(min, max);
    layout->addWidget(s);

    return s;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QLayout *sl = ui->layoutScanProps;

    blur = createDS(this, "Blur", 1, 7, sl);
    //SLIDER(blur, setBlur);
    //blur->setValue((scanner.getBlurSize() + 1) / 2);

    thresh = createDS(this, "Threshold", 1, 254, sl);
    //SLIDER(thresh, setThreshold);
    //thresh->setValue(scanner.getThreshold());

    connect(blur,   &DigitalSlider::valueChanged, &scanner, &LaserScanner::setBlurSize);
    connect(thresh, &DigitalSlider::valueChanged, &scanner, &LaserScanner::setThreshold);

    ui->statusbar->showMessage("Idle");

    ui->selectCamera->clear();

    QStringList camNames;
    enumerateCameraNames(&camNames);
    ui->selectCamera->addItems(camNames);

    ui->selectResolution->clear();
    ui->selectResolution->addItems(defaultRes);

    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    QStringList portNames;

    for (int i = 0; i < ports.count(); ++i) {
        portNames.append(ports[i].portName() + ": " + ports[i].description());
    }
    ui->selectLaserPort->clear();
    ui->selectLaserPort->addItems(portNames);

    ui->viewMain->setScene(new QGraphicsScene(this));
    ui->viewMain->scene()->addItem(&pixMain);

    view = Main;
    scan = false;

    //camera to UI
    connect(&cam,               &QCVCamera::cameraReady,            this,       &MainWindow::cameraReady);
    connect(&cam,               &QCVCamera::cameraOpenError,        this,       &MainWindow::cameraOpenFailed);
    connect(&cam,               &QCVCamera::newFrame,               this,       &MainWindow::originalFrame);
    connect(&cam,               &QCVCamera::captureFinished,        this,       &MainWindow::captureFinished);
    connect(&cam,               &QCVCamera::resolutionChanged,      this,       &MainWindow::resolutionChanged);

    //camera to laser
    connect(&cam,               &QCVCamera::newFrame,               &scanner,   &LaserScanner::newImage);
    connect(&cam,               &QCVCamera::resolutionChanged,      &scanner,   &LaserScanner::resolutionChanged);

    //laser to camera
    connect(&scanner,           &LaserScanner::requestCapture,      &cam,       &QCVCamera::capture);

    //UI to camera
    connect(this,               &MainWindow::requestCapture,        &cam,       &QCVCamera::capture);
    connect(this,               &MainWindow::openCamera,            &cam,       &QCVCamera::openCamera);
    connect(this,               &MainWindow::setResolution,         &cam,       &QCVCamera::setResolution);
    connect(this,               &MainWindow::showCameraProperties,  &cam,       &QCVCamera::showProperties);

    //UI to laser
    connect(this,               &MainWindow::openLaser,             &scanner,   &LaserScanner::openLaserPort);
    connect(this,               &MainWindow::setLaserStepDelay,     &scanner,   &LaserScanner::setDelay);
    connect(this,               &MainWindow::setLaserHome,          &scanner,   &LaserScanner::setHome);
    connect(this,               &MainWindow::setLaserLeft,          &scanner,   &LaserScanner::setLeftExtent);
    connect(this,               &MainWindow::setLaserRight,         &scanner,   &LaserScanner::setRightExtent);
    connect(this,               &MainWindow::setStepsPerRev,        &scanner,   &LaserScanner::setStepsPerRev);
    connect(this,               &MainWindow::setMicrosteps,         &scanner,   &LaserScanner::setMicrosteps);
    connect(this,               &MainWindow::setGearing,            &scanner,   &LaserScanner::setGearing);
    connect(this,               &MainWindow::setDistance,           &scanner,   &LaserScanner::setDist);
    connect(this,               &MainWindow::setBaseImage,          &scanner,   &LaserScanner::saveBase);
    connect(this,               &MainWindow::startScan,             &scanner,   &LaserScanner::startScan);
    connect(this,               &MainWindow::stopScan,              &scanner,   &LaserScanner::stopScan);
    connect(this,               &MainWindow::laserOff,              &scanner,   &LaserScanner::laserOff);

    connect(ui->btnLaserOff,    &QPushButton::clicked,              &scanner,   &LaserScanner::laserOff);
    connect(ui->btnLaserOn,     &QPushButton::clicked,              &scanner,   &LaserScanner::laserOn);
    connect(ui->btnFastLeft,    &QPushButton::pressed,              &scanner,   &LaserScanner::jogLeft);
    connect(ui->btnFastLeft,    &QPushButton::released,             &scanner,   &LaserScanner::stop);
    connect(ui->btnFastRight,   &QPushButton::pressed,              &scanner,   &LaserScanner::jogRight);
    connect(ui->btnFastRight,   &QPushButton::released,             &scanner,   &LaserScanner::stop);
    connect(ui->btnStepLeft,    &QPushButton::clicked,              &scanner,   &LaserScanner::stepLeft);
    connect(ui->btnStepRight,   &QPushButton::clicked,              &scanner,   &LaserScanner::stepRight);
    connect(ui->btnGoHome,      &QPushButton::clicked,              &scanner,   &LaserScanner::goHome);
    connect(ui->btnGoLeft,      &QPushButton::clicked,              &scanner,   &LaserScanner::goLeftExtent);
    connect(ui->btnGoRight,     &QPushButton::clicked,              &scanner,   &LaserScanner::goRightExtent);
    connect(ui->btnBase,        &QPushButton::clicked,              &scanner,   &LaserScanner::saveBase);

    //laser to UI
    connect(&scanner,           &LaserScanner::laserReady,          this,       &MainWindow::laserOpened);
    connect(&scanner,           &LaserScanner::laserOpenError,      this,       &MainWindow::laserOpenError);
    connect(&scanner,           &LaserScanner::currentPosition,     this,       &MainWindow::laserPosition);
    connect(&scanner,           &LaserScanner::currentAngle,        this,       &MainWindow::laserAngle);
    connect(&scanner,           &LaserScanner::channelImage,        this,       &MainWindow::channelFrame);
    connect(&scanner,           &LaserScanner::blurredImage,        this,       &MainWindow::blurredFrame);
    connect(&scanner,           &LaserScanner::diffedImage,         this,       &MainWindow::diffedFrame);
    connect(&scanner,           &LaserScanner::threshImage,         this,       &MainWindow::threshFrame);
    connect(&scanner,           &LaserScanner::scanImage,           this,       &MainWindow::scanFrame);
    connect(&scanner,           &LaserScanner::depthImage,          this,       &MainWindow::depthFrame);
    connect(&scanner,           &LaserScanner::scanComplete,        this,       &MainWindow::scanComplete);
}

MainWindow::~MainWindow()
{
    cam.shutdown();
    scanner.shutdown();

    delete ui;
}


void MainWindow::on_selectCamera_currentIndexChanged(int index)
{
    if (index < 0) {
        return;
    }

    curCamIndex = index;
    emit openCamera(index);
}

void MainWindow::setMainImage(const QMat &mat)
{
    QImage img = mat.toImage();
    pixMain.setPixmap(QPixmap::fromImage(img));
    ui->viewMain->fitInView(&pixMain, Qt::KeepAspectRatio);
}

void MainWindow::originalFrame(QMat mat)
{
    if (view == Main) {
        setMainImage(mat);
    }
}

void MainWindow::channelFrame(QMat mat)
{
    if (view == Chan) {
        setMainImage(mat);
    }
}

void MainWindow::blurredFrame(QMat mat)
{
    if (view == Blur) {
        setMainImage(mat);
    }
}

void MainWindow::diffedFrame(QMat mat) {
    if (view == Diff) {
        setMainImage(mat);
    }
}

void MainWindow::threshFrame(QMat mat)
{
    if (view == Thresh) {
        setMainImage(mat);
    }
}

void MainWindow::scanFrame(QMat mat)
{
    if (view == Scan) {
        setMainImage(mat);
    }
}

void MainWindow::depthFrame(QMat mat)
{
    if (view == Depth) {
        cv::Mat *depth = mat.mat();

        int nR = depth->rows;
        int nC = depth->cols;

        cv::Mat scaled = cv::Mat::zeros(depth->rows, depth->cols, CV_8UC1);

        float *row;
        uchar *sRow;

        float min = 1000000, max = 0;

        for (int r = 0; r < nR; ++r) {
            row = depth->ptr<float>(r);
            for (int c = 0; c < nC; ++c) {
                float v = row[c];
                if (v > 0 && v < min) min = v;
                if (v > max) max = v;
            }
        }

        float range = max - min;

        for (int r = 0; r < nR; ++r) {
            row = depth->ptr<float>(r);
            sRow = scaled.ptr(r);

            for (int c = 0; c < nC; ++c) {
                float v = row[c];

                v = 1.0 - ((v - min) / range);

                v *= 255.0;
                if (v > 255) v = 255;
                else if (v < 0) v = 0;

                sRow[c] = v;
            }
        }

        setMainImage(QMat(scaled));
    }
}

void MainWindow::cameraReady()
{
    //start freerunning preview
    emit requestCapture();
}

void MainWindow::cameraOpenFailed()
{
    ui->selectCamera->setCurrentIndex(-1);
    ui->selectCamera->setCurrentText("Select Camera...");

    QMessageBox::critical(this, "Bothered Scanner", "Unable to open selected camera.");
}

void MainWindow::resolutionChanged(int width, int height)
{
    QString resString = "";

    resString.append(QString::number(width));
    resString.append("x");
    resString.append(QString::number(height));

    ui->selectResolution->setCurrentText(resString);
}

void MainWindow::resolutionError()
{
    QMessageBox::critical(this, "Bothered Scanner", "Unable to set resolution.");
}

void MainWindow::captureFinished()
{
    //only freerun the preview if not scanning a depth map
    if (!scan) {
        emit requestCapture();
    }
}

void MainWindow::captureSkipped(QString reason)
{
    ui->statusbar->showMessage(QString("Frame Skipped: ").append(reason));
}

void MainWindow::laserOpened()
{
    ui->statusbar->showMessage("Laser Opened");
}

void MainWindow::laserOpenError(QString reason)
{
    QMessageBox::critical(this, "Bothered Scanner", reason);
}

void MainWindow::laserPosition(int pos)
{
    ui->lblPosition->setText(QString::number(pos));
}

void MainWindow::laserAngle(double angle)
{
    ui->lblAngle->setText(QString::number(angle));
}

void MainWindow::scanComplete()
{
    scan = false;
    ui->btnStartScan->setChecked(false);
    ui->btnLaserOff->setChecked(true);
    ui->btnLaserOn->setChecked(false);

    emit laserOff();
    emit requestCapture();
}

void MainWindow::on_selectResolution_currentIndexChanged(const QString &arg1)
{
    if (arg1.isEmpty()) {
        return;
    }

    QStringList s = arg1.split("x");

    if (s.count() < 2) {
        return;
    }

    int newWidth = s[0].toInt(), newHeight = s[1].toInt();

    emit setResolution(newWidth, newHeight);
}

void MainWindow::on_btnMain_clicked()
{
    view = ViewMode::Main;
}

void MainWindow::on_btnGray_clicked()
{
    view = ViewMode::Chan;
}

void MainWindow::on_btnBlur_clicked()
{
    view = ViewMode::Blur;
}

void MainWindow::on_btnDiff_clicked()
{
    view = ViewMode::Diff;
}

void MainWindow::on_btnThresh_clicked()
{
    view = ViewMode::Thresh;
}

void MainWindow::on_btnProps_clicked()
{
    emit showCameraProperties();
}

void MainWindow::on_btnScan_clicked()
{
    view = ViewMode::Scan;
}

void MainWindow::on_btnDepth_clicked()
{
    view = Depth;
}

void MainWindow::on_selectLaserPort_currentIndexChanged(const QString &port)
{
    QStringList p = port.split(":");
    emit openLaser(p[0]);
}

void MainWindow::on_numJogPeriod_valueChanged(int delay)
{
    emit setLaserStepDelay(delay);
}

void MainWindow::on_btnSetHome_clicked()
{
    emit setLaserHome();
    ui->lblHome->setText(QString::number(scanner.getCurrentPos()));
}

void MainWindow::on_btnSetLeft_clicked()
{
    emit setLaserLeft();
    ui->lblLeft->setText(QString::number(scanner.getCurrentPos()));
}

void MainWindow::on_btnSetRight_clicked()
{
    emit setLaserRight();
    ui->lblRight->setText(QString::number(scanner.getCurrentPos()));
}

void MainWindow::on_numStepsPerRev_valueChanged(double arg1)
{
    emit setStepsPerRev(arg1);
}

void MainWindow::on_numMicrosteps_valueChanged(double arg1)
{
    emit setMicrosteps(arg1);
}

void MainWindow::on_numGearing_valueChanged(double arg1)
{
    emit setGearing(arg1);
}

void MainWindow::on_numDistance_valueChanged(double arg1)
{
    emit setDistance(arg1);
}

void MainWindow::on_btnStartScan_toggled(bool checked)
{
    scan = checked;

    if (checked) {
        emit startScan();
    } else {
        emit stopScan();
    }
}
