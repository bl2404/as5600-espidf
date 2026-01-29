/**
 * @file test_all_features.c
 * @brief Comprehensive test of all AS5600 features
 * 
 * This test application exercises all functions of the AS5600 library:
 * - I2C communication
 * - Angle reading (raw, scaled, degrees)
 * - Magnet detection
 * - Status checking
 * - Configuration changes
 * - AGC and magnitude reading
 * 
 * Use this to verify your hardware setup and library installation.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "as5600.h"

static const char *TAG = "AS5600_TEST";

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

/**
 * @brief Test basic reading functions
 */
static void test_reading_functions(as5600_handle_t *as5600)
{
    ESP_LOGI(TAG, "\n========== Testing Reading Functions ==========");
    
    uint16_t raw_angle, angle, position;
    float degrees;
    
    // Test raw angle
    if (as5600_get_raw_angle(as5600, &raw_angle) == ESP_OK) {
        ESP_LOGI(TAG, "✓ Raw Angle: %d (0-4095)", raw_angle);
    } else {
        ESP_LOGE(TAG, "✗ Failed to read raw angle");
    }
    
    // Test scaled angle
    if (as5600_get_angle(as5600, &angle) == ESP_OK) {
        ESP_LOGI(TAG, "✓ Angle: %d (0-4095)", angle);
    } else {
        ESP_LOGE(TAG, "✗ Failed to read angle");
    }
    
    // Test degrees
    if (as5600_get_degrees(as5600, &degrees) == ESP_OK) {
        ESP_LOGI(TAG, "✓ Degrees: %.2f° (0-360)", degrees);
    } else {
        ESP_LOGE(TAG, "✗ Failed to read degrees");
    }
    
    // Test position (alias for raw angle)
    if (as5600_get_position(as5600, &position) == ESP_OK) {
        ESP_LOGI(TAG, "✓ Position: %d (should match raw angle)", position);
    } else {
        ESP_LOGE(TAG, "✗ Failed to read position");
    }
}

/**
 * @brief Test magnet detection functions
 */
static void test_magnet_detection(as5600_handle_t *as5600)
{
    ESP_LOGI(TAG, "\n========== Testing Magnet Detection ==========");
    
    bool detected, too_strong, too_weak;
    uint8_t status;
    
    if (as5600_is_magnet_detected(as5600, &detected) == ESP_OK) {
        ESP_LOGI(TAG, "✓ Magnet Detected: %s", detected ? "YES" : "NO");
    } else {
        ESP_LOGE(TAG, "✗ Failed to check magnet detection");
    }
    
    if (as5600_is_magnet_too_strong(as5600, &too_strong) == ESP_OK) {
        ESP_LOGI(TAG, "✓ Magnet Too Strong: %s", too_strong ? "YES" : "NO");
    } else {
        ESP_LOGE(TAG, "✗ Failed to check magnet strength");
    }
    
    if (as5600_is_magnet_too_weak(as5600, &too_weak) == ESP_OK) {
        ESP_LOGI(TAG, "✓ Magnet Too Weak: %s", too_weak ? "YES" : "NO");
    } else {
        ESP_LOGE(TAG, "✗ Failed to check magnet weakness");
    }
    
    if (as5600_get_status(as5600, &status) == ESP_OK) {
        ESP_LOGI(TAG, "✓ Status Register: 0x%02X", status);
    } else {
        ESP_LOGE(TAG, "✗ Failed to read status register");
    }
    
    // Provide feedback
    if (detected && !too_strong && !too_weak) {
        ESP_LOGI(TAG, "✓ MAGNET POSITION IS PERFECT!");
    } else if (too_strong) {
        ESP_LOGW(TAG, "⚠ Move magnet farther away");
    } else if (too_weak) {
        ESP_LOGW(TAG, "⚠ Move magnet closer");
    } else if (!detected) {
        ESP_LOGE(TAG, "✗ NO MAGNET - place magnet above sensor!");
    }
}

/**
 * @brief Test AGC and magnitude
 */
