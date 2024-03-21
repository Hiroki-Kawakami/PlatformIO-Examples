#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "iot_button.h"
#include "led_strip.h"

#define BUTTON_PIN GPIO_NUM_39
#define LED_PIN    GPIO_NUM_27

static const uint8_t led_colors[][3] = {
    { 30,  0,  0 },  // Red
    {  0, 30,  0 },  // Green
    {  0,  0, 30 },  // Blue
    { 30, 30,  0 },  // Yellow
    { 30,  0, 30 },  // Magenta
    {  0, 30, 30 },  // Cyan
    { 30, 30, 30 },  // White
};
static int led_color_index = 0;
static int blink_speed = 200;

static led_strip_handle_t led_strip;
void led_set_pixel(uint8_t r, uint8_t g, uint8_t b) {
    led_strip_set_pixel(led_strip, 0, r, g, b);
    led_strip_refresh(led_strip);
}

void button_single_clock(void *arg, void *usr_data) {
    if (++led_color_index >= sizeof(led_colors) / sizeof(led_colors[0])) {
        led_color_index = 0;
    }
}

void button_long_press_hold(void *arg, void *usr_data) {
    blink_speed = blink_speed != 200 ? 200 : 1000;
}

void app_main() {
    // Setup Button
    button_config_t gpio_btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = 1000,
        .short_press_time = 50,
        .gpio_button_config = {
            .gpio_num = BUTTON_PIN,
            .active_level = 0,
        },
    };
    button_handle_t gpio_btn = iot_button_create(&gpio_btn_cfg);
    if (gpio_btn) {
        iot_button_register_cb(gpio_btn, BUTTON_SINGLE_CLICK, button_single_clock, NULL);
        iot_button_register_cb(gpio_btn, BUTTON_LONG_PRESS_HOLD, button_long_press_hold, NULL);
    }

    // Setup LED
    led_strip_config_t strip_cfg = {
        .strip_gpio_num = LED_PIN,
        .max_leds = 1,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };
    led_strip_rmt_config_t rmt_cfg = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .mem_block_symbols = 0,
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_cfg, &rmt_cfg, &led_strip));

    // Main Loop
    while (true) {
        const uint8_t *color = led_colors[led_color_index];
        led_set_pixel(color[0], color[1], color[2]);
        vTaskDelay(blink_speed / portTICK_PERIOD_MS);
        led_set_pixel(0, 0, 0);
        vTaskDelay(blink_speed / portTICK_PERIOD_MS);
    }
}
