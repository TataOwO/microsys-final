/**
 * GPIO Test Program
 * Tests the gpio_util functions for controlling GPIO pins
 *
 * Usage: sudo ./test_gpio [test_number]
 *   1 - Test single GPIO export/unexport
 *   2 - Test GPIO direction setting
 *   3 - Test GPIO value setting (LED blink)
 *   4 - Run all tests
 */

#include <iostream>
#include <unistd.h>
#include <fstream>
#include <string>
#include "../gpio_util.hpp"
#include "../PIN_LOOKUP.hpp"

// Test result tracking
int tests_passed = 0;
int tests_failed = 0;

// Color codes for terminal output
#define GREEN "\033[32m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

void print_result(const std::string& test_name, bool passed) {
    if (passed) {
        std::cout << GREEN << "[PASS] " << RESET << test_name << std::endl;
        tests_passed++;
    } else {
        std::cout << RED << "[FAIL] " << RESET << test_name << std::endl;
        tests_failed++;
    }
}

void print_header(const std::string& header) {
    std::cout << std::endl << YELLOW << "=== " << header << " ===" << RESET << std::endl;
}

// Check if GPIO is exported by checking if the directory exists
bool is_gpio_exported(int gpio) {
    std::string path = "/sys/class/gpio/gpio" + std::to_string(gpio);
    std::ifstream f(path);
    return f.good();
}

// Check GPIO direction
std::string get_gpio_direction(int gpio) {
    std::string path = "/sys/class/gpio/gpio" + std::to_string(gpio) + "/direction";
    std::ifstream f(path);
    std::string dir;
    if (f >> dir) {
        return dir;
    }
    return "unknown";
}

// Check GPIO value
int get_gpio_value(int gpio) {
    std::string path = "/sys/class/gpio/gpio" + std::to_string(gpio) + "/value";
    std::ifstream f(path);
    int val = -1;
    f >> val;
    return val;
}

// Test 1: GPIO Export/Unexport
void test_gpio_export_unexport() {
    print_header("Test 1: GPIO Export/Unexport");

    int test_gpio = LOOKUP::PIN::P7;  // GPIO 396

    // Make sure it's unexported first
    gpio::util::gpio_unexport(test_gpio);
    usleep(100000);  // 100ms delay

    // Test export
    std::cout << "  Exporting GPIO " << test_gpio << "..." << std::endl;
    int result = gpio::util::gpio_export(test_gpio);
    usleep(100000);

    bool export_success = is_gpio_exported(test_gpio);
    print_result("gpio_export() creates GPIO directory", export_success);

    // Test unexport
    std::cout << "  Unexporting GPIO " << test_gpio << "..." << std::endl;
    result = gpio::util::gpio_unexport(test_gpio);
    usleep(100000);

    bool unexport_success = !is_gpio_exported(test_gpio);
    print_result("gpio_unexport() removes GPIO directory", unexport_success);
}

// Test 2: GPIO Direction
void test_gpio_direction() {
    print_header("Test 2: GPIO Direction Setting");

    int test_gpio = LOOKUP::PIN::P7;

    // Export first
    gpio::util::gpio_export(test_gpio);
    usleep(100000);

    // Test setting direction to out
    std::cout << "  Setting direction to 'out'..." << std::endl;
    gpio::util::gpio_set_dir(test_gpio, "out");
    usleep(50000);

    std::string dir = get_gpio_direction(test_gpio);
    print_result("gpio_set_dir(out) sets direction to out", dir == "out");

    // Test setting direction to in
    std::cout << "  Setting direction to 'in'..." << std::endl;
    gpio::util::gpio_set_dir(test_gpio, "in");
    usleep(50000);

    dir = get_gpio_direction(test_gpio);
    print_result("gpio_set_dir(in) sets direction to in", dir == "in");

    // Cleanup
    gpio::util::gpio_unexport(test_gpio);
}

