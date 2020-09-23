#ifndef POINTCLOUD_H
#define POINTCLOUD_H

#include <QMultiMap>
#include <QVector3D>
#include <QColor>

struct CloudPoint {
    QVector3D pos;
    QColor col;
};

struct PointKey {
    qint16 r;
    qint16 c;
};

union PointKeyUnion {
    PointKey key;
    quint32 val;
};

class PointCloud;

class PointCloud
{
public:
    PointCloud();

    void reset(int width, int height);

    void addPoint(int r, int c, const QVector3D &pos, const QColor &col);

    QString toPLY();

private:

    QMultiMap<quint32, CloudPoint> cloud;
    int rows;
    int cols;
};

#endif // POINTCLOUD_H
