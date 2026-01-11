/**
 * LED Controller Test Program
 * Tests the LED_CTRL class functionality
 *
 * Usage: sudo ./test_led [test_number]
 *   1 - Test single LED enable/disable
 *   2 - Test all 4 LEDs
 *   3 - Test LED patterns (blink, chase, etc.)
 *   4 - Interactive LED test
 */

#include <iostream>
#include <memory>
#include <unistd.h>
#include <termios.h>
#include "../gpio_LED_CTRL.hpp"
#include "../PIN_LOOKUP.hpp"

// Color codes
#define GREEN "\033[32m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define RESET "\033[0m"

// LED instances
std::shared_ptr<gpio::LED_CTRL> LED1;
std::shared_ptr<gpio::LED_CTRL> LED2;
std::shared_ptr<gpio::LED_CTRL> LED3;
std::shared_ptr<gpio::LED_CTRL> LED4;

void init_leds() {
    LED1 = std::make_shared<gpio::LED_CTRL>(LOOKUP::PIN::P7);
    LED2 = std::make_shared<gpio::LED_CTRL>(LOOKUP::PIN::P13);
    LED3 = std::make_shared<gpio::LED_CTRL>(LOOKUP::PIN::P15);
    LED4 = std::make_shared<gpio::LED_CTRL>(LOOKUP::PIN::P32);
}

void cleanup_leds() {
    LED1->disable();
    LED2->disable();
    LED3->disable();
    LED4->disable();
}

void print_header(const std::string& header) {
    std::cout << std::endl << YELLOW << "=== " << header << " ===" << RESET << std::endl;
}

// Test 1: Single LED
void test_single_led() {
    print_header("Test 1: Single LED Enable/Disable");

    std::cout << "  Testing LED1 (GPIO " << LOOKUP::PIN::P7 << ")..." << std::endl;

    std::cout << "    LED1 enable (should light up)..." << std::endl;
    LED1->enable();
    usleep(1000000);  // 1 second

    std::cout << "    LED1 disable (should turn off)..." << std::endl;
    LED1->disable();
    usleep(500000);

    std::cout << GREEN << "  [PASS] Single LED test completed" << RESET << std::endl;
}

// Test 2: All 4 LEDs
void test_all_leds() {
    print_header("Test 2: All 4 LEDs");

    std::shared_ptr<gpio::LED_CTRL> leds[] = {LED1, LED2, LED3, LED4};
    const char* names[] = {"LED1", "LED2", "LED3", "LED4"};

    // Enable one by one
    std::cout << "  Enabling LEDs one by one..." << std::endl;
    for (int i = 0; i < 4; i++) {
        std::cout << "    " << names[i] << " ON" << std::endl;
        leds[i]->enable();
        usleep(400000);
    }

    std::cout << "  All LEDs should be ON now. Waiting 1 second..." << std::endl;
    usleep(1000000);

    // Disable one by one
    std::cout << "  Disabling LEDs one by one..." << std::endl;
    for (int i = 0; i < 4; i++) {
        std::cout << "    " << names[i] << " OFF" << std::endl;
        leds[i]->disable();
        usleep(400000);
    }

    std::cout << GREEN << "  [PASS] All 4 LEDs test completed" << RESET << std::endl;
}

// Test 3: LED Patterns
void test_led_patterns() {
    print_header("Test 3: LED Patterns");

    std::shared_ptr<gpio::LED_CTRL> leds[] = {LED1, LED2, LED3, LED4};

    // Pattern 1: Chase
    std::cout << "  Pattern 1: Chase (3 cycles)..." << std::endl;
    for (int cycle = 0; cycle < 3; cycle++) {
        for (int i = 0; i < 4; i++) {
            leds[i]->enable();
            usleep(150000);
            leds[i]->disable();
        }
    }

    // Pattern 2: Bounce
    std::cout << "  Pattern 2: Bounce (3 cycles)..." << std::endl;
    for (int cycle = 0; cycle < 3; cycle++) {
        for (int i = 0; i < 4; i++) {
            leds[i]->enable();
            usleep(150000);
            leds[i]->disable();
        }
        for (int i = 2; i >= 1; i--) {
            leds[i]->enable();
            usleep(150000);
            leds[i]->disable();
        }
    }

    // Pattern 3: Blink all
    std::cout << "  Pattern 3: Blink all (5 times)..." << std::endl;
    for (int i = 0; i < 5; i++) {
        for (auto& led : leds) led->enable();
        usleep(200000);
        for (auto& led : leds) led->disable();
        usleep(200000);
    }

    // Pattern 4: Alternating
    std::cout << "  Pattern 4: Alternating (5 times)..." << std::endl;
    for (int i = 0; i < 5; i++) {
        LED1->enable(); LED3->enable();
        LED2->disable(); LED4->disable();
        usleep(300000);
        LED1->disable(); LED3->disable();
        LED2->enable(); LED4->enable();
        usleep(300000);
    }
    LED2->disable(); LED4->disable();

    // Pattern 5: Binary count
    std::cout << "  Pattern 5: Binary count (0-15)..." << std::endl;
    for (int num = 0; num < 16; num++) {
        (num & 1) ? LED1->enable() : LED1->disable();
        (num & 2) ? LED2->enable() : LED2->disable();
        (num & 4) ? LED3->enable() : LED3->disable();
        (num & 8) ? LED4->enable() : LED4->disable();
        usleep(300000);
    }

    cleanup_leds();
    std::cout << GREEN << "  [PASS] LED patterns test completed" << RESET << std::endl;
}

