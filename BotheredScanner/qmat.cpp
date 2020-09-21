#include "qmat.h"

QMat::QMat()
{
}

QMat::~QMat()
{
}

QMat::QMat(const QMat &r)
{
    clone(r._mat);
}

QMat::QMat(const cv::Mat &mat)
{
    clone(mat);
}

cv::Mat *QMat::mat()
{
    return &_mat;
}

QImage QMat::toImage() const
{
    if (_mat.type() == CV_8UC1) {
        return QImage(_mat.data, _mat.cols, _mat.rows, _mat.cols, QImage::Format_Grayscale8);
    }

    return QImage(_mat.data, _mat.cols, _mat.rows, _mat.cols * 3, QImage::Format_BGR888);
}

void QMat::clone(const cv::Mat &other)
{
    _mat = other.clone();
}
