# Machine Operation Control System - Implementation Plan

## Backstory / Scenario

> We are system design engineers creating a "Machine Operation Execution" system.
>
> **Requirements:**
> - Use LED indicators to show current progress (blinking = executing)
> - Standby mode: LEDs stay ON steady
> - Executing mode: The executing item's LED blinks
> - Auto-detect if surroundings are safe (ADC photoresistor as proximity sensor)
> - If someone gets too close → trigger alarm, all LEDs blink, interrupt execution, lock system
> - Can manually select which item to execute
>
> **Core Concept:** Lock when executing, unlock when complete. Lock on safety alarm, unlock when manually dismissed.

---

## System State Machine

```
                    ┌─────────────────────────────────────────┐
                    │                                         │
                    ▼                                         │
            ┌───────────────┐                                 │
            │    STANDBY    │                                 │
            │ (All LEDs ON) │                                 │
            └───────┬───────┘                                 │
                    │                                         │
        ┌───────────┼───────────┐                             │
        │           │           │                             │
        ▼           ▼           ▼                             │
   [Press 1]   [Press 2]   [Press 3/4]                        │
        │           │           │                             │
        ▼           ▼           ▼                             │
┌───────────┐ ┌───────────┐ ┌───────────┐                     │
│EXECUTING_1│ │EXECUTING_2│ │EXECUTING_X│                     │
│(LED1 blink│ │(LED2 blink│ │(LEDX blink│                     │
│ others ON)│ │ others ON)│ │ others ON)│                     │
└─────┬─────┘ └─────┬─────┘ └─────┬─────┘                     │
      │             │             │                           │
      │   ADC < threshold (DANGER!)                           │
      │             │             │                           │
      └─────────────┼─────────────┘                           │
                    │                                         │
                    ▼                                         │
            ┌───────────────┐      Execution complete         │
            │     ALARM     │ ─────────────────────────────────
            │(ALL LEDs FAST │
            │    BLINK)     │
            │   [LOCKED]    │
            └───────┬───────┘
                    │
              [Press 'U' to unlock]
              [or Password]
                    │
                    ▼
            ┌───────────────┐
            │    STANDBY    │
            └───────────────┘
```

---

## State Definitions

| State | LED Behavior | System Status | ADC Monitoring |
|-------|--------------|---------------|----------------|
| **STANDBY** | All 4 LEDs ON (steady) | Unlocked, ready | Active |
| **EXECUTING_1** | LED1 blinks, LED2-4 ON | Locked | Active |
| **EXECUTING_2** | LED2 blinks, LED1,3,4 ON | Locked | Active |
| **EXECUTING_3** | LED3 blinks, LED1,2,4 ON | Locked | Active |
| **EXECUTING_4** | LED4 blinks, LED1-3 ON | Locked | Active |
| **ALARM** | ALL LEDs fast blink | Locked, Interrupted | Paused |

---

## ADC Safety Logic

**Concept**: Photoresistor detects if someone/something is blocking the light (too close to machine)

```
ADC Value Interpretation:
─────────────────────────────────────────────────────
HIGH (800-1023)  │  Bright  │  SAFE    │  Normal operation
MEDIUM (400-799) │  Normal  │  SAFE    │  Normal operation
LOW (0-399)      │  Dark    │  DANGER! │  Something blocking light
─────────────────────────────────────────────────────

Note: User mentioned ADC is LOW when BRIGHT, HIGH when DARK
      So we invert: HIGH value = DARK = DANGER
```

**Adjusted for your hardware** (ADC high when dark):
```
ADC > threshold (e.g., 600) → DANGER (something blocking light)
ADC ≤ threshold             → SAFE
```

---

## Implementation Changes

### 1. New State Management

**Add to `mainwindow.h`:**

```cpp
// System states
enum class SystemState {
    STANDBY,        // All LEDs ON steady, ready for commands
    EXECUTING_1,    // LED1 blinking, executing item 1
    EXECUTING_2,    // LED2 blinking, executing item 2
    EXECUTING_3,    // LED3 blinking, executing item 3
    EXECUTING_4,    // LED4 blinking, executing item 4
    ALARM           // All LEDs fast blink, system locked
};

private:
    SystemState currentState = SystemState::STANDBY;
    bool systemLocked = false;
    int executingItem = 0;          // 0 = none, 1-4 = item number
    int executionProgress = 0;      // 0-100%
    int executionDuration = 5000;   // 5 seconds per item (configurable)

    QTimer *executionTimer;         // Timer for execution progress
    QTimer *alarmBlinkTimer;        // Fast blink timer for alarm

    void enterStandby();
    void startExecution(int itemNumber);
    void completeExecution();
    void triggerAlarm();
    void dismissAlarm();
    void checkSafety(int adcValue);
```

