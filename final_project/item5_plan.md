# Item 5 Implementation Plan: Kernel Driver Enhancement (Black Box)

## Requirement Summary

From the project requirements (Page 12):

> **Item 5 (10%): Driver Enhancement**
> - Implement "black box" functionality in Kernel Driver
> - Log each ioctl operation (timestamp, password input, result) to Kernel Buffer
> - Implement read operation for User Space to retrieve and display "System Log" in Qt

---

## Current State Analysis

### Existing Kernel Driver (lab9/lab10 `demo.c`)

```c
// Current functionality:
- drv_open()    → Just prints message
- drv_release() → Just prints message
- drv_read()    → Returns LED status (4 bytes)
- drv_write()   → Controls LEDs via GPIO
- drv_ioctl()   → Empty (just prints message)
```

### What We Need to Add

1. **Log Buffer** - Store operation history in kernel memory
2. **Timestamp** - Record when each operation occurred
3. **Password Verification** - ioctl command to verify password
4. **Log Retrieval** - Read operation to get logs from user space
5. **Qt Integration** - Display logs in GUI

---

## Architecture Design

### Data Flow

```
┌─────────────────────────────────────────────────────────────┐
│                        Qt GUI                               │
│  ┌─────────────────┐        ┌─────────────────────────────┐ │
│  │ Password Input  │        │      System Log Display     │ │
│  │ [____] [Verify] │        │  [2024-01-09 14:30:15]      │ │
│  └────────┬────────┘        │  Password: **** → SUCCESS   │ │
│           │                 │  [2024-01-09 14:30:20]      │ │
│           │                 │  Password: **** → FAILED    │ │
│           ▼                 └──────────────▲──────────────┘ │
│     ioctl(fd, cmd, pw)              read(fd, buf, len)      │
└───────────┬─────────────────────────────────┬───────────────┘
            │                                 │
════════════╪═══════════ KERNEL SPACE ════════╪════════════════
            │                                 │
            ▼                                 │
┌─────────────────────────────────────────────┴───────────────┐
│                    Kernel Driver (demo.c)                   │
│                                                             │
│  ┌──────────────┐    ┌────────────────────────────────────┐ │
│  │  drv_ioctl() │    │         Log Buffer (Ring)          │ │
│  │              │───▶│  [0] timestamp | password | result │ │
│  │ - Verify pw  │    │  [1] timestamp | password | result │ │
│  │ - Log result │    │  [2] timestamp | password | result │ │
│  └──────────────┘    │  ...                               │ │
│                      └────────────────────────────────────┘ │
│                                     │                       │
│  ┌──────────────┐                   │                       │
│  │  drv_read()  │◀──────────────────┘                       │
│  │              │                                           │
│  │ - Return logs│                                           │
│  └──────────────┘                                           │
└─────────────────────────────────────────────────────────────┘
```

---

## Implementation Details

### 1. Kernel Driver Modifications

#### 1.1 Log Entry Structure

```c
#define MAX_LOG_ENTRIES 50
#define MAX_PASSWORD_LEN 32
#define MAX_LOG_LINE 128

struct log_entry {
    unsigned long timestamp;      // Kernel jiffies or seconds
    char password[MAX_PASSWORD_LEN];  // Input password (masked for display)
    int result;                   // 0 = failed, 1 = success
};

static struct log_entry log_buffer[MAX_LOG_ENTRIES];
static int log_head = 0;          // Next write position
static int log_count = 0;         // Total entries (max MAX_LOG_ENTRIES)
```

#### 1.2 IOCTL Commands

```c
#define IOCTL_VERIFY_PASSWORD _IOW('k', 1, char*)   // Verify password
#define IOCTL_GET_LOG_COUNT   _IOR('k', 2, int*)    // Get number of logs
#define IOCTL_CLEAR_LOGS      _IO('k', 3)           // Clear log buffer

// The correct password (can be configured)
#define CORRECT_PASSWORD "1234"
```

#### 1.3 Modified drv_ioctl()

