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

// System states for the Machine Operation Control System
enum class SystemState {
    STANDBY,        // All LEDs ON steady, ready for commands
    EXECUTING_1,    // LED1 blinking, executing item 1
    EXECUTING_2,    // LED2 blinking, executing item 2
    EXECUTING_3,    // LED3 blinking, executing item 3
    EXECUTING_4,    // LED4 blinking, executing item 4
    ALARM           // All LEDs fast blink, system locked
};

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
    void on_btnStartADC_clicked();
    void on_btnStopADC_clicked();
    void on_btnDismissAlarm_clicked();
    void on_btnSetThreshold_clicked();
    void on_btnExecute1_clicked();
    void on_btnExecute2_clicked();
    void on_btnExecute3_clicked();
    void on_btnExecute4_clicked();

    // Timer slots
    void onADCTimerTimeout();
    void onBlinkTimerTimeout();
    void onExecutionTimerTimeout();
    void onAlarmBlinkTimerTimeout();

private:
    Ui::MainWindow *ui;

    // LED controllers (4 LEDs representing 4 machine operations)
    std::shared_ptr<gpio::LED_CTRL> LED1 = std::make_shared<gpio::LED_CTRL>(LOOKUP::PIN::P7);
    std::shared_ptr<gpio::LED_CTRL> LED2 = std::make_shared<gpio::LED_CTRL>(LOOKUP::PIN::P13);
    std::shared_ptr<gpio::LED_CTRL> LED3 = std::make_shared<gpio::LED_CTRL>(LOOKUP::PIN::P15);
    std::shared_ptr<gpio::LED_CTRL> LED4 = std::make_shared<gpio::LED_CTRL>(LOOKUP::PIN::P32);

    // LED states
    bool led1State = false;
    bool led2State = false;
    bool led3State = false;
    bool led4State = false;

    // ADC related (Safety sensor)
    QTimer *adcTimer;
    int currentADC = 0;
    int safetyThreshold = 600;  // ADC > threshold = DANGER
    QString adcScriptPath;
    bool adcMonitoring = false;

    // System state machine
    SystemState currentState = SystemState::STANDBY;
    bool systemLocked = false;

    // Execution related
    int executingItem = 0;          // 0 = none, 1-4 = item number
    int executionProgress = 0;      // 0-100%
    int executionDuration = 5000;   // 5 seconds per item (ms)
    QTimer *executionTimer;

    // Blink related
    QTimer *blinkTimer;             // For executing item blink
    QTimer *alarmBlinkTimer;        // For alarm fast blink
    bool blinkState = false;

    // Helper functions
    void initializeUI();
    void initializeTimers();
    void readADC();
    void onADCValueReceived(int value);
    void updateLEDDisplay();
    void updateStatusDisplay();

    // LED control functions
    void setLED(int ledNum, bool state);
    void setAllLEDs(bool state);

    // State machine functions
    void enterStandby();
    void startExecution(int itemNumber);
    void completeExecution();
    void triggerAlarm();
    void dismissAlarm();
    void checkSafety(int adcValue);

    // Shortcut info
    void showShortcutHelp();
};

#endif // MAINWINDOW_H
