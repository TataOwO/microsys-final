#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Enable keyboard focus
    setFocusPolicy(Qt::StrongFocus);

    // Find the read_adc.py script path
    QString appDir = QCoreApplication::applicationDirPath();
    adcScriptPath = appDir + "/read_adc.py";

    // If not in build dir, try source directory
    if (!QFileInfo::exists(adcScriptPath)) {
        adcScriptPath = QDir::currentPath() + "/read_adc.py";
    }
    if (!QFileInfo::exists(adcScriptPath)) {
        adcScriptPath = "/home/user/microsys-final/final_project/read_adc.py";
    }

    initializeUI();
    initializeTimers();

    // Start in STANDBY mode
    enterStandby();

    std::cout << "Machine Operation Control System initialized" << std::endl;
    std::cout << "ADC Script Path: " << adcScriptPath.toStdString() << std::endl;
}

MainWindow::~MainWindow()
{
    // Cleanup: turn off all LEDs
    setAllLEDs(false);
    delete ui;
}

void MainWindow::initializeUI()
{
    // Set window title
    setWindowTitle("Machine Operation Control System");

    // Initialize LED displays as off (will be turned on in enterStandby)
    ui->ledPic1->hide();
    ui->ledPic2->hide();
    ui->ledPic3->hide();
    ui->ledPic4->hide();

    // Set initial threshold value
    ui->inputThreshold->setText(QString::number(safetyThreshold));

    // Set ADC progress bar range (MCP3008 is 10-bit: 0-1023)
    ui->adcProgressBar->setRange(0, 1023);
    ui->adcProgressBar->setValue(0);

    // Set execution progress bar
    ui->executionProgressBar->setRange(0, 100);
    ui->executionProgressBar->setValue(0);
}

void MainWindow::initializeTimers()
{
    // ADC reading timer (every 500ms)
    adcTimer = new QTimer(this);
    connect(adcTimer, &QTimer::timeout, this, &MainWindow::onADCTimerTimeout);

    // Blink timer for executing item (300ms)
    blinkTimer = new QTimer(this);
    connect(blinkTimer, &QTimer::timeout, this, &MainWindow::onBlinkTimerTimeout);

    // Alarm blink timer (fast - 100ms)
    alarmBlinkTimer = new QTimer(this);
    connect(alarmBlinkTimer, &QTimer::timeout, this, &MainWindow::onAlarmBlinkTimerTimeout);

    // Execution progress timer
    executionTimer = new QTimer(this);
    connect(executionTimer, &QTimer::timeout, this, &MainWindow::onExecutionTimerTimeout);
}

// ============== State Machine Functions ==============

void MainWindow::enterStandby()
{
    currentState = SystemState::STANDBY;
    systemLocked = false;
    executingItem = 0;
    executionProgress = 0;

    // Stop all timers except ADC
    blinkTimer->stop();
    alarmBlinkTimer->stop();
    executionTimer->stop();

    // All LEDs ON steady
    setAllLEDs(true);

    // Update UI
    ui->executionProgressBar->setValue(0);
    updateStatusDisplay();

    std::cout << "Entered STANDBY mode" << std::endl;
}

void MainWindow::startExecution(int itemNumber)
{
    if (systemLocked) {
        QMessageBox::warning(this, "System Locked",
            "Cannot start execution - system is locked!\n"
            "Dismiss the alarm first (press U).");
        return;
    }

    if (currentState != SystemState::STANDBY) {
        QMessageBox::warning(this, "Busy",
            "System is busy. Wait for current operation to complete.");
        return;
    }

    // Set state based on item number
    switch (itemNumber) {
        case 1: currentState = SystemState::EXECUTING_1; break;
        case 2: currentState = SystemState::EXECUTING_2; break;
        case 3: currentState = SystemState::EXECUTING_3; break;
        case 4: currentState = SystemState::EXECUTING_4; break;
        default: return;
    }

    systemLocked = true;
    executingItem = itemNumber;
    executionProgress = 0;

    // All LEDs ON first
    setAllLEDs(true);

    // Start blinking the executing item's LED
    blinkTimer->start(300);  // Blink every 300ms

    // Start execution progress timer (update every 1%)
    executionTimer->start(executionDuration / 100);

    updateStatusDisplay();

    std::cout << "Started execution of Item " << itemNumber << std::endl;
}

void MainWindow::completeExecution()
{
    std::cout << "Execution of Item " << executingItem << " completed" << std::endl;

    // Stop timers
    blinkTimer->stop();
    executionTimer->stop();

    // Return to standby
    enterStandby();

    // Show completion message
    ui->lblStatus->setText("Execution Complete!");
    ui->lblStatus->setStyleSheet("color: green; font-weight: bold;");
}

void MainWindow::triggerAlarm()
{
    std::cout << "!!! ALARM TRIGGERED !!!" << std::endl;

    currentState = SystemState::ALARM;
    systemLocked = true;

    // Stop execution
    blinkTimer->stop();
    executionTimer->stop();
    executingItem = 0;
    executionProgress = 0;

    // Start fast alarm blink (all LEDs)
    alarmBlinkTimer->start(100);  // Very fast blink

    // Update UI
    ui->executionProgressBar->setValue(0);
    updateStatusDisplay();
}

