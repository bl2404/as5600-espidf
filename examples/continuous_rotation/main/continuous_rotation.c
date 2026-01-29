/**
 * @file continuous_rotation.c
 * @brief Example: Track continuous rotation beyond 360 degrees
 * 
 * This example demonstrates how to:
 * - Track unlimited rotations (not limited to 0-360 degrees)
 * - Count revolution numbers
 * - Calculate absolute position across multiple rotations
 * - Handle wraparound detection
 * 
 * Hardware Setup:
 * - AS5600 SDA -> ESP32 GPIO 21 (or change SDA_GPIO below)
 * - AS5600 SCL -> ESP32 GPIO 22 (or change SCL_GPIO below)
 * - AS5600 VCC -> 3.3V or 5V
 * - AS5600 GND -> GND
 * - Place magnet above AS5600 sensor and rotate it
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "as5600.h"

static const char *TAG = "CONTINUOUS_ROTATION";

// I2C Configuration
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          400000
#define SDA_GPIO                    21
#define SCL_GPIO                    22

// Rotation tracking
static int32_t revolutions = 0;      // Number of full rotations
static int32_t last_position = 0;    // Last raw position reading
static int64_t absolute_position = 0; // Total position across rotations

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

/**
 * @brief Update position and detect rotations
 */
static void update_position(uint16_t current_position)
{
    // Detect wraparound from 4095->0 (forward rotation)
    if ((last_position - current_position) > 2047) {
        revolutions++;
        ESP_LOGI(TAG, "Forward rotation detected! Total revolutions: %ld", revolutions);
    }
    // Detect wraparound from 0->4095 (backward rotation)
    else if ((last_position - current_position) < -2047) {
        revolutions--;
        ESP_LOGI(TAG, "Backward rotation detected! Total revolutions: %ld", revolutions);
    }
    
    // Calculate absolute position (12-bit resolution = 4096 steps per revolution)
    absolute_position = (int64_t)revolutions * 4096 + current_position;
    
    last_position = current_position;
}

void app_main(void)
{
    ESP_LOGI(TAG, "AS5600 Continuous Rotation Tracking Example");
    
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
    
    // Get initial position
    uint16_t initial_position;
    ret = as5600_get_position(&as5600, &initial_position);
    if (ret == ESP_OK) {
        last_position = initial_position;
        absolute_position = initial_position;
        ESP_LOGI(TAG, "Initial position: %d", initial_position);
    }
    
    ESP_LOGI(TAG, "Starting continuous rotation tracking...");
    ESP_LOGI(TAG, "Rotate the magnet and watch the position counter!");
    
    // Main loop - track rotation continuously
    while (1) {
        uint16_t current_position;
        float degrees;
        
        // Read current position
        ret = as5600_get_position(&as5600, &current_position);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read position");
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        
        // Read angle in degrees
        ret = as5600_get_degrees(&as5600, &degrees);
        
        // Update rotation tracking
        update_position(current_position);
        
        // Calculate total degrees rotated
        float total_degrees = (float)absolute_position * (360.0f / 4096.0f);
        
        // Display information
        ESP_LOGI(TAG, "Position: %5d | Revolutions: %4ld | Absolute: %8lld | Degrees: %8.2f° | Current: %.2f°",
                 current_position, 
                 revolutions, 
                 absolute_position,
                 total_degrees,
                 degrees);
        
        vTaskDelay(pdMS_TO_TICKS(100));  // Read every 100ms for smooth tracking
    }
}