static void test_agc_magnitude(as5600_handle_t *as5600)
{
    ESP_LOGI(TAG, "\n========== Testing AGC and Magnitude ==========");
    
    uint8_t gain;
    uint16_t magnitude;
    
    if (as5600_get_gain(as5600, &gain) == ESP_OK) {
        ESP_LOGI(TAG, "✓ AGC Gain: %d", gain);
        if (gain < 255) {
            ESP_LOGI(TAG, "  AGC is active (gain < 255)");
        }
    } else {
        ESP_LOGE(TAG, "✗ Failed to read AGC gain");
    }
    
    if (as5600_get_magnitude(as5600, &magnitude) == ESP_OK) {
        ESP_LOGI(TAG, "✓ Magnitude: %d", magnitude);
    } else {
        ESP_LOGE(TAG, "✗ Failed to read magnitude");
    }
}

/**
 * @brief Test configuration functions
 */
static void test_configuration(as5600_handle_t *as5600)
{
    ESP_LOGI(TAG, "\n========== Testing Configuration Functions ==========");
    
    // Test power mode (set to normal, which should already be default)
    if (as5600_set_power_mode(as5600, AS5600_POWER_MODE_NORM) == ESP_OK) {
        ESP_LOGI(TAG, "✓ Power Mode set successfully");
    } else {
        ESP_LOGE(TAG, "✗ Failed to set power mode");
    }
    
    // Test hysteresis
    if (as5600_set_hysteresis(as5600, AS5600_HYSTERESIS_1LSB) == ESP_OK) {
        ESP_LOGI(TAG, "✓ Hysteresis set successfully");
        // Set back to off
        as5600_set_hysteresis(as5600, AS5600_HYSTERESIS_OFF);
    } else {
        ESP_LOGE(TAG, "✗ Failed to set hysteresis");
    }
    
    ESP_LOGI(TAG, "Configuration test complete (basic settings tested)");
}

void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  AS5600 Comprehensive Test Suite");
    ESP_LOGI(TAG, "========================================\n");
    
    // Initialize I2C
    ESP_LOGI(TAG, "Initializing I2C...");
    if (i2c_master_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C!");
        return;
    }
    ESP_LOGI(TAG, "✓ I2C initialized on port %d (SDA: %d, SCL: %d)\n", 
             I2C_MASTER_NUM, SDA_GPIO, SCL_GPIO);
    
    // Initialize AS5600
    ESP_LOGI(TAG, "Initializing AS5600...");
    as5600_config_t as5600_config = {
        .i2c_port = I2C_MASTER_NUM,
        .dev_addr = AS5600_I2C_ADDRESS,
    };
    
    as5600_handle_t as5600;
    if (as5600_init(&as5600_config, &as5600) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize AS5600!");
        return;
    }
    ESP_LOGI(TAG, "✓ AS5600 initialized at address 0x%02X\n", AS5600_I2C_ADDRESS);
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Run all tests
    test_magnet_detection(&as5600);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    test_reading_functions(&as5600);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    test_agc_magnitude(&as5600);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    test_configuration(&as5600);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    ESP_LOGI(TAG, "\n========================================");
    ESP_LOGI(TAG, "  All Tests Complete!");
    ESP_LOGI(TAG, "========================================\n");
    
    // Continuous reading loop
    ESP_LOGI(TAG, "Starting continuous angle monitoring...\n");
    
    uint32_t loop_count = 0;
    while (1) {
        float degrees;
        uint16_t raw_angle;
        bool magnet_ok = false;
        
        // Quick status check
        bool detected, too_strong, too_weak;
        if (as5600_is_magnet_detected(&as5600, &detected) == ESP_OK &&
            as5600_is_magnet_too_strong(&as5600, &too_strong) == ESP_OK &&
            as5600_is_magnet_too_weak(&as5600, &too_weak) == ESP_OK) {
            magnet_ok = detected && !too_strong && !too_weak;
        }
        
        // Read angles
        as5600_get_degrees(&as5600, &degrees);
        as5600_get_raw_angle(&as5600, &raw_angle);
        
        // Display with status indicator
        const char *status_icon = magnet_ok ? "✓" : "⚠";
        ESP_LOGI(TAG, "[%s] Loop %lu | Angle: %6.2f° | Raw: %4d | Magnet: %s",
                 status_icon, loop_count++, degrees, raw_angle,
                 magnet_ok ? "OK" : "CHECK POSITION");
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
