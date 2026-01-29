/**
 * @file read_magnet.c
 * @brief Example: Check magnet detection status on AS5600 sensor
 * 
 * This example demonstrates how to:
 * - Check if magnet is detected
 * - Check if magnet is too strong
 * - Check if magnet is too weak
 * - Read AGC (Automatic Gain Control) value
 * - Read magnitude value
 * 
 * Hardware Setup:
 * - AS5600 SDA -> ESP32 GPIO 21 (or change SDA_GPIO below)
 * - AS5600 SCL -> ESP32 GPIO 22 (or change SCL_GPIO below)
 * - AS5600 VCC -> 3.3V or 5V
 * - AS5600 GND -> GND
 * - Place magnet above AS5600 sensor (try different distances)
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "as5600.h"

static const char *TAG = "READ_MAGNET";

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
    ESP_LOGI(TAG, "AS5600 Magnet Status Example");
    
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
    
    ESP_LOGI(TAG, "Starting magnet status monitoring...");
    ESP_LOGI(TAG, "Try moving the magnet closer or farther from the sensor");
    
    // Main loop - check magnet status continuously
    while (1) {
        bool magnet_detected = false;
        bool magnet_too_strong = false;
        bool magnet_too_weak = false;
        uint8_t gain = 0;
        uint16_t magnitude = 0;
        uint8_t status = 0;
        
        // Read all status information
        esp_err_t ret;
        
        ret = as5600_is_magnet_detected(&as5600, &magnet_detected);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read magnet detected status");
        }
        
        ret = as5600_is_magnet_too_strong(&as5600, &magnet_too_strong);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read magnet too strong status");
        }
        
        ret = as5600_is_magnet_too_weak(&as5600, &magnet_too_weak);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read magnet too weak status");
        }
        
        ret = as5600_get_gain(&as5600, &gain);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read AGC gain");
        }
        
        ret = as5600_get_magnitude(&as5600, &magnitude);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read magnitude");
        }
        
        ret = as5600_get_status(&as5600, &status);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read status register");
        }
        
        // Display status
        ESP_LOGI(TAG, "====================================");
        ESP_LOGI(TAG, "Magnet Status:");
        ESP_LOGI(TAG, "  Detected:    %s", magnet_detected ? "YES" : "NO");
        ESP_LOGI(TAG, "  Too Strong:  %s", magnet_too_strong ? "YES" : "NO");
        ESP_LOGI(TAG, "  Too Weak:    %s", magnet_too_weak ? "YES" : "NO");
        ESP_LOGI(TAG, "  AGC Gain:    %d", gain);
        ESP_LOGI(TAG, "  Magnitude:   %d", magnitude);
        ESP_LOGI(TAG, "  Status Reg:  0x%02X", status);
        
        // Provide guidance
        if (magnet_detected && !magnet_too_strong && !magnet_too_weak) {
            ESP_LOGI(TAG, "✓ Magnet position is GOOD!");
        } else if (magnet_too_strong) {
            ESP_LOGW(TAG, "⚠ Magnet is TOO CLOSE - move it away");
        } else if (magnet_too_weak) {
            ESP_LOGW(TAG, "⚠ Magnet is TOO FAR - move it closer");
        } else if (!magnet_detected) {
            ESP_LOGE(TAG, "✗ NO MAGNET DETECTED");
        }
        
        ESP_LOGI(TAG, "====================================");
        
        vTaskDelay(pdMS_TO_TICKS(1000));  // Check every second
    }
}
