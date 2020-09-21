#include "digitalslider.h"
#include "ui_digitalslider.h"

DigitalSlider::DigitalSlider(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DigitalSlider)
{
    ui->setupUi(this);
}

DigitalSlider::~DigitalSlider()
{
    delete ui;
}

void DigitalSlider::setLabel(QString value)
{
    ui->label->setText(value);
}

void DigitalSlider::setRange(int min, int max)
{
    ui->slider->setMinimum(min);
    ui->slider->setMaximum(max);

    ui->numeric->setMinimum(min);
    ui->numeric->setMaximum(max);

    setValue(ui->slider->value());
}

void DigitalSlider::setValue(int v)
{
    /*if (v == 0) {
        setEnabled(false);
        return;
    }*/

    setEnabled(true);

    if (v < ui->slider->minimum()) {
        setRange(v, ui->slider->maximum());
    } else if (v > ui->slider->maximum()) {
        setRange(ui->slider->minimum(), v);
    }

    ui->slider->setValue(v);
    ui->numeric->setValue(v);
}

void DigitalSlider::uiSetValue(int v)
{
    setValue(v);
    emit valueChanged(v);
}