// Test 4: Interactive
void test_interactive() {
    print_header("Test 4: Interactive LED Control");

    std::cout << CYAN << "  Controls:" << RESET << std::endl;
    std::cout << "    1-4: Toggle LED 1-4" << std::endl;
    std::cout << "    a: All ON" << std::endl;
    std::cout << "    o: All OFF" << std::endl;
    std::cout << "    b: Blink all" << std::endl;
    std::cout << "    q: Quit" << std::endl;
    std::cout << std::endl;

    // Set terminal to raw mode for single char input
    struct termios old_tio, new_tio;
    tcgetattr(STDIN_FILENO, &old_tio);
    new_tio = old_tio;
    new_tio.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

    bool led_states[] = {false, false, false, false};
    std::shared_ptr<gpio::LED_CTRL> leds[] = {LED1, LED2, LED3, LED4};

    bool running = true;
    while (running) {
        char c;
        read(STDIN_FILENO, &c, 1);

        switch (c) {
            case '1':
            case '2':
            case '3':
            case '4': {
                int idx = c - '1';
                led_states[idx] = !led_states[idx];
                if (led_states[idx]) {
                    leds[idx]->enable();
                    std::cout << "LED" << (idx+1) << " ON" << std::endl;
                } else {
                    leds[idx]->disable();
                    std::cout << "LED" << (idx+1) << " OFF" << std::endl;
                }
                break;
            }
            case 'a':
            case 'A':
                for (int i = 0; i < 4; i++) {
                    leds[i]->enable();
                    led_states[i] = true;
                }
                std::cout << "All LEDs ON" << std::endl;
                break;
            case 'o':
            case 'O':
                for (int i = 0; i < 4; i++) {
                    leds[i]->disable();
                    led_states[i] = false;
                }
                std::cout << "All LEDs OFF" << std::endl;
                break;
            case 'b':
            case 'B':
                std::cout << "Blinking..." << std::endl;
                for (int i = 0; i < 5; i++) {
                    for (auto& led : leds) led->enable();
                    usleep(200000);
                    for (auto& led : leds) led->disable();
                    usleep(200000);
                }
                // Restore previous state
                for (int i = 0; i < 4; i++) {
                    if (led_states[i]) leds[i]->enable();
                }
                break;
            case 'q':
            case 'Q':
                running = false;
                break;
        }
    }

    // Restore terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    cleanup_leds();
    std::cout << GREEN << "  Interactive test ended" << RESET << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "LED Controller Test Suite" << std::endl;
    std::cout << "=========================" << std::endl;
    std::cout << "NOTE: Run with sudo for GPIO access" << std::endl;

    init_leds();

    int test_num = 0;
    if (argc > 1) {
        test_num = std::stoi(argv[1]);
    }

    if (test_num == 0) {
        std::cout << std::endl << "Select test:" << std::endl;
        std::cout << "  1 - Single LED test" << std::endl;
        std::cout << "  2 - All 4 LEDs test" << std::endl;
        std::cout << "  3 - LED patterns test" << std::endl;
        std::cout << "  4 - Interactive test" << std::endl;
        std::cout << "  5 - Run all (except interactive)" << std::endl;
        std::cout << "Choice: ";
        std::cin >> test_num;
    }

    switch (test_num) {
        case 1:
            test_single_led();
            break;
        case 2:
            test_all_leds();
            break;
        case 3:
            test_led_patterns();
            break;
        case 4:
            test_interactive();
            break;
        case 5:
        default:
            test_single_led();
            test_all_leds();
            test_led_patterns();
            break;
    }

    cleanup_leds();
    std::cout << std::endl << "LED test completed." << std::endl;
    return 0;
}
