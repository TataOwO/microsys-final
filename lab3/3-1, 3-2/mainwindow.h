#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "string"
#include "gpio_LED_CTRL.hpp"
#include "PIN_LOOKUP.hpp"
#include <stdio.h>
#include <memory>
#include <unistd.h>

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
    void on_led_blink_clicked();

    void on_slider_sliderMoved(int position);

    void on_spinBox_editingFinished();

private:
    Ui::MainWindow *ui;

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

