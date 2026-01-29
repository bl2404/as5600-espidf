/**
 * @file read_angle.c
 * @brief Example: Read angle from AS5600 sensor
 * 
 * This example demonstrates how to:
 * - Initialize I2C bus
 * - Initialize AS5600 sensor
 * - Read angle values continuously
 * - Display results in degrees
 * 
 * Hardware Setup:
 * - AS5600 SDA -> ESP32 GPIO 21 (or change SDA_GPIO below)
 * - AS5600 SCL -> ESP32 GPIO 22 (or change SCL_GPIO below)
 * - AS5600 VCC -> 3.3V or 5V
 * - AS5600 GND -> GND
 * - Place magnet above AS5600 sensor
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "as5600.h"

static const char *TAG = "READ_ANGLE";

// I2C Configuration
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          400000
#define SDA_GPIO                    21
#define SCL_GPIO                    22

/**
 * @brief Initialize I2C master
 */
static esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_GPIO,
        .scl_io_num = SCL_GPIO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) {
        return err;
    }
    
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

void app_main(void)
{
    ESP_LOGI(TAG, "AS5600 Read Angle Example");
    
    // Initialize I2C
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");
    
    // Initialize AS5600
    as5600_config_t as5600_config = {
        .i2c_port = I2C_MASTER_NUM,
        .dev_addr = AS5600_I2C_ADDRESS,
    };
    
    as5600_handle_t as5600;
    ESP_ERROR_CHECK(as5600_init(&as5600_config, &as5600));
    ESP_LOGI(TAG, "AS5600 initialized successfully");
    
    // Check if magnet is detected
    bool magnet_detected = false;
    esp_err_t ret = as5600_is_magnet_detected(&as5600, &magnet_detected);
    
    if (ret == ESP_OK && magnet_detected) {
        ESP_LOGI(TAG, "Magnet detected!");
    } else {
        ESP_LOGW(TAG, "No magnet detected - readings may be invalid");
    }
    
    // Main loop - read angle continuously
    while (1) {
        float degrees;
        uint16_t raw_angle;
        
        // Get angle in degrees (0-360)
        ret = as5600_get_degrees(&as5600, &degrees);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Angle: %.2f degrees", degrees);
        }
        
        // Get raw angle value (0-4095)
        ret = as5600_get_raw_angle(&as5600, &raw_angle);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Raw angle: %d", raw_angle);
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));  // Read every 500ms
    }
}
