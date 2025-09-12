#include <avr/io.h>
#include <Arduino_FreeRTOS.h>
#include "task.h"
#include "semphr.h"

volatile int sec = 0;
volatile int set = 0;
volatile long timeLeft[2][2] = {{1,0},{1,0}};
volatile uint8_t currentPlayer = 0;
volatile uint8_t pause = 0;
volatile uint8_t flashing = 0;

void initPorts();
uint8_t readKeypad();
void displayDigit(uint8_t digit, uint8_t position, uint8_t point);

// ------ Tasky ------

void taskKeypad(void *pvParameters) {
    uint8_t key, old_key = 0;
    uint8_t number = 0;
    int old_value = 0;
    for(;;) {
        key = readKeypad();

        // --- set = 0 logic ---
        if (!set) {
            if (key == 10 && old_key != 10) currentPlayer = 1 - currentPlayer;
            if (key == 3 && old_key != 3) pause = 1 - pause;
            if (key >= 1 && key <= 9) {
              if (key == 3 && old_key != 3) { pause = 1 - pause; old_key = 3; } 
              if (key == 4 && old_key != 4) { 
              if (key == 3 && old_key != 3)
        			{
        				pause = 1 - pause;
        				old_key = 3;
        			}
        			if (key == 4 && old_key != 4)
        			{
        				if (timeLeft[currentPlayer][0]+1 < 99)
        				timeLeft[currentPlayer][0] += 1;
        				old_key = 4;
        			}
        			if (key ==5 && old_key != 5)
        			{
        				if (timeLeft[currentPlayer][1] + 10000 <= 59000)
        				{
        					timeLeft[currentPlayer][1] += 10000;
        				}	else
        				{
        					timeLeft[currentPlayer][0] += 1;
        					timeLeft[currentPlayer][1] = 0;
        				}
        				old_key = 5;
        			}
        			if (key == 6 && old_key != 6)
        			{
        				if (timeLeft[currentPlayer][1]+1000 <= 59000)
        				{
        					timeLeft[currentPlayer][1] += 1000;
        				}	else
        				{
        					timeLeft[currentPlayer][0] += 1;
        					timeLeft[currentPlayer][1] = 0;
        				}
        				old_key = 6;
        			}
        			if (key == 7 && old_key != 7)
        			{
        				if (timeLeft[currentPlayer][0] > 0)
        					timeLeft[currentPlayer][0] -= 1;
        				old_key = 7;
        			}
        			if (key == 8 && old_key != 8)
        			{
        				if (timeLeft[currentPlayer][1]-10000 > 0)
        				{
        					timeLeft[currentPlayer][1] -= 10000;
        				}	else if (timeLeft[currentPlayer][0] > 0)
        				{
        					timeLeft[currentPlayer][0] -= 1;
        					timeLeft[currentPlayer][1] = 60000;
        				}
        				old_key = 8;
        			}
        			if (key == 9 && old_key != 9)
        			{
        				if (timeLeft[currentPlayer][1]-1000 > 0)
        				{
        					timeLeft[currentPlayer][1] -= 1000;
        				}	else if (timeLeft[currentPlayer][0] > 0)
        				{
        					timeLeft[currentPlayer][0] -= 1;
        					timeLeft[currentPlayer][1] = 60000;
        				}
        				old_key = 9;
        			}
        			if (key == 2 && old_key != 2)
        			{
        				timeLeft[currentPlayer][1] = 0;
        				old_key = 2;
        			}
        			if (key == 1 && old_key != 1)
        			{
        				timeLeft[currentPlayer][0] = 0;
        				old_key = 1;
        			}
              }
            if (key == 15 && old_key != 15) {
                set = 1;
                flashing = 1;
                old_value = 0;
            }
            old_key = key;
        } else {
            // --- set = 1 logic ---
            if (old_key != key) {
                if (key == 15) {
                    set = 0;
                    flashing = 0;
                    number = 0;
                }
                if (key == 14) {
                    timeLeft[currentPlayer][0] = timeLeft[1-currentPlayer][0];
                    timeLeft[currentPlayer][1] = timeLeft[1-currentPlayer][1];
                }
                if (key == 10) currentPlayer = 1 - currentPlayer;

                if (key >= 0 && key <= 9) {
                    number++;
                    switch(number) {
                        case 1:
                            timeLeft[currentPlayer][0] = key;
                            old_value = key;
                            break;
                        case 2:
                            timeLeft[currentPlayer][0] = key + old_value*10;
                            old_value = 0;
                            break;
                        case 3:
                            timeLeft[currentPlayer][1] = key*1000;
                            old_value = key;
                            break;
                        case 4:
                            timeLeft[currentPlayer][1] = key*1000;
                            number = 0;
                            set = 0;
                            flashing = 0;
                            break;
                    }
                }
                old_key = key;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // odczyt klawiatury co 50ms
    }
}

// Task wyświetlacza 7-segmentowego
void taskDisplay(void *pvParameters) {
    int blink = 0;
    for(;;) {
        // Miganie przy flashingu
        if (flashing) {
            blink++;
            if (blink >= 50) blink = 0;
            if (blink < 25) DDRE = 0x00; // segmenty wyłączone
            else DDRE = 0xFF;            // segmenty włączone
        }

        int displaySec = timeLeft[currentPlayer][1]/1000;
        displayDigit(displaySec % 10, 1, 0);
        displayDigit(displaySec / 10, 2, 0);

        int displayMin = timeLeft[currentPlayer][0];
        displayDigit(displayMin % 10, 4, 1);
        displayDigit(displayMin / 10, 8, 0);

        vTaskDelay(pdMS_TO_TICKS(20)); // refresh co 20 ms
    }
}

// Task odliczania czasu gracza
void taskTimer(void *pvParameters) {
    for(;;) {
        if (!pause && !set) {
            if (timeLeft[currentPlayer][1] >= 1000) {
                timeLeft[currentPlayer][1] -= 1000;
            } else if (timeLeft[currentPlayer][0] > 0) {
                timeLeft[currentPlayer][0] -= 1;
                timeLeft[currentPlayer][1] = 59000;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // co 1s
    }
}

// ------ Main ------
int main(void) {
    initPorts();

    xTaskCreate(taskKeypad, "Keypad", 256, NULL, 2, NULL);
    xTaskCreate(taskDisplay, "Display", 256, NULL, 1, NULL);
    xTaskCreate(taskTimer, "Timer", 256, NULL, 3, NULL);

    vTaskStartScheduler(); // uruchom RTOS

    for(;;);
}
