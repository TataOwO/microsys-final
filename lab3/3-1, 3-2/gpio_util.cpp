#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include "gpio_util.hpp"

namespace gpio {
namespace util {
int gpio_export(unsigned int gpio) {
    int fd, len;
    char buf[64];

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) {
        perror("gpio/export");
        return fd;
    }

    len = snprintf(buf, sizeof(buf), "%d", gpio);
    write(fd, buf, len);
    close(fd);

    return 0;
}

int gpio_unexport(unsigned int gpio) {
    int fd, len;
    char buf[64];

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd < 0) {
        perror("gpio/unexport");
        return fd;
    }

    len = snprintf(buf, sizeof(buf), "%d", gpio);
    write(fd, buf, len);
    close(fd);

    return 0;
}

int gpio_set_dir(unsigned int gpio, std::string dirStatus) {
    int fd;
    char buf[64];

	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", gpio);

    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        perror("gpio/direction");
        return fd;
    }

	if (dirStatus == "out") {
		write(fd, "out", 4);
	} else {
		write(fd, "in", 3);
	}

	close(fd);
    return 0;
}

int gpio_set_value(unsigned int gpio, int value) {
    int fd;
    char buf[64];

	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", gpio);

    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        perror("gpio/value");
        return fd;
    }

	if (value == 0) {
		write(fd, "0", 2);
	} else {
		write(fd, "1", 2);
	}

	close(fd);
    return 0;
}
}
};

//int main(int argc, char* argv[]) {
//	int input;
//	std::cout << "輸入1為啟用GPIO 255並設定狀態為1\n輸入2為將GPIO 255設定狀態為0，並停用GPIO 255\n>>>";
//	std::cin >> input;
//	
//	if (input == 1) {
//		cgpio::util::gpio_export(255);
//		cgpio::util::gpio_set_dir(255, "out");
//		cgpio::util::gpio_set_value(255, 1);
//	} else if (input == 2) {
//		cgpio::util::gpio_set_value(255, 0);
//		cgpio::util::gpio_unexport(255);
//	}
//
//	return 0;
//}