```c
static long drv_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    char user_password[MAX_PASSWORD_LEN];
    int result;
    struct log_entry *entry;

    switch (cmd) {
        case IOCTL_VERIFY_PASSWORD:
            // Copy password from user space
            if (copy_from_user(user_password, (char*)arg, MAX_PASSWORD_LEN)) {
                return -EFAULT;
            }
            user_password[MAX_PASSWORD_LEN-1] = '\0';

            // Verify password
            result = (strcmp(user_password, CORRECT_PASSWORD) == 0) ? 1 : 0;

            // Log this attempt
            entry = &log_buffer[log_head];
            entry->timestamp = get_seconds();  // Current time
            strncpy(entry->password, user_password, MAX_PASSWORD_LEN);
            entry->result = result;

            // Update ring buffer index
            log_head = (log_head + 1) % MAX_LOG_ENTRIES;
            if (log_count < MAX_LOG_ENTRIES) log_count++;

            printk("Password verification: %s → %s\n",
                   user_password, result ? "SUCCESS" : "FAILED");

            return result;

        case IOCTL_GET_LOG_COUNT:
            if (copy_to_user((int*)arg, &log_count, sizeof(int))) {
                return -EFAULT;
            }
            return 0;

        case IOCTL_CLEAR_LOGS:
            log_head = 0;
            log_count = 0;
            printk("Logs cleared\n");
            return 0;

        default:
            return -EINVAL;
    }
}
```

#### 1.4 Modified drv_read() - Return Logs

```c
static ssize_t drv_read(struct file *filp, char *buf, size_t count, loff_t *ppos) {
    char output[4096];  // Output buffer
    int len = 0;
    int i, idx;
    struct log_entry *entry;
    struct tm tm_result;

    // Format all log entries
    len += sprintf(output + len, "=== System Log (%d entries) ===\n", log_count);

    for (i = 0; i < log_count; i++) {
        // Calculate actual index in ring buffer
        idx = (log_head - log_count + i + MAX_LOG_ENTRIES) % MAX_LOG_ENTRIES;
        entry = &log_buffer[idx];

        // Format: [timestamp] Password: **** → SUCCESS/FAILED
        len += sprintf(output + len, "[%lu] Password: %s → %s\n",
                      entry->timestamp,
                      entry->password,  // Could mask this for security
                      entry->result ? "SUCCESS" : "FAILED");
    }

    // Copy to user space
    if (count < len) len = count;
    if (copy_to_user(buf, output, len)) {
        return -EFAULT;
    }

    return len;
}
```

---

### 2. Qt GUI Modifications

#### 2.1 New UI Elements

Add to `mainwindow.ui`:
- `QGroupBox` for "System Log" section
- `QTextEdit` (read-only) for displaying logs: `txtSystemLog`
- `QLineEdit` for password input: `inputPassword`
- `QPushButton` for verify: `btnVerify`
- `QPushButton` for refresh logs: `btnRefreshLog`
- `QPushButton` for clear logs: `btnClearLog`

#### 2.2 New Header Definitions

Add to `mainwindow.h`:
```cpp
// Device path
#define DEVICE_PATH "/dev/demo"

// IOCTL commands (must match kernel)
#define IOCTL_VERIFY_PASSWORD _IOW('k', 1, char*)
#define IOCTL_GET_LOG_COUNT   _IOR('k', 2, int*)
#define IOCTL_CLEAR_LOGS      _IO('k', 3)

private:
    int deviceFd = -1;  // File descriptor for /dev/demo

    void openDevice();
    void closeDevice();
    void verifyPassword();
    void refreshSystemLog();
    void clearSystemLog();
```

#### 2.3 Implementation

