/**
 * @file as5600.h
 * @brief AS5600 Magnetic Rotary Position Sensor Library for ESP-IDF
 * 
 * This library provides an interface for the AS5600 12-bit magnetic rotary position sensor
 * using ESP-IDF's I2C driver.
 * 
 * Features:
 * - Read raw and scaled angle measurements
 * - Magnet detection and strength checking
 * - Configuration of power modes, filters, and output stages
 * - Support for continuous rotation tracking
 * 
 * @author Ported to ESP-IDF
 * @date 2026
 */

#ifndef AS5600_H
#define AS5600_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/i2c.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief AS5600 I2C address
 */
#define AS5600_I2C_ADDRESS 0x36

/**
 * @brief AS5600 power mode options
 */
typedef enum {
    AS5600_POWER_MODE_NORM = 0,  /**< Normal mode */
    AS5600_POWER_MODE_LPM1 = 1,  /**< Low Power Mode 1 */
    AS5600_POWER_MODE_LPM2 = 2,  /**< Low Power Mode 2 */
    AS5600_POWER_MODE_LPM3 = 3   /**< Low Power Mode 3 */
} as5600_power_mode_t;

/**
 * @brief AS5600 hysteresis options
 */
typedef enum {
    AS5600_HYSTERESIS_OFF  = 0,  /**< Hysteresis off */
    AS5600_HYSTERESIS_1LSB = 1,  /**< 1 LSB hysteresis */
    AS5600_HYSTERESIS_2LSB = 2,  /**< 2 LSB hysteresis */
    AS5600_HYSTERESIS_3LSB = 3   /**< 3 LSB hysteresis */
} as5600_hysteresis_t;

/**
 * @brief AS5600 output stage options
 */
typedef enum {
    AS5600_OUTPUT_STAGE_ANALOG_FULL    = 0,  /**< Analog (full range) */
    AS5600_OUTPUT_STAGE_ANALOG_REDUCED = 1,  /**< Analog (reduced range) */
    AS5600_OUTPUT_STAGE_DIGITAL_PWM    = 2   /**< Digital PWM */
} as5600_output_stage_t;

/**
 * @brief AS5600 slow filter options
 */
typedef enum {
    AS5600_SLOW_FILTER_16X = 0,  /**< 16x slow filter */
    AS5600_SLOW_FILTER_8X  = 1,  /**< 8x slow filter */
    AS5600_SLOW_FILTER_4X  = 2,  /**< 4x slow filter */
    AS5600_SLOW_FILTER_2X  = 3   /**< 2x slow filter */
} as5600_slow_filter_t;

/**
 * @brief AS5600 fast filter threshold options
 */
typedef enum {
    AS5600_FAST_FILTER_THRESHOLD_SLOW  = 0,  /**< Slow filter only */
    AS5600_FAST_FILTER_THRESHOLD_6LSB  = 1,  /**< 6 LSB threshold */
    AS5600_FAST_FILTER_THRESHOLD_7LSB  = 2,  /**< 7 LSB threshold */
    AS5600_FAST_FILTER_THRESHOLD_9LSB  = 3,  /**< 9 LSB threshold */
    AS5600_FAST_FILTER_THRESHOLD_18LSB = 4,  /**< 18 LSB threshold */
    AS5600_FAST_FILTER_THRESHOLD_21LSB = 5,  /**< 21 LSB threshold */
    AS5600_FAST_FILTER_THRESHOLD_24LSB = 6,  /**< 24 LSB threshold */
    AS5600_FAST_FILTER_THRESHOLD_10LSB = 7   /**< 10 LSB threshold */
} as5600_fast_filter_threshold_t;

/**
 * @brief AS5600 configuration structure
 */
typedef struct {
    i2c_port_t i2c_port;        /**< I2C port number */
    uint8_t dev_addr;           /**< Device I2C address (default: AS5600_I2C_ADDRESS) */
} as5600_config_t;

/**
 * @brief AS5600 device handle
 */
typedef struct {
    i2c_port_t i2c_port;
    uint8_t dev_addr;
} as5600_handle_t;

/**
 * @brief Initialize AS5600 sensor
 * 
 * @param config Pointer to configuration structure
 * @param handle Pointer to device handle (will be populated)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t as5600_init(const as5600_config_t *config, as5600_handle_t *handle);

/**
 * @brief Get raw angle (12-bit value: 0-4095)
 * 
 * @param handle Device handle
 * @param raw_angle Pointer to store raw angle value
 * @return esp_err_t ESP_OK on success
 */