### 2. Modified LED Behavior

**Standby Mode:**
```cpp
void MainWindow::enterStandby() {
    currentState = SystemState::STANDBY;
    systemLocked = false;
    executingItem = 0;

    // Stop all timers
    blinkTimer->stop();
    alarmBlinkTimer->stop();

    // All LEDs ON steady
    setLED(1, true);
    setLED(2, true);
    setLED(3, true);
    setLED(4, true);

    updateStatusDisplay();
}
```

**Executing Mode:**
```cpp
void MainWindow::startExecution(int itemNumber) {
    if (systemLocked) {
        showMessage("System is locked!");
        return;
    }

    currentState = static_cast<SystemState>(
        static_cast<int>(SystemState::EXECUTING_1) + itemNumber - 1
    );
    systemLocked = true;
    executingItem = itemNumber;
    executionProgress = 0;

    // All LEDs ON
    setAllLEDs(true);

    // Start blinking the executing item's LED
    blinkTimer->start(300);  // Blink every 300ms

    // Start execution progress timer
    executionTimer->start(executionDuration / 100);  // Update every 1%

    updateStatusDisplay();
}

void MainWindow::onBlinkTimerTimeout() {
    if (currentState == SystemState::ALARM) {
        // Fast blink ALL LEDs
        blinkState = !blinkState;
        setAllLEDs(blinkState);
    } else if (executingItem > 0) {
        // Only blink the executing item's LED
        blinkState = !blinkState;
        setLED(executingItem, blinkState);
        // Keep others ON
        for (int i = 1; i <= 4; i++) {
            if (i != executingItem) setLED(i, true);
        }
    }
}
```

**Alarm Mode:**
```cpp
void MainWindow::triggerAlarm() {
    currentState = SystemState::ALARM;
    systemLocked = true;

    // Stop execution
    executionTimer->stop();
    executingItem = 0;
    executionProgress = 0;

    // Start fast alarm blink (all LEDs)
    alarmBlinkTimer->start(100);  // Very fast blink

    // Show alarm message
    ui->lblStatus->setText("⚠️ ALARM - SYSTEM LOCKED ⚠️");
    ui->lblStatus->setStyleSheet("color: red; font-weight: bold; font-size: 16px;");

    updateStatusDisplay();
}

void MainWindow::dismissAlarm() {
    if (currentState != SystemState::ALARM) return;

    alarmBlinkTimer->stop();
    enterStandby();

    ui->lblStatus->setText("Alarm dismissed. System ready.");
}
```

### 3. Modified ADC Safety Check

```cpp
void MainWindow::onADCValueReceived(int value) {
    currentADC = value;

    // Update UI
    ui->adcProgressBar->setValue(value);
    ui->lblADCValue->setText(QString::number(value));

    // Check safety (skip if already in alarm)
    if (currentState != SystemState::ALARM) {
        checkSafety(value);
    }
}

void MainWindow::checkSafety(int adcValue) {
    // HIGH ADC = Dark = Something blocking = DANGER
    // (Adjusted based on your hardware where ADC is high when dark)

    if (adcValue > safetyThreshold) {
        // DANGER detected!
        ui->lblSafetyStatus->setText("⚠️ DANGER - PROXIMITY ALERT");
        ui->lblSafetyStatus->setStyleSheet("color: red; font-weight: bold;");

        // Trigger alarm if executing or in standby
        if (currentState != SystemState::ALARM) {
            triggerAlarm();
        }
    } else {
        // Safe
        ui->lblSafetyStatus->setText("✓ SAFE");
        ui->lblSafetyStatus->setStyleSheet("color: green;");
    }
}
```

### 4. Modified Keyboard Shortcuts

