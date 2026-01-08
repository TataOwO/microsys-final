#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "string"
#include "gpio_LED_CTRL.hpp"
#include "PIN_LOOKUP.hpp"
#include <stdio.h>
#include <memory>
#include <unistd.h>
#include <QKeyEvent>
#include <stdlib.h>
#include <ctime>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_led_shine_clicked();


    void on_score_num_linkActivated(const QString &link);

private:
    Ui::MainWindow *ui;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void light();
    int bolt_num = 0;
    int score = 0;
    int count = 0;
    bool is_Pressed = false;

private:
    std::shared_ptr<gpio::LED_CTRL> LED1 = std::make_shared<gpio::LED_CTRL>(LOOKUP::PIN::P7 );
    std::shared_ptr<gpio::LED_CTRL> LED2 = std::make_shared<gpio::LED_CTRL>(LOOKUP::PIN::P13);
    std::shared_ptr<gpio::LED_CTRL> LED3 = std::make_shared<gpio::LED_CTRL>(LOOKUP::PIN::P15);
    std::shared_ptr<gpio::LED_CTRL> LED4 = std::make_shared<gpio::LED_CTRL>(LOOKUP::PIN::P32);


private:
    void switch_status_1();
    void switch_status_2();
    void blink(int count);
};

#endif // MAINWINDOW_H