void MainWindow::dismissAlarm()
{
    if (currentState != SystemState::ALARM) {
        return;
    }

    std::cout << "Alarm dismissed" << std::endl;

    alarmBlinkTimer->stop();
    enterStandby();

    ui->lblStatus->setText("Alarm dismissed. System ready.");
    ui->lblStatus->setStyleSheet("color: blue; font-weight: bold;");
}

void MainWindow::checkSafety(int adcValue)
{
    // HIGH ADC = Dark = Something blocking light = DANGER
    // (Based on user's hardware where ADC is high when dark)

    if (adcValue > safetyThreshold) {
        // DANGER detected!
        ui->lblSafetyStatus->setText("DANGER - PROXIMITY ALERT");
        ui->lblSafetyStatus->setStyleSheet("color: red; font-weight: bold;");

        // Trigger alarm if not already in alarm state
        if (currentState != SystemState::ALARM) {
            triggerAlarm();
        }
    } else {
        // Safe
        ui->lblSafetyStatus->setText("SAFE");
        ui->lblSafetyStatus->setStyleSheet("color: green; font-weight: bold;");
    }
}

// ============== Timer Handlers ==============

void MainWindow::onADCTimerTimeout()
{
    readADC();
}

void MainWindow::onBlinkTimerTimeout()
{
    // Blink only the executing item's LED
    if (executingItem > 0 && currentState != SystemState::ALARM) {
        blinkState = !blinkState;
        setLED(executingItem, blinkState);

        // Keep other LEDs ON
        for (int i = 1; i <= 4; i++) {
            if (i != executingItem) {
                setLED(i, true);
            }
        }
    }
}

void MainWindow::onAlarmBlinkTimerTimeout()
{
    // Fast blink ALL LEDs for alarm
    blinkState = !blinkState;
    setAllLEDs(blinkState);
}

void MainWindow::onExecutionTimerTimeout()
{
    executionProgress++;
    ui->executionProgressBar->setValue(executionProgress);

    if (executionProgress >= 100) {
        completeExecution();
    }
}

// ============== ADC Functions ==============

void MainWindow::readADC()
{
    QProcess *process = new QProcess(this);

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        Q_UNUSED(exitCode);
        Q_UNUSED(exitStatus);

        QString output = process->readAllStandardOutput().trimmed();
        QString errorOutput = process->readAllStandardError().trimmed();

        if (!errorOutput.isEmpty()) {
            std::cerr << "ADC Error: " << errorOutput.toStdString() << std::endl;
        }

        bool ok;
        int value = output.toInt(&ok);
        if (ok) {
            onADCValueReceived(value);
        } else {
            std::cerr << "Failed to parse ADC value: " << output.toStdString() << std::endl;
        }

        process->deleteLater();
    });

    process->start("python3", QStringList() << adcScriptPath);
}

void MainWindow::onADCValueReceived(int value)
{
    currentADC = value;

    // Update UI
    ui->adcProgressBar->setValue(value);
    ui->lblADCValue->setText(QString::number(value));

    // Check safety (skip if already in alarm - need manual dismiss)
    if (currentState != SystemState::ALARM) {
        checkSafety(value);
    }
}

// ============== LED Control Functions ==============

void MainWindow::setLED(int ledNum, bool state)
{
    switch(ledNum) {
        case 1:
            led1State = state;
            if (state) {
                LED1->enable();
                ui->ledPic1->show();
            } else {
                LED1->disable();
                ui->ledPic1->hide();
            }
            break;
        case 2:
            led2State = state;
            if (state) {
                LED2->enable();
                ui->ledPic2->show();
            } else {
                LED2->disable();
                ui->ledPic2->hide();
            }
            break;
        case 3:
            led3State = state;
            if (state) {
                LED3->enable();
                ui->ledPic3->show();
            } else {
                LED3->disable();
                ui->ledPic3->hide();
            }
            break;
        case 4:
            led4State = state;
            if (state) {
                LED4->enable();
                ui->ledPic4->show();
            } else {
                LED4->disable();
                ui->ledPic4->hide();
            }
            break;
    }
}

void MainWindow::setAllLEDs(bool state)
{
    setLED(1, state);
    setLED(2, state);
    setLED(3, state);
    setLED(4, state);
}

// ============== Button Handlers ==============

void MainWindow::on_btnStartADC_clicked()
{
    adcMonitoring = true;
    adcTimer->start(500);  // Read ADC every 500ms
    ui->lblStatus->setText("Safety monitoring started");
    updateStatusDisplay();
    std::cout << "ADC monitoring started" << std::endl;
}

void MainWindow::on_btnStopADC_clicked()
{
    adcMonitoring = false;
    adcTimer->stop();
    ui->lblStatus->setText("Safety monitoring stopped");
    ui->lblSafetyStatus->setText("--");
    ui->lblSafetyStatus->setStyleSheet("");
    updateStatusDisplay();
    std::cout << "ADC monitoring stopped" << std::endl;
}

void MainWindow::on_btnDismissAlarm_clicked()
{
    dismissAlarm();
}

