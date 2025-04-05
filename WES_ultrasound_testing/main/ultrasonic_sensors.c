#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define TRIG_GPIO GPIO_NUM_33
#define ECHO_GPIO GPIO_NUM_25

void hcsr04_init()
{
    gpio_set_direction(TRIG_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_GPIO, GPIO_MODE_INPUT);
    gpio_set_level(TRIG_GPIO, 0);
}

float hcsr04_get_distance_cm()
{
    int64_t start_time, end_time;

    // Send 10us pulse
    gpio_set_level(TRIG_GPIO, 1);
    ets_delay_us(10);
    gpio_set_level(TRIG_GPIO, 0);

    // Wait for echo HIGH
    while (gpio_get_level(ECHO_GPIO) == 0)
    {
    }
    start_time = esp_timer_get_time();

    // Wait for echo LOW
    while (gpio_get_level(ECHO_GPIO) == 1)
    {
    }
    end_time = esp_timer_get_time();

    int64_t duration = end_time - start_time;

    // Convert to cm
    float distance = duration / 58.0;
    return distance;
}

void app_main()
{
    hcsr04_init();

    while (1)
    {
        float distance = hcsr04_get_distance_cm();
        printf("Distance: %.2f cm\n", distance);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
