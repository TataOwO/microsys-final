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

    std::cout << "Smart Light Control System initialized" << std::endl;
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
    setWindowTitle("Smart Light Control System - Final Project");

    // Initialize LED displays as off
    ui->ledPic1->hide();
    ui->ledPic2->hide();
    ui->ledPic3->hide();
    ui->ledPic4->hide();

    // Set initial threshold value
    ui->inputThreshold->setText(QString::number(threshold));

    // Set initial ADC progress bar range (MCP3008 is 10-bit: 0-1023)
    ui->adcProgressBar->setRange(0, 1023);
    ui->adcProgressBar->setValue(0);

    // Update status displays
    updateStatusDisplay();
}

void MainWindow::initializeTimers()
{
    // ADC reading timer (every 500ms)
    adcTimer = new QTimer(this);
    connect(adcTimer, &QTimer::timeout, this, &MainWindow::onADCTimerTimeout);

    // Blink timer (for LED blinking effect)
    blinkTimer = new QTimer(this);
    connect(blinkTimer, &QTimer::timeout, this, &MainWindow::onBlinkTimerTimeout);
}

// ============== Keyboard Shortcuts (Item 1: 4+ shortcuts) ==============

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (!systemRunning && event->key() != Qt::Key_H) {
        // Only allow 'H' for help when system is stopped
        if (event->key() == Qt::Key_H) {
            showShortcutHelp();
        }
        return;
    }

    switch(event->key()) {
        // Shortcut 1-4: Toggle individual LEDs
        case Qt::Key_1:
            toggleLED(1);
            break;
        case Qt::Key_2:
            toggleLED(2);
            break;
        case Qt::Key_3:
            toggleLED(3);
            break;
        case Qt::Key_4:
            toggleLED(4);
            break;

        // Shortcut 5: Toggle Auto Mode
        case Qt::Key_A:
            on_btnToggleMode_clicked();
            break;

        // Shortcut 6: Blink All LEDs
        case Qt::Key_B:
            on_btnBlinkAll_clicked();
            break;

        // Shortcut 7: Stop System
        case Qt::Key_S:
            on_btnStop_clicked();
            break;

        // Shortcut 8: Read ADC manually
        case Qt::Key_R:
            readADC();
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
        "Keyboard Shortcuts:\n\n"
        "[1] - Toggle LED 1\n"
        "[2] - Toggle LED 2\n"
        "[3] - Toggle LED 3\n"
        "[4] - Toggle LED 4\n"
        "[A] - Toggle Auto/Manual Mode\n"
        "[B] - Blink All LEDs\n"
        "[S] - Stop System\n"
        "[R] - Read ADC Value\n"
        "[H] - Show this help");
}

// ============== Button Handlers ==============

void MainWindow::on_btnStart_clicked()
{
    systemRunning = true;
    adcTimer->start(500);  // Read ADC every 500ms
    updateStatusDisplay();
    std::cout << "System started" << std::endl;
}

void MainWindow::on_btnStop_clicked()
{
    systemRunning = false;
    adcTimer->stop();
    stopBlinking();
    setAllLEDs(false);
    updateStatusDisplay();
    std::cout << "System stopped" << std::endl;
}

void MainWindow::on_btnBlinkAll_clicked()
{
    if (isBlinking) {
        stopBlinking();
    } else {
        blinkAllLEDs();
    }
}

void MainWindow::on_btnAllOn_clicked()
{
    stopBlinking();
    setAllLEDs(true);
}

void MainWindow::on_btnAllOff_clicked()
{
    stopBlinking();
    setAllLEDs(false);
}

void MainWindow::on_btnSetThreshold_clicked()
{
    bool ok;
    int newThreshold = ui->inputThreshold->text().toInt(&ok);

    if (ok && newThreshold >= 0 && newThreshold <= 1023) {
        threshold = newThreshold;
        std::cout << "Threshold set to: " << threshold << std::endl;
        updateStatusDisplay();
    } else {
        QMessageBox::warning(this, "Invalid Input",
            "Please enter a valid threshold value (0-1023)");
    }
}

void MainWindow::on_btnToggleMode_clicked()
{
    autoMode = !autoMode;
    updateStatusDisplay();

    if (autoMode) {
        stopBlinking();  // Stop blinking when entering auto mode
        std::cout << "Switched to AUTO mode" << std::endl;
    } else {
        std::cout << "Switched to MANUAL mode" << std::endl;
    }
}

// ============== ADC Functions (Item 3) ==============

void MainWindow::onADCTimerTimeout()
{
    readADC();
}

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

    // Update light status
    if (value < threshold) {
        ui->lblLightStatus->setText("DARK");
        ui->lblLightStatus->setStyleSheet("color: #555; font-weight: bold;");
    } else {
        ui->lblLightStatus->setText("BRIGHT");
        ui->lblLightStatus->setStyleSheet("color: #ffcc00; font-weight: bold;");
    }

    // Auto control LEDs if in auto mode (Item 3: control LED based on signal)
    if (autoMode && !isBlinking) {
        autoControlLEDs();
    }
}

void MainWindow::autoControlLEDs()
{
    if (currentADC < threshold) {
        // Dark environment - turn on LEDs
        setAllLEDs(true);
    } else {
        // Bright environment - turn off LEDs
        setAllLEDs(false);
    }
}

// ============== LED Control Functions (Item 2) ==============

void MainWindow::toggleLED(int ledNum)
{
    if (autoMode) {
        std::cout << "Cannot toggle LED in AUTO mode" << std::endl;
        return;
    }

    switch(ledNum) {
        case 1:
            setLED(1, !led1State);
            break;
        case 2:
            setLED(2, !led2State);
            break;
        case 3:
            setLED(3, !led3State);
            break;
        case 4:
            setLED(4, !led4State);
            break;
    }
}

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

    std::cout << "LED " << ledNum << " " << (state ? "ON" : "OFF") << std::endl;
}

void MainWindow::setAllLEDs(bool state)
{
    setLED(1, state);
    setLED(2, state);
    setLED(3, state);
    setLED(4, state);
}

void MainWindow::blinkAllLEDs()
{
    isBlinking = true;
    blinkState = false;
    blinkTimer->start(300);  // Blink every 300ms
    ui->btnBlinkAll->setText("Stop Blink");
    std::cout << "Started blinking all LEDs" << std::endl;
}

void MainWindow::stopBlinking()
{
    if (isBlinking) {
        isBlinking = false;
        blinkTimer->stop();
        ui->btnBlinkAll->setText("Blink All");
        std::cout << "Stopped blinking" << std::endl;
    }
}

void MainWindow::onBlinkTimerTimeout()
{
    blinkState = !blinkState;
    setAllLEDs(blinkState);
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
    // Update system status
    if (systemRunning) {
        ui->lblSystemStatus->setText("RUNNING");
        ui->lblSystemStatus->setStyleSheet("color: green; font-weight: bold;");
    } else {
        ui->lblSystemStatus->setText("STOPPED");
        ui->lblSystemStatus->setStyleSheet("color: red; font-weight: bold;");
    }

    // Update mode status
    if (autoMode) {
        ui->btnToggleMode->setText("Switch to Manual");
        ui->lblModeStatus->setText("AUTO");
        ui->lblModeStatus->setStyleSheet("color: blue; font-weight: bold;");
    } else {
        ui->btnToggleMode->setText("Switch to Auto");
        ui->lblModeStatus->setText("MANUAL");
        ui->lblModeStatus->setStyleSheet("color: orange; font-weight: bold;");
    }

    // Update threshold display
    ui->lblThresholdValue->setText(QString::number(threshold));
}