void MainWindow::on_btnSetThreshold_clicked()
{
    bool ok;
    int newThreshold = ui->inputThreshold->text().toInt(&ok);

    if (ok && newThreshold >= 0 && newThreshold <= 1023) {
        safetyThreshold = newThreshold;
        std::cout << "Safety threshold set to: " << safetyThreshold << std::endl;
        updateStatusDisplay();
    } else {
        QMessageBox::warning(this, "Invalid Input",
            "Please enter a valid threshold value (0-1023)");
    }
}

void MainWindow::on_btnExecute1_clicked()
{
    startExecution(1);
}

void MainWindow::on_btnExecute2_clicked()
{
    startExecution(2);
}

void MainWindow::on_btnExecute3_clicked()
{
    startExecution(3);
}

void MainWindow::on_btnExecute4_clicked()
{
    startExecution(4);
}

// ============== Keyboard Shortcuts ==============

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()) {
        // Execute items 1-4 (only in STANDBY)
        case Qt::Key_1:
            startExecution(1);
            break;
        case Qt::Key_2:
            startExecution(2);
            break;
        case Qt::Key_3:
            startExecution(3);
            break;
        case Qt::Key_4:
            startExecution(4);
            break;

        // Unlock/Dismiss alarm
        case Qt::Key_U:
            dismissAlarm();
            break;

        // Emergency stop (return to standby)
        case Qt::Key_Escape:
            if (currentState == SystemState::ALARM) {
                dismissAlarm();
            } else {
                enterStandby();
            }
            break;

        // Toggle ADC monitoring
        case Qt::Key_M:
            if (adcMonitoring) {
                on_btnStopADC_clicked();
            } else {
                on_btnStartADC_clicked();
            }
            break;

        // Help
        case Qt::Key_H:
            showShortcutHelp();
            break;

        default:
            QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::showShortcutHelp()
{
    QMessageBox::information(this, "Keyboard Shortcuts",
        "Machine Operation Control System\n"
        "================================\n\n"
        "[1] - Execute Item 1\n"
        "[2] - Execute Item 2\n"
        "[3] - Execute Item 3\n"
        "[4] - Execute Item 4\n"
        "[U] - Unlock / Dismiss Alarm\n"
        "[M] - Toggle Safety Monitoring\n"
        "[ESC] - Emergency Stop\n"
        "[H] - Show this help\n\n"
        "Note: Items can only be executed in STANDBY mode.\n"
        "System locks during execution and alarm.");
}

// ============== Status Display ==============

void MainWindow::updateLEDDisplay()
{
    // Update LED picture visibility based on state
    led1State ? ui->ledPic1->show() : ui->ledPic1->hide();
    led2State ? ui->ledPic2->show() : ui->ledPic2->hide();
    led3State ? ui->ledPic3->show() : ui->ledPic3->hide();
    led4State ? ui->ledPic4->show() : ui->ledPic4->hide();
}

void MainWindow::updateStatusDisplay()
{
    // Update system state display
    QString stateStr;
    QString stateStyle;

    switch (currentState) {
        case SystemState::STANDBY:
            stateStr = "STANDBY";
            stateStyle = "color: green; font-weight: bold;";
            break;
        case SystemState::EXECUTING_1:
            stateStr = "EXECUTING Item 1";
            stateStyle = "color: blue; font-weight: bold;";
            break;
        case SystemState::EXECUTING_2:
            stateStr = "EXECUTING Item 2";
            stateStyle = "color: blue; font-weight: bold;";
            break;
        case SystemState::EXECUTING_3:
            stateStr = "EXECUTING Item 3";
            stateStyle = "color: blue; font-weight: bold;";
            break;
        case SystemState::EXECUTING_4:
            stateStr = "EXECUTING Item 4";
            stateStyle = "color: blue; font-weight: bold;";
            break;
        case SystemState::ALARM:
            stateStr = "!!! ALARM !!!";
            stateStyle = "color: red; font-weight: bold; font-size: 14px;";
            break;
    }

    ui->lblSystemState->setText(stateStr);
    ui->lblSystemState->setStyleSheet(stateStyle);

    // Update lock status
    if (systemLocked) {
        ui->lblLockStatus->setText("LOCKED");
        ui->lblLockStatus->setStyleSheet("color: red; font-weight: bold;");
    } else {
        ui->lblLockStatus->setText("UNLOCKED");
        ui->lblLockStatus->setStyleSheet("color: green; font-weight: bold;");
    }

    // Update monitoring status
    if (adcMonitoring) {
        ui->lblMonitoringStatus->setText("ON");
        ui->lblMonitoringStatus->setStyleSheet("color: green; font-weight: bold;");
    } else {
        ui->lblMonitoringStatus->setText("OFF");
        ui->lblMonitoringStatus->setStyleSheet("color: gray;");
    }

    // Update current item display
    if (executingItem > 0) {
        ui->lblCurrentItem->setText(QString("Item %1").arg(executingItem));
    } else {
        ui->lblCurrentItem->setText("None");
    }

    // Update threshold display
    ui->lblThresholdValue->setText(QString::number(safetyThreshold));
}
