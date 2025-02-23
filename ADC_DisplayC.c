/***********************************************************
 * Sistema de Controle com Joystick e Display OLED
 * Versão Final com Centralização Corrigida
 ***********************************************************/

 #include <stdio.h>
 #include <stdlib.h>
 #include "pico/stdlib.h"
 #include "hardware/adc.h"
 #include "hardware/pwm.h"
 #include "hardware/i2c.h"
 #include "lib/ssd1306.h"
 
 // Configurações de Hardware
 #define I2C_PORT i2c1
 #define I2C_SDA 14
 #define I2C_SCL 15
 #define DISPLAY_ADDR 0x3C
 #define JOYSTICK_X_PIN 26
 #define JOYSTICK_Y_PIN 27
 #define JOYSTICK_PB 22
 #define BUTTON_A 5
 #define DEBOUNCE_DELAY_MS 250
 
 // Configurações dos LEDs
 #define LED_RED 12
 #define LED_GREEN 11
 #define LED_BLUE 13
 #define PWM_MAX 65535
 
 // Configurações do Display
 #define WIDTH 128
 #define HEIGHT 64
 #define SQUARE_SIZE 8
 #define BORDER_WIDTH 2
 #define CENTER_X ((WIDTH - SQUARE_SIZE) / 2)    // 60
 #define CENTER_Y ((HEIGHT - SQUARE_SIZE) / 2)   // 28
 
 // Variáveis globais
 volatile bool joystick_button_pressed = false;
 volatile bool button_a_pressed = false;
 volatile uint32_t last_joystick_time = 0;
 volatile uint32_t last_button_a_time = 0;
 
 bool pwm_enabled = true;
 bool green_led_on = false;
 
 // Variáveis PWM
 uint red_slice, green_slice, blue_slice;
 uint red_channel, green_channel, blue_channel;
 
 ssd1306_t ssd;
 
 // Protótipos de função
 void gpio_irq_handler(uint gpio, uint32_t events);
 void setup_pwm();
 void update_leds(int32_t adc_x, int32_t adc_y);
 void update_display(int32_t x_pos, int32_t y_pos);
 void handle_buttons();
 int32_t map_value(int32_t value, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max);
 
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
     // Configuração dos LEDs PWM
     const uint leds[] = {LED_RED, LED_GREEN, LED_BLUE};
     for (int i = 0; i < 3; i++) {
         gpio_set_function(leds[i], GPIO_FUNC_PWM);
         uint slice = pwm_gpio_to_slice_num(leds[i]);
         pwm_set_wrap(slice, PWM_MAX);
         pwm_set_chan_level(slice, pwm_gpio_to_channel(leds[i]), 0);
         pwm_set_enabled(slice, true);
     }
 }
 
 int32_t map_value(int32_t value, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
     return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
 }
 
 void update_leds(int32_t adc_x, int32_t adc_y) {
     // Cálculo da intensidade dos LEDs
     int32_t diff_x = adc_x - 2048;
     uint16_t duty_red = (abs(diff_x) * PWM_MAX) / 2048;
     
     int32_t diff_y = adc_y - 2048;
     uint16_t duty_blue = (abs(diff_y) * PWM_MAX) / 2048;
 
     // Aplica configurações PWM
     if (pwm_enabled) {
         pwm_set_chan_level(red_slice, red_channel, duty_red);
         pwm_set_chan_level(blue_slice, blue_channel, duty_blue);
     } else {
         pwm_set_chan_level(red_slice, red_channel, 0);
         pwm_set_chan_level(blue_slice, blue_channel, 0);
     }
     pwm_set_chan_level(green_slice, green_channel, green_led_on ? PWM_MAX : 0);
 }
 
 void update_display(int32_t x_pos, int32_t y_pos) {
     ssd1306_fill(&ssd, false);
     
     // Desenha borda dupla
     ssd1306_rect(&ssd, 0, 0, WIDTH, HEIGHT, true, false);
     ssd1306_rect(&ssd, 
                 BORDER_WIDTH, 
                 BORDER_WIDTH, 
                 WIDTH - (BORDER_WIDTH*2), 
                 HEIGHT - (BORDER_WIDTH*2), 
                 true, false);
     
     // Desenha quadrado centralizado
     ssd1306_rect(&ssd, x_pos, y_pos, SQUARE_SIZE, SQUARE_SIZE, true, true);
     
     ssd1306_send_data(&ssd);
 }
 
 void handle_buttons() {
     if (joystick_button_pressed) {
         joystick_button_pressed = false;
         green_led_on = !green_led_on;
     }
 
     if (button_a_pressed) {
         button_a_pressed = false;
         pwm_enabled = !pwm_enabled;
     }
 }
 
 int main() {
     stdio_init_all();
     adc_init();
     
     // Configuração do ADC
     adc_gpio_init(JOYSTICK_X_PIN);
     adc_gpio_init(JOYSTICK_Y_PIN);
     
     setup_pwm();
 
     // Configuração dos botões
     const uint buttons[] = {JOYSTICK_PB, BUTTON_A};
     for (int i = 0; i < 2; i++) {
         gpio_init(buttons[i]);
         gpio_set_dir(buttons[i], GPIO_IN);
         gpio_pull_up(buttons[i]);
     }
 
     // Configuração de interrupções
     gpio_set_irq_enabled_with_callback(JOYSTICK_PB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
     gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
 
     // Inicialização do display
     i2c_init(I2C_PORT, 400 * 1000);
     gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
     gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
     gpio_pull_up(I2C_SDA);
     gpio_pull_up(I2C_SCL);
 
     ssd1306_init(&ssd, WIDTH, HEIGHT, false, DISPLAY_ADDR, I2C_PORT);
     ssd1306_fill(&ssd, false);
     ssd1306_send_data(&ssd);
 
     // Configuração dos canais PWM
     red_slice = pwm_gpio_to_slice_num(LED_RED);
     red_channel = pwm_gpio_to_channel(LED_RED);
     green_slice = pwm_gpio_to_slice_num(LED_GREEN);
     green_channel = pwm_gpio_to_channel(LED_GREEN);
     blue_slice = pwm_gpio_to_slice_num(LED_BLUE);
     blue_channel = pwm_gpio_to_channel(LED_BLUE);
 
     while (true) {
         // Leitura dos eixos do joystick
         adc_select_input(0);
         uint16_t adc_x = adc_read();
         adc_select_input(1);
         uint16_t adc_y = adc_read();
 
         // Mapeamento preciso das posições
         int32_t x_pos = map_value(adc_x, 0, 2048, 0, WIDTH - SQUARE_SIZE);
         int32_t y_pos = map_value(4095 - adc_y, 0, 1000, 0, HEIGHT - SQUARE_SIZE); 
 
         // Ajuste fino de centralização
         const int32_t center_offset = 2; // Compensação para calibração precisa
         x_pos = (x_pos + CENTER_X - center_offset) / 2;
         y_pos = (y_pos + CENTER_Y - center_offset) / 2;
 
         // Garante que está dentro dos limites
         x_pos = (x_pos < 0) ? 0 : (x_pos > WIDTH - SQUARE_SIZE) ? WIDTH - SQUARE_SIZE : x_pos;
         y_pos = (y_pos < 0) ? 0 : (y_pos > HEIGHT - SQUARE_SIZE) ? HEIGHT - SQUARE_SIZE : y_pos;
 
         update_leds(adc_x, adc_y);
         update_display(x_pos, y_pos);
         handle_buttons();
 
         sleep_ms(10);
     }
 }