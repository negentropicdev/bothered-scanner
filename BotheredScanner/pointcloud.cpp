#include "pointcloud.h"

#include <QMapIterator>
#include <QDebug>

PointCloud::PointCloud() {
}

void PointCloud::reset(int width, int height) {
    cloud.clear();
    rows = height;
    cols = width;
}

void PointCloud::addPoint(int r, int c, const QVector3D &pos, const QColor &col) {
    CloudPoint pt;
    pt.pos = pos;
    pt.col = col;

    PointKeyUnion k;
    k.key.r = r;
    k.key.c = c;

    cloud.insert(k.val, pt);
}

QString PointCloud::toPLY() {
    QString ply;

    ply.append("ply\n");

    ply.append("format ascii 1.0\n");

    ply.append("element vertex ");
    ply.append(QString::number(cloud.count()));
    ply.append("\n");

    ply.append("property float x\n");
    ply.append("property float y\n");
    ply.append("property float z\n");
    ply.append("property uchar diffuse_red\n");
    ply.append("property uchar diffuse_green\n");
    ply.append("property uchar diffuse_blue\n");
    ply.append("element face 0\n");
    ply.append("property list uchar int vertex_indices\n");
    ply.append("end_header\n");

    PointKeyUnion k;
    for (int r = 0; r < rows; ++r) {
        k.key.r = r;
        for (int c = 0; c < cols; ++c) {
            k.key.c = c;
            QList<CloudPoint> values = cloud.values(k.val);
            for (int i = 0; i < values.size(); ++i) {
                CloudPoint pt = values[i];
                ply.append(QString::number(pt.pos.x()));
                ply.append(" ");
                ply.append(QString::number(pt.pos.y()));
                ply.append(" ");
                ply.append(QString::number(pt.pos.z()));
                ply.append(" ");
                ply.append(QString::number(pt.col.red()));
                ply.append(" ");
                ply.append(QString::number(pt.col.green()));
                ply.append(" ");
                ply.append(QString::number(pt.col.blue()));
                ply.append("\n");
            }
        }
    }

    return ply;
}