esp_err_t as5600_get_raw_angle(as5600_handle_t *handle, uint16_t *raw_angle);

/**
 * @brief Get scaled angle (12-bit value: 0-4095)
 * 
 * @param handle Device handle
 * @param angle Pointer to store angle value
 * @return esp_err_t ESP_OK on success
 */
esp_err_t as5600_get_angle(as5600_handle_t *handle, uint16_t *angle);

/**
 * @brief Get angle in degrees (0-360)
 * 
 * @param handle Device handle
 * @param degrees Pointer to store angle in degrees
 * @return esp_err_t ESP_OK on success
 */
esp_err_t as5600_get_degrees(as5600_handle_t *handle, float *degrees);

/**
 * @brief Get position (alias for raw angle)
 * 
 * @param handle Device handle
 * @param position Pointer to store position value
 * @return esp_err_t ESP_OK on success
 */
esp_err_t as5600_get_position(as5600_handle_t *handle, uint16_t *position);

/**
 * @brief Get status register
 * 
 * @param handle Device handle
 * @param status Pointer to store status value
 * @return esp_err_t ESP_OK on success
 */
esp_err_t as5600_get_status(as5600_handle_t *handle, uint8_t *status);

/**
 * @brief Check if magnet is too strong
 * 
 * @param handle Device handle
 * @param too_strong Pointer to store result
 * @return esp_err_t ESP_OK on success
 */
esp_err_t as5600_is_magnet_too_strong(as5600_handle_t *handle, bool *too_strong);

/**
 * @brief Check if magnet is too weak
 * 
 * @param handle Device handle
 * @param too_weak Pointer to store result
 * @return esp_err_t ESP_OK on success
 */
esp_err_t as5600_is_magnet_too_weak(as5600_handle_t *handle, bool *too_weak);

/**
 * @brief Check if magnet is detected
 * 
 * @param handle Device handle
 * @param detected Pointer to store result
 * @return esp_err_t ESP_OK on success
 */
esp_err_t as5600_is_magnet_detected(as5600_handle_t *handle, bool *detected);

/**
 * @brief Get AGC (Automatic Gain Control) value
 * 
 * @param handle Device handle
 * @param gain Pointer to store gain value
 * @return esp_err_t ESP_OK on success
 */
esp_err_t as5600_get_gain(as5600_handle_t *handle, uint8_t *gain);

/**
 * @brief Get magnitude value
 * 
 * @param handle Device handle
 * @param magnitude Pointer to store magnitude value
 * @return esp_err_t ESP_OK on success
 */
esp_err_t as5600_get_magnitude(as5600_handle_t *handle, uint16_t *magnitude);

/**
 * @brief Set power mode
 * 
 * @param handle Device handle
 * @param power_mode Power mode to set
 * @return esp_err_t ESP_OK on success
 */
esp_err_t as5600_set_power_mode(as5600_handle_t *handle, as5600_power_mode_t power_mode);

/**
 * @brief Set hysteresis
 * 
 * @param handle Device handle
 * @param hysteresis Hysteresis value to set
 * @return esp_err_t ESP_OK on success
 */
esp_err_t as5600_set_hysteresis(as5600_handle_t *handle, as5600_hysteresis_t hysteresis);

/**
 * @brief Set output stage
 * 
 * @param handle Device handle
 * @param output_stage Output stage to set
 * @return esp_err_t ESP_OK on success
 */
esp_err_t as5600_set_output_stage(as5600_handle_t *handle, as5600_output_stage_t output_stage);

/**
 * @brief Set PWM frequency
 * 
 * @param handle Device handle
 * @param frequency Frequency value (0-3)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t as5600_set_pwm_frequency(as5600_handle_t *handle, uint8_t frequency);

/**
 * @brief Set slow filter
 * 
 * @param handle Device handle
 * @param slow_filter Slow filter setting
 * @return esp_err_t ESP_OK on success
 */
esp_err_t as5600_set_slow_filter(as5600_handle_t *handle, as5600_slow_filter_t slow_filter);

/**
 * @brief Set fast filter threshold
 * 
 * @param handle Device handle
 * @param threshold Fast filter threshold
 * @return esp_err_t ESP_OK on success
 */
esp_err_t as5600_set_fast_filter_threshold(as5600_handle_t *handle, as5600_fast_filter_threshold_t threshold);

#ifdef __cplusplus
}
#endif

#endif // AS5600_H
