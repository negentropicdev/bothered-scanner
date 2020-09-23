QT       += core gui multimedia serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

INCLUDEPATH += C:/dev/opencv/build-4.4.0/install/include

CONFIG( debug, debug|release ) {
LIBS += -LC:/dev/opencv/build-4.4.0/install/x64/vc16/lib/ \
    -lopencv_core440d\
    -lopencv_imgproc440d\
    -lopencv_highgui440d\
    -lopencv_imgcodecs440d\
    -lopencv_videoio440d\
    -lopencv_video440d\
    -lopencv_calib3d440d\
    -lopencv_photo440d\
    -lopencv_features2d440d
}
else {
LIBS += -LC:/dev/opencv/build-4.4.0/install/x64/vc16/lib/ \
    -lopencv_core440\
    -lopencv_imgproc440\
    -lopencv_highgui440\
    -lopencv_imgcodecs440\
    -lopencv_videoio440\
    -lopencv_video440\
    -lopencv_calib3d440\
    -lopencv_photo440\
    -lopencv_features2d440
}

DEPENDPATH += C:/dev/opencv/build-4.4.0/install/include

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    digitalslider.cpp \
    laserscanner.cpp \
    main.cpp \
    mainwindow.cpp \
    pointcloud.cpp \
    qcvcamera.cpp \
    qmat.cpp

HEADERS += \
    digitalslider.h \
    laserscanner.h \
    mainwindow.h \
    pointcloud.h \
    qcvcamera.h \
    qmat.h

FORMS += \
    digitalslider.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES +=
