#ifndef DIGITALSLIDER_H
#define DIGITALSLIDER_H

#include <QWidget>

namespace Ui {
class DigitalSlider;
}

class DigitalSlider : public QWidget
{
    Q_OBJECT

public:
    explicit DigitalSlider(QWidget *parent = nullptr);
    ~DigitalSlider();

    void setLabel(QString value);
    void setRange(int min, int max);

signals:
    void valueChanged(int v);

public slots:
    void setValue(int v);

private slots:
    void uiSetValue(int v);

private:
    Ui::DigitalSlider *ui;
};

#endif // DIGITALSLIDER_H
