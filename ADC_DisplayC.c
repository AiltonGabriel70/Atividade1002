#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"


#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define DISPLAY_ADDR 0x3C
#define JOYSTICK_X_PIN 26
#define JOYSTICK_Y_PIN 27
#define JOYSTICK_PB 22
#define BUTTON_A 5
#define DEBOUNCE_DELAY_MS 250

// LED GPIOs
#define LED_RED 12
#define LED_GREEN 11
#define LED_BLUE 13

// PWM parameters
#define PWM_MAX 65535

// Display dimensions
#define WIDTH 128
#define HEIGHT 64
#define SQUARE_SIZE 8

volatile bool joystick_button_pressed = false;
volatile bool button_a_pressed = false;
volatile uint32_t last_joystick_time = 0;
volatile uint32_t last_button_a_time = 0;

bool pwm_enabled = true;
bool green_led_on = false;
uint8_t border_style = 0;

uint red_slice, green_slice, blue_slice;
uint red_channel, green_channel, blue_channel;

ssd1306_t ssd;

void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t now = time_us_32();
    if (events & GPIO_IRQ_EDGE_FALL) {
        if (gpio == JOYSTICK_PB) {
            if ((now - last_joystick_time) > (DEBOUNCE_DELAY_MS * 1000)) {
                joystick_button_pressed = true;
                last_joystick_time = now;
            }
        } else if (gpio == BUTTON_A) {
            if ((now - last_button_a_time) > (DEBOUNCE_DELAY_MS * 1000)) {
                button_a_pressed = true;
                last_button_a_time = now;
            }
        }
    }
}

void setup_pwm() {
    gpio_set_function(LED_RED, GPIO_FUNC_PWM);
    red_slice = pwm_gpio_to_slice_num(LED_RED);
    red_channel = pwm_gpio_to_channel(LED_RED);
    pwm_set_wrap(red_slice, PWM_MAX);
    pwm_set_chan_level(red_slice, red_channel, 0);
    pwm_set_enabled(red_slice, true);

    gpio_set_function(LED_GREEN, GPIO_FUNC_PWM);
    green_slice = pwm_gpio_to_slice_num(LED_GREEN);
    green_channel = pwm_gpio_to_channel(LED_GREEN);
    pwm_set_wrap(green_slice, PWM_MAX);
    pwm_set_chan_level(green_slice, green_channel, 0);
    pwm_set_enabled(green_slice, true);

    gpio_set_function(LED_BLUE, GPIO_FUNC_PWM);
    blue_slice = pwm_gpio_to_slice_num(LED_BLUE);
    blue_channel = pwm_gpio_to_channel(LED_BLUE);
    pwm_set_wrap(blue_slice, PWM_MAX);
    pwm_set_chan_level(blue_slice, blue_channel, 0);
    pwm_set_enabled(blue_slice, true);
}

void update_leds(int32_t adc_x, int32_t adc_y) {
    int32_t diff_x = adc_x - 2048;
    uint16_t duty_red = (abs(diff_x) * PWM_MAX) / 2048;

    int32_t diff_y = adc_y - 2048;
    uint16_t duty_blue = (abs(diff_y) * PWM_MAX) / 2048;

    if (pwm_enabled) {
        pwm_set_chan_level(red_slice, red_channel, duty_red);
        pwm_set_chan_level(blue_slice, blue_channel, duty_blue);
    } else {
        pwm_set_chan_level(red_slice, red_channel, 0);
        pwm_set_chan_level(blue_slice, blue_channel, 0);
    }

    uint16_t green_duty = green_led_on ? PWM_MAX : 0;
    pwm_set_chan_level(green_slice, green_channel, green_duty);
}

void update_display(int32_t x_pos, int32_t y_pos) {
    // Limpa o display
    ssd1306_fill(&ssd, false);
    
    // Desenha o quadrado móvel
    ssd1306_rect(&ssd, x_pos, y_pos, SQUARE_SIZE, SQUARE_SIZE, true, true);
    
    // Desenha a borda de acordo com o estilo
    switch (border_style % 4) {
        case 1:
            ssd1306_rect(&ssd, 0, 0, WIDTH, HEIGHT, true, false);
            break;
        case 2:
            ssd1306_rect(&ssd, 2, 2, WIDTH-4, HEIGHT-4, true, false);
            break;
        case 3:
            ssd1306_rect(&ssd, 4, 4, WIDTH-8, HEIGHT-8, true, false);
            break;
        default:
            break;
    }
    
    ssd1306_send_data(&ssd);
}

void handle_buttons() {
    if (joystick_button_pressed) {
        joystick_button_pressed = false;
        green_led_on = !green_led_on;
        border_style++;
    }

    if (button_a_pressed) {
        button_a_pressed = false;
        pwm_enabled = !pwm_enabled;
    }
}

int main() {
    stdio_init_all();
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);

    setup_pwm();

    gpio_init(JOYSTICK_PB);
    gpio_set_dir(JOYSTICK_PB, GPIO_IN);
    gpio_pull_up(JOYSTICK_PB);

    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_set_irq_enabled_with_callback(JOYSTICK_PB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, DISPLAY_ADDR, I2C_PORT);
    
    // Correção aplicada: Substituição do ssd1306_clear
    ssd1306_fill(&ssd, false);  // Limpa o display
    ssd1306_send_data(&ssd);     // Atualiza o display

    while (true) {
        adc_select_input(0);
        uint16_t adc_x = adc_read();
        adc_select_input(1);
        uint16_t adc_y = adc_read();

        int32_t x_pos = (adc_x * (WIDTH - SQUARE_SIZE)) / 4095;
        int32_t y_pos = (adc_y * (HEIGHT - SQUARE_SIZE)) / 4095;

        update_leds(adc_x, adc_y);
        update_display(x_pos, y_pos);
        handle_buttons();

        sleep_ms(10);
    }
}