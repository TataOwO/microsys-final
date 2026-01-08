#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "string"
#include "string.h"
#include "QTimer"
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    std::srand(time(NULL));

    setFocusPolicy(Qt::StrongFocus);

    LED1->disable();
    LED2->disable();
    LED3->disable();
    LED4->disable();
    ui->ledpicture1->hide();
    ui->ledpicture2->hide();
    ui->ledpicture3->hide();
    ui->ledpicture4->hide();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::light() {
    int rd = rand() % 4 + 1;

    switch (rd) {
    case 1:
        LED1->enable();
        ui->ledpicture1->show();
        break;
    case 2:
        LED2->enable();
        ui->ledpicture2->show();
        break;
    case 3:
        LED3->enable();
        ui->ledpicture3->show();
        break;
    case 4:
        LED4->enable();
        ui->ledpicture4->show();
        break;
    default:
        LED2->enable();
        ui->ledpicture2->show();
        LED4->enable();
        ui->ledpicture4->show();
    }

    bolt_num = rd;
    count++;

    std::cout << rd << std::endl;
}

void MainWindow::on_led_shine_clicked()
{
    score = 0;
    light();

    /*if (ui->led1check->checkState()) {

    }
    else {
        LED1->disable();
        ui->ledpicture1->hide();
    }

    if (ui->led2check->checkState()) {

    }
    else {
        LED2->disable();
        ui->ledpicture2->hide();
    }

    if (ui->led3check->checkState()) {

    }
    else {
        LED3->disable();
        ui->ledpicture3->hide();
    }

    if (ui->led4check->checkState()) {

    }
    else {
        LED4->disable();
        ui->ledpicture4->hide();
    }*/
}

/*void MainWindow::on_led_blink_clicked() {
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
}*/

void MainWindow::keyPressEvent(QKeyEvent *event){
    if(event->key() == Qt::Key_1 && bolt_num == 1)
        score += 10;
    else if(event->key() == Qt::Key_2 && bolt_num == 2)
        score += 10;
    else if(event->key() == Qt::Key_3 && bolt_num == 3)
        score += 10;
    else if(event->key() == Qt::Key_4 && bolt_num == 4)
        score += 10;

    switch (bolt_num) {
    case 1:
        LED1->disable();
        ui->ledpicture1->hide();
        break;
    case 2:
        LED2->disable();
        ui->ledpicture2->hide();
        break;
    case 3:
        LED3->disable();
        ui->ledpicture3->hide();
        break;
    case 4:
        LED4->disable();
        ui->ledpicture4->hide();
        break;
    default:
        break;
    }

    auto a = QString::fromStdString(std::to_string(score));
    ui->score_num->setText(a);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event){
    if(count < 10){
        light();
    }else{
        count = 0;
    }
}

void MainWindow::on_score_num_linkActivated(const QString &link)
{

}