```cpp
void MainWindow::openDevice() {
    deviceFd = open(DEVICE_PATH, O_RDWR);
    if (deviceFd < 0) {
        qDebug() << "Failed to open device:" << DEVICE_PATH;
    }
}

void MainWindow::verifyPassword() {
    if (deviceFd < 0) {
        QMessageBox::warning(this, "Error", "Device not open");
        return;
    }

    QString password = ui->inputPassword->text();
    QByteArray pwBytes = password.toUtf8();

    int result = ioctl(deviceFd, IOCTL_VERIFY_PASSWORD, pwBytes.data());

    if (result == 1) {
        QMessageBox::information(this, "Success", "Password verified!");
        // Enable some protected functionality here
    } else {
        QMessageBox::warning(this, "Failed", "Incorrect password!");
    }

    // Auto-refresh log display
    refreshSystemLog();
}

void MainWindow::refreshSystemLog() {
    if (deviceFd < 0) return;

    char buffer[4096];
    ssize_t bytesRead = read(deviceFd, buffer, sizeof(buffer) - 1);

    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        ui->txtSystemLog->setText(QString::fromUtf8(buffer));
    }
}

void MainWindow::clearSystemLog() {
    if (deviceFd < 0) return;

    ioctl(deviceFd, IOCTL_CLEAR_LOGS, 0);
    refreshSystemLog();
}
```

---

### 3. File Structure

```
final_project/
├── ... (existing files)
├── kernel/
│   ├── demo.c          # Enhanced kernel driver
│   ├── Makefile        # Kernel module build
│   └── demo_ioctl.h    # Shared IOCTL definitions
└── mainwindow.cpp      # Updated with log display
```

---

## Implementation Steps

### Step 1: Create Kernel Module (30 min)

1. Create `final_project/kernel/` directory
2. Copy and modify `demo.c` from lab9
3. Add log buffer and structures
4. Implement enhanced `drv_ioctl()` with password verification
5. Modify `drv_read()` to return formatted logs
6. Create shared header `demo_ioctl.h` for IOCTL definitions
7. Create Makefile

### Step 2: Update Qt GUI (30 min)

1. Add new UI elements in Qt Designer or manually in `.ui`
2. Add IOCTL definitions to `mainwindow.h`
3. Implement device open/close
4. Implement `verifyPassword()`
5. Implement `refreshSystemLog()`
6. Implement `clearSystemLog()`
7. Add keyboard shortcut for password verify (e.g., `P`)

### Step 3: Testing (15 min)

1. Build and load kernel module
2. Create device node: `sudo mknod /dev/demo c 60 0`
3. Build Qt application
4. Test password verification
5. Test log display
6. Verify logs persist across multiple attempts

---

## Build & Run Instructions

### Kernel Module

```bash
cd final_project/kernel

# Build
make

# Load module
sudo insmod demo.ko

# Create device node (if not exists)
sudo mknod /dev/demo c 60 0
sudo chmod 666 /dev/demo

# Verify module loaded
lsmod | grep demo
dmesg | tail
```

### Qt Application

```bash
cd final_project
qmake && make
sudo ./final_project
```

### Unload Module

```bash
sudo rmmod demo
```

---

## Expected Demo Flow

1. **Start Application** → Qt GUI opens
2. **Enter wrong password** → "FAILED" message, logged
3. **Enter correct password (1234)** → "SUCCESS" message, logged
4. **Click "Refresh Log"** → See all attempts in System Log
5. **Try multiple passwords** → All logged with timestamps
6. **Click "Clear Log"** → Logs cleared

---

## Risk Assessment

| Risk | Mitigation |
|------|------------|
| Kernel compilation issues | Use existing lab9 Makefile as base |
| IOCTL number conflicts | Use `_IOW`/`_IOR` macros with unique type 'k' |
| Buffer overflow | Fixed-size buffers with length checks |
| Device permission | Create udev rule or chmod after mknod |

---

## Summary

This implementation adds "black box" logging to the kernel driver:

- **Kernel Side**: Ring buffer stores last 50 operations with timestamps
- **User Side**: Qt GUI displays logs and allows password verification
- **Integration**: Uses standard Linux ioctl/read interface

Estimated time: **~1-1.5 hours** for full implementation and testing.
