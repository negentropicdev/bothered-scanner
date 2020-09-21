#ifndef QMAT_H
#define QMAT_H

#include <QObject>
#include <QImage>

#include <opencv2/opencv.hpp>

class QMat
{
public:
    QMat();
    ~QMat();
    QMat(const QMat &);

    QMat(const cv::Mat &);

    cv::Mat *mat();

    QImage toImage() const;

private:
    void clone(const cv::Mat &);

    cv::Mat _mat;
};

Q_DECLARE_METATYPE(QMat);

#endif // QMAT_H
