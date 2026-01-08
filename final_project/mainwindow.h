#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QTimer>
#include <QProcess>
#include <memory>
#include "gpio_LED_CTRL.hpp"
#include "PIN_LOOKUP.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    // Keyboard event handlers
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    // Button click handlers
    void on_btnStart_clicked();
    void on_btnStop_clicked();
    void on_btnBlinkAll_clicked();
    void on_btnAllOn_clicked();
    void on_btnAllOff_clicked();
    void on_btnSetThreshold_clicked();
    void on_btnToggleMode_clicked();

    // Timer slots
    void onADCTimerTimeout();
    void onBlinkTimerTimeout();

private:
    Ui::MainWindow *ui;

    // LED controllers (4 LEDs for Item 2)
    std::shared_ptr<gpio::LED_CTRL> LED1 = std::make_shared<gpio::LED_CTRL>(LOOKUP::PIN::P7);
    std::shared_ptr<gpio::LED_CTRL> LED2 = std::make_shared<gpio::LED_CTRL>(LOOKUP::PIN::P13);
    std::shared_ptr<gpio::LED_CTRL> LED3 = std::make_shared<gpio::LED_CTRL>(LOOKUP::PIN::P15);
    std::shared_ptr<gpio::LED_CTRL> LED4 = std::make_shared<gpio::LED_CTRL>(LOOKUP::PIN::P32);

    // LED states
    bool led1State = false;
    bool led2State = false;
    bool led3State = false;
    bool led4State = false;

    // ADC related (Item 3)
    QTimer *adcTimer;
    int currentADC = 0;
    int threshold = 500;
    QString adcScriptPath;

    // System state
    bool systemRunning = false;
    bool autoMode = false;

    // Blink related
    QTimer *blinkTimer;
    bool blinkState = false;
    bool isBlinking = false;

    // Helper functions
    void initializeUI();
    void initializeTimers();
    void readADC();
    void onADCValueReceived(int value);
    void autoControlLEDs();
    void updateLEDDisplay();
    void updateStatusDisplay();

    // LED control functions
    void toggleLED(int ledNum);
    void setLED(int ledNum, bool state);
    void setAllLEDs(bool state);
    void blinkAllLEDs();
    void stopBlinking();

    // Shortcut info
    void showShortcutHelp();
};

#endif // MAINWINDOW_H
