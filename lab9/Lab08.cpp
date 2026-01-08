#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <string>
#include <cstring>

using namespace std;

#define DEVICE_PATH "/dev/demo"

int main(int argc, char *argv[]) {
    // Check if the argc is sufficient
    if (argc < 2) {
        cout << "Usage: ./Lab8 <command>" << endl;
        cout << "Example (Control): ./Lab8 LED1on" << endl;
        cout << "Example (Status):  ./Lab8 LED1" << endl;
        return -1;
    }

    string cmd = argv[1];
    int fd;

    // open file (read mode)
    // ensure ur /dev/demo permissions must allow read and write access (usually requires sudo)
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }

    // identify whether it is a "control command" or a "query command".
    if (cmd.find("on") != string::npos || cmd.find("off") != string::npos) {
        // --- Write ---
        // let OP (like "LED1on") write to the driver directly
        ssize_t ret = write(fd, cmd.c_str(), cmd.length());
        if (ret < 0) {
            perror("Write failed");
        } else {
            // write success
        }
    } 
    else {
        // --- Read ---
        // load all statuses of LED ( 4 bytes means 4 LED)
        char statusBuf[4] = {0}; 
        ssize_t ret = read(fd, statusBuf, 4);

        if (ret < 0) {
            perror("Read failed");
        } else {
            if (cmd == "LED1") {
                cout << "LED1 Status: " << statusBuf[0] << endl;
            } else if (cmd == "LED2") {
                cout << "LED2 Status: " << statusBuf[1] << endl;
            } else if (cmd == "LED3") {
                cout << "LED3 Status: " << statusBuf[2] << endl;
            } else if (cmd == "LED4") {
                cout << "LED4 Status: " << statusBuf[3] << endl;
            } else {
                cout << "Unknown LED command. Raw Status: " 
                     << statusBuf[0] << statusBuf[1] << statusBuf[2] << statusBuf[3] << endl;
            }
        }
    }

    close(fd);
    return 0;
}
