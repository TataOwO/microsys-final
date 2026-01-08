#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>

using namespace std;

#define DEVICE_PATH "/dev/demo"

volatile bool is_calculating = true; 
int counter = 0;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t led_mutex = PTHREAD_MUTEX_INITIALIZER;  
int fd;  

void control_led(const char* cmd) {
    pthread_mutex_lock(&led_mutex);
    write(fd, cmd, strlen(cmd));
    pthread_mutex_unlock(&led_mutex);
}

// ============================================
// task 1 Data Parallelism
// ============================================

// Worker Thread 1:  1 ~ 25,000,000
void* thread_sum1(void* arg) {
    long long local_sum = 0;
    for (long long i = 1; i <= 25000000; ++i) {
        local_sum += i;
        
        // pause 1000 times * 10ms = 10000ms = 10秒
        if (i % 25000 == 0) {
            usleep(10000); 
        }
    }

    pthread_exit((void*)local_sum);
}

// Worker Thread 2:  25,000,001 ~ 50,000,000
void* thread_sum2(void* arg) {
    long long local_sum = 0;
    for (long long i = 25000001; i <= 50000000; ++i) {
        local_sum += i;

        if (i % 25000 == 0) {
            usleep(10000);
        }
    }

    pthread_exit((void*)local_sum);
}

// Monitor Thread (T3): Heartbeat - LED 1 
void* thread_monitor(void* arg) {
    while (is_calculating) {
        control_led("LED1on");
        // 0.5 sec (50 * 10ms)
        for (int i = 0; i < 50 && is_calculating; ++i) {
            usleep(10000);  // 10ms
        }
        
        if (!is_calculating) break;
        
        control_led("LED1off");
        // 0.5 sec (50 * 10ms)
        for (int i = 0; i < 50 && is_calculating; ++i) {
            usleep(10000);  // 10ms
        }
    }
    control_led("LED1on");
    pthread_exit(NULL);
}

// ============================================
// task 2
// ============================================

void* thread_increment(void* arg) {
    // int thread_id = *(int*)arg; 
    
    for (int i = 0; i < 10000; ++i) {
        pthread_mutex_lock(&count_mutex);
        
        // === Critical Section  ===
        // LED 2 turn on
        write(fd, "LED2on", 6);
        
        // delay 
        usleep(100); 
        
        counter++;
        
        // LED 2 turn off
        write(fd, "LED2off", 7);
        // === Critical Section  ===
        
        pthread_mutex_unlock(&count_mutex);
    }
    
    pthread_exit(NULL);
}

// ============================================
// main
// ============================================

int main() {
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("can not open /dev/demo，use dmesg check ur driver");
        return -1;
    }
    
    // initialize LED status
    control_led("LED1off");
    control_led("LED2off");

    cout << "========================================" << endl;
    cout << "Starting Item 1: Parallel Summation & Heartbeat" << endl;
    cout << "(Will run for approx. 10 seconds to show LED blinking)" << endl;
    cout << "========================================" << endl;
    
    pthread_t t1, t2, t3;
    void* result1;
    void* result2;
    
    // Monitor Thread (T3)
    pthread_create(&t3, NULL, thread_monitor, NULL);
    
    // Worker Threads (T1, T2)
    pthread_create(&t1, NULL, thread_sum1, NULL);
    pthread_create(&t2, NULL, thread_sum2, NULL);
    
    pthread_join(t1, &result1);
    pthread_join(t2, &result2);
    
    // T3 turn off
    is_calculating = false;
    pthread_join(t3, NULL);
    
    long long total_sum = (long long)(intptr_t)result1 + (long long)(intptr_t)result2;
    
    cout << "T1 Result (1~25000000): " << (long long)(intptr_t)result1 << endl;
    cout << "T2 Result (25000001~50000000): " << (long long)(intptr_t)result2 << endl;
    cout << "Total Sum: " << total_sum << endl;
    cout << "Expected:  1250000025000000" << endl;
    cout << "LED 1 is now Always ON (Calculation Complete)" << endl;
    cout << endl;

    sleep(2); 

    // ========== task 2 ==========
    cout << "========================================" << endl;
    cout << "Starting Item 2: Mutex Resource Protection" << endl;
    cout << "========================================" << endl;
    
    pthread_t t4, t5;
    int id1 = 1, id2 = 2;
    
    // reset counter
    counter = 0;
    
    cout << "Entering Critical Section..." << endl;
    
    pthread_create(&t4, NULL, thread_increment, &id1);
    pthread_create(&t5, NULL, thread_increment, &id2);
    
    pthread_join(t4, NULL);
    pthread_join(t5, NULL);
    
    cout << "Leaving Critical Section..." << endl;
    cout << "Final Counter: " << counter << " (Expected: 20000)" << endl;
    
    if (counter == 20000) {
        cout << "Mutex protection successful!" << endl;
    } else {
        cout << "WARNING: Counter mismatch! Race condition detected." << endl;
    }

    cout << endl;
    cout << "========================================" << endl;
    cout << "Experiment Complete!" << endl;
    cout << "========================================" << endl;
    
    // turn off all of LED
    control_led("LED1off");
    control_led("LED2off");
    
    pthread_mutex_destroy(&count_mutex);
    pthread_mutex_destroy(&led_mutex);
    
    close(fd);
    return 0;
}
