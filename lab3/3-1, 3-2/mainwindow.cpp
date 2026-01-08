#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "string"
#include "string.h"
#include "QTimer"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_led_shine_clicked()
{
    if (ui->led1check->checkState()) {
        LED1->enable();
        ui->ledpicture1->show();
    }
    else {
        LED1->disable();
        ui->ledpicture1->hide();
    }

    if (ui->led2check->checkState()) {
        LED2->enable();
        ui->ledpicture2->show();
    }
    else {
        LED2->disable();
        ui->ledpicture2->hide();
    }

    if (ui->led3check->checkState()) {
        LED3->enable();
        ui->ledpicture3->show();
    }
    else {
        LED3->disable();
        ui->ledpicture3->hide();
    }

    if (ui->led4check->checkState()) {
        LED4->enable();
        ui->ledpicture4->show();
    }
    else {
        LED4->disable();
        ui->ledpicture4->hide();
    }
}

void MainWindow::on_led_blink_clicked() {
    std::string label_text = ui->blink_text->toPlainText().toStdString();
    int count = std::stoi(label_text);

    blink(count);
}

void MainWindow::blink(int count) {
    for (int i=0; i<count; ++i) {
        QTimer::singleShot(i*ui->slider->value()*50, [this]() {this->switch_status_1();});
        QTimer::singleShot(i*ui->slider->value()*50 + ui->slider->value()*25, [this]() {this->switch_status_2();});
    }
    QTimer::singleShot(count*ui->slider->value()*50, [this]() {
        LED1->disable();
        LED2->disable();
        ui->ledpicture1->hide();
        ui->ledpicture2->hide();
    });
}

void MainWindow::switch_status_1() {
    LED1->disable();
    LED2->disable();
    LED3->enable();
    LED4->enable();
    ui->ledpicture1->hide();
    ui->ledpicture2->hide();
    ui->ledpicture3->show();
    ui->ledpicture4->show();
}

void MainWindow::switch_status_2() {
    LED1->enable();
    LED2->enable();
    LED3->disable();
    LED4->disable();
    ui->ledpicture1->show();
    ui->ledpicture2->show();
    ui->ledpicture3->hide();
    ui->ledpicture4->hide();
}

void MainWindow::on_slider_sliderMoved(int position)
{
    ui->progressBar->setValue(position);
    ui->spinBox->setValue(position);
}

void MainWindow::on_spinBox_editingFinished()
{
    ui->slider->setValue(ui->spinBox->value());
    ui->progressBar->setValue(ui->spinBox->value());
}