```cpp
void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch(event->key()) {
        // Execute items 1-4
        case Qt::Key_1:
            if (currentState == SystemState::STANDBY)
                startExecution(1);
            break;
        case Qt::Key_2:
            if (currentState == SystemState::STANDBY)
                startExecution(2);
            break;
        case Qt::Key_3:
            if (currentState == SystemState::STANDBY)
                startExecution(3);
            break;
        case Qt::Key_4:
            if (currentState == SystemState::STANDBY)
                startExecution(4);
            break;

        // Unlock/Dismiss alarm
        case Qt::Key_U:
            if (currentState == SystemState::ALARM)
                dismissAlarm();
            break;

        // Emergency stop (go to standby)
        case Qt::Key_Escape:
            enterStandby();
            break;

        // Help
        case Qt::Key_H:
            showShortcutHelp();
            break;
    }
}
```

### 5. Updated UI Layout

```
┌─────────────────────────────────────────────────────────────┐
│          Machine Operation Control System                   │
├─────────────────────────────────────────────────────────────┤
│   ┌─────────────────────────────────────────────────────┐   │
│   │  [LED1]     [LED2]     [LED3]     [LED4]            │   │
│   │  Item 1     Item 2     Item 3     Item 4            │   │
│   │  (Press 1)  (Press 2)  (Press 3)  (Press 4)         │   │
│   └─────────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────┤
│   System State:  [  STANDBY  ]     Lock: [UNLOCKED]         │
│                                                             │
│   Execution:     [░░░░░░░░░░░░░░░░░░░░] 0%                 │
│   Current Item:  None                                       │
├─────────────────────────────────────────────────────────────┤
│   Safety Sensor: [████████░░░░░] 650    Status: ✓ SAFE     │
│   Threshold:     [____600____] [Set]                        │
├─────────────────────────────────────────────────────────────┤
│   [Start ADC] [Stop ADC] [Dismiss Alarm]                    │
│                                                             │
│   Shortcuts: 1-4=Execute Item, U=Unlock, ESC=Stop, H=Help   │
└─────────────────────────────────────────────────────────────┘
```

---

## Updated Keyboard Shortcuts

| Key | Action | Available In |
|-----|--------|--------------|
| `1` | Execute Item 1 | STANDBY only |
| `2` | Execute Item 2 | STANDBY only |
| `3` | Execute Item 3 | STANDBY only |
| `4` | Execute Item 4 | STANDBY only |
| `U` | Dismiss Alarm / Unlock | ALARM only |
| `ESC` | Emergency Stop → Standby | Any state |
| `H` | Show Help | Any state |

---

## Demo Flow

### Normal Operation Demo

1. **Start** → System in STANDBY, all LEDs ON steady
2. **Press 1** → LED1 starts blinking, others stay ON, progress bar fills
3. **Wait 5 seconds** → Execution complete, back to STANDBY
4. **Press 2** → LED2 starts blinking, others stay ON
5. **Complete** → All LEDs ON steady

### Safety Alarm Demo

1. **Start** → System in STANDBY
2. **Press 1** → Start executing Item 1
3. **Cover photoresistor with hand** → ADC goes HIGH (dark)
4. **ALARM triggered** → ALL LEDs blink rapidly, system LOCKED
5. **Uncover photoresistor** → Still in ALARM (must manually dismiss)
6. **Press U** → Alarm dismissed, back to STANDBY

---

## File Changes Summary

| File | Changes |
|------|---------|
| `mainwindow.h` | Add SystemState enum, new member variables, new methods |
| `mainwindow.cpp` | Implement state machine, modify LED behavior, safety check |
| `mainwindow.ui` | Update labels, add progress bar for execution, safety status |

---

## Implementation Checklist

- [ ] Add `SystemState` enum to header
- [ ] Add state-related member variables
- [ ] Implement `enterStandby()`
- [ ] Implement `startExecution(int item)`
- [ ] Implement `completeExecution()`
- [ ] Implement `triggerAlarm()`
- [ ] Implement `dismissAlarm()`
- [ ] Modify `onBlinkTimerTimeout()` for different blink modes
- [ ] Modify `checkSafety()` for danger detection
- [ ] Update `keyPressEvent()` for new shortcuts
- [ ] Add execution progress timer
- [ ] Update UI with new labels and progress bar
- [ ] Test all state transitions
- [ ] Test alarm trigger and dismiss

---

## Estimated Time

| Task | Time |
|------|------|
| Header modifications | 10 min |
| State machine implementation | 30 min |
| LED behavior modifications | 20 min |
| ADC safety logic | 10 min |
| UI updates | 15 min |
| Testing | 15 min |
| **Total** | **~1.5 hours** |