// Test 3: GPIO Value (LED Blink)
void test_gpio_value() {
    print_header("Test 3: GPIO Value Setting (LED Blink)");

    int test_gpio = LOOKUP::PIN::P7;

    // Setup
    gpio::util::gpio_export(test_gpio);
    usleep(100000);
    gpio::util::gpio_set_dir(test_gpio, "out");
    usleep(50000);

    // Test setting value to 1
    std::cout << "  Setting value to 1 (LED ON)..." << std::endl;
    gpio::util::gpio_set_value(test_gpio, 1);
    usleep(50000);

    int val = get_gpio_value(test_gpio);
    print_result("gpio_set_value(1) sets value to 1", val == 1);

    // Blink test
    std::cout << "  Blinking LED 3 times (watch GPIO " << test_gpio << ")..." << std::endl;
    for (int i = 0; i < 3; i++) {
        gpio::util::gpio_set_value(test_gpio, 1);
        usleep(300000);  // 300ms on
        gpio::util::gpio_set_value(test_gpio, 0);
        usleep(300000);  // 300ms off
    }
    print_result("LED blink cycle completed", true);

    // Test setting value to 0
    std::cout << "  Setting value to 0 (LED OFF)..." << std::endl;
    gpio::util::gpio_set_value(test_gpio, 0);
    usleep(50000);

    val = get_gpio_value(test_gpio);
    print_result("gpio_set_value(0) sets value to 0", val == 0);

    // Cleanup
    gpio::util::gpio_unexport(test_gpio);
}

// Test all 4 GPIOs used in the project
void test_all_project_gpios() {
    print_header("Test 4: All Project GPIOs (4 LEDs)");

    int gpios[] = {
        LOOKUP::PIN::P7,   // LED1 - 396
        LOOKUP::PIN::P13,  // LED2 - 397
        LOOKUP::PIN::P15,  // LED3 - 255
        LOOKUP::PIN::P32   // LED4 - 297
    };
    const char* names[] = {"LED1 (P7)", "LED2 (P13)", "LED3 (P15)", "LED4 (P32)"};

    // Export and setup all GPIOs
    std::cout << "  Setting up all 4 GPIOs..." << std::endl;
    for (int i = 0; i < 4; i++) {
        gpio::util::gpio_export(gpios[i]);
        usleep(50000);
        gpio::util::gpio_set_dir(gpios[i], "out");
        usleep(50000);
    }

    // Test each GPIO
    bool all_ok = true;
    for (int i = 0; i < 4; i++) {
        bool exported = is_gpio_exported(gpios[i]);
        if (!exported) all_ok = false;
        print_result(std::string(names[i]) + " exported successfully", exported);
    }

    // Sequential light test
    std::cout << std::endl << "  Running sequential LED test..." << std::endl;
    for (int i = 0; i < 4; i++) {
        std::cout << "    " << names[i] << " ON" << std::endl;
        gpio::util::gpio_set_value(gpios[i], 1);
        usleep(400000);
        gpio::util::gpio_set_value(gpios[i], 0);
    }
    print_result("Sequential LED test completed", true);

    // All on/off test
    std::cout << "  All LEDs ON..." << std::endl;
    for (int i = 0; i < 4; i++) {
        gpio::util::gpio_set_value(gpios[i], 1);
    }
    usleep(500000);

    std::cout << "  All LEDs OFF..." << std::endl;
    for (int i = 0; i < 4; i++) {
        gpio::util::gpio_set_value(gpios[i], 0);
    }
    usleep(200000);
    print_result("All LEDs on/off test completed", true);

    // Cleanup
    std::cout << "  Cleaning up..." << std::endl;
    for (int i = 0; i < 4; i++) {
        gpio::util::gpio_unexport(gpios[i]);
    }
}

void print_summary() {
    std::cout << std::endl << "================================" << std::endl;
    std::cout << "Test Summary:" << std::endl;
    std::cout << GREEN << "  Passed: " << tests_passed << RESET << std::endl;
    std::cout << RED << "  Failed: " << tests_failed << RESET << std::endl;
    std::cout << "================================" << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "GPIO Test Suite" << std::endl;
    std::cout << "===============" << std::endl;
    std::cout << "NOTE: Run with sudo for GPIO access" << std::endl;

    int test_num = 4;  // Default: run all
    if (argc > 1) {
        test_num = std::stoi(argv[1]);
    }

    switch (test_num) {
        case 1:
            test_gpio_export_unexport();
            break;
        case 2:
            test_gpio_direction();
            break;
        case 3:
            test_gpio_value();
            break;
        case 4:
        default:
            test_gpio_export_unexport();
            test_gpio_direction();
            test_gpio_value();
            test_all_project_gpios();
            break;
    }

    print_summary();

    return tests_failed > 0 ? 1 : 0;
}
