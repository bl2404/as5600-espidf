/**
 * @file as5600.c
 * @brief AS5600 Magnetic Rotary Position Sensor Library Implementation
 */

#include "as5600.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "AS5600";

// Register addresses
#define AS5600_REG_ZMCO           0x00
#define AS5600_REG_ZPOS_MSB       0x01
#define AS5600_REG_ZPOS_LSB       0x02
#define AS5600_REG_MPOS_MSB       0x03
#define AS5600_REG_MPOS_LSB       0x04
#define AS5600_REG_MANG_MSB       0x05
#define AS5600_REG_MANG_LSB       0x06
#define AS5600_REG_CONF_MSB       0x07
#define AS5600_REG_CONF_LSB       0x08
#define AS5600_REG_RAW_ANGLE_MSB  0x0C
#define AS5600_REG_RAW_ANGLE_LSB  0x0D
#define AS5600_REG_ANGLE_MSB      0x0E
#define AS5600_REG_ANGLE_LSB      0x0F
#define AS5600_REG_STATUS         0x0B
#define AS5600_REG_AGC            0x1A
#define AS5600_REG_MAGNITUDE_MSB  0x1B
#define AS5600_REG_MAGNITUDE_LSB  0x1C
#define AS5600_REG_BURN           0xFF

// I2C timeout
#define AS5600_I2C_TIMEOUT_MS     1000

/**
 * @brief Read a single register from AS5600
 */
static esp_err_t as5600_read_register(as5600_handle_t *handle, uint8_t reg_addr, uint8_t *data)
{
    esp_err_t ret;
    
    ret = i2c_master_write_read_device(handle->i2c_port, handle->dev_addr,
                                       &reg_addr, 1,
                                       data, 1,
                                       pdMS_TO_TICKS(AS5600_I2C_TIMEOUT_MS));
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read register 0x%02X: %s", reg_addr, esp_err_to_name(ret));
    }
    
    return ret;
}

/**
 * @brief Read two consecutive registers from AS5600
 */
static esp_err_t as5600_read_register_16(as5600_handle_t *handle, uint8_t reg_msb, uint8_t reg_lsb, uint16_t *data)
{
    esp_err_t ret;
    uint8_t msb, lsb;
    
    ret = as5600_read_register(handle, reg_msb, &msb);
    if (ret != ESP_OK) {
        return ret;
    }
    
    ret = as5600_read_register(handle, reg_lsb, &lsb);
    if (ret != ESP_OK) {
        return ret;
    }
    
    *data = ((uint16_t)msb << 8) | lsb;
    return ESP_OK;
}

/**
 * @brief Write a single register to AS5600
 */
static esp_err_t as5600_write_register(as5600_handle_t *handle, uint8_t reg_addr, uint8_t data)
{
    esp_err_t ret;
    uint8_t write_buf[2] = {reg_addr, data};
    
    ret = i2c_master_write_to_device(handle->i2c_port, handle->dev_addr,
                                     write_buf, 2,
                                     pdMS_TO_TICKS(AS5600_I2C_TIMEOUT_MS));
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write register 0x%02X: %s", reg_addr, esp_err_to_name(ret));
    }
    
    return ret;
}

esp_err_t as5600_init(const as5600_config_t *config, as5600_handle_t *handle)
{
    if (config == NULL || handle == NULL) {
        ESP_LOGE(TAG, "Invalid parameters");
        return ESP_ERR_INVALID_ARG;
    }
    
    handle->i2c_port = config->i2c_port;
    handle->dev_addr = config->dev_addr;
    
    ESP_LOGI(TAG, "AS5600 initialized on I2C port %d, address 0x%02X", 
             handle->i2c_port, handle->dev_addr);
    
    return ESP_OK;
}

esp_err_t as5600_get_raw_angle(as5600_handle_t *handle, uint16_t *raw_angle)
{
    if (handle == NULL || raw_angle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    return as5600_read_register_16(handle, AS5600_REG_RAW_ANGLE_MSB, 
                                   AS5600_REG_RAW_ANGLE_LSB, raw_angle);
}

esp_err_t as5600_get_angle(as5600_handle_t *handle, uint16_t *angle)
{
    if (handle == NULL || angle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    return as5600_read_register_16(handle, AS5600_REG_ANGLE_MSB, 
                                   AS5600_REG_ANGLE_LSB, angle);
}

esp_err_t as5600_get_degrees(as5600_handle_t *handle, float *degrees)
{
    if (handle == NULL || degrees == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t msb, lsb;
    esp_err_t ret;
    
    ret = as5600_read_register(handle, AS5600_REG_RAW_ANGLE_MSB, &msb);
    if (ret != ESP_OK) {
        return ret;
    }
    
    ret = as5600_read_register(handle, AS5600_REG_RAW_ANGLE_LSB, &lsb);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Convert to degrees: MSB * 22.5 + LSB * 0.087890625
    *degrees = (float)msb * 22.5f + (float)lsb * 0.087890625f;
    
    return ESP_OK;
}

esp_err_t as5600_get_position(as5600_handle_t *handle, uint16_t *position)
{
    return as5600_get_raw_angle(handle, position);
}

esp_err_t as5600_get_status(as5600_handle_t *handle, uint8_t *status)
{
    if (handle == NULL || status == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = as5600_read_register(handle, AS5600_REG_STATUS, status);
    if (ret == ESP_OK) {
        *status &= 0b00111000;  // Mask relevant bits
    }
    
    return ret;
}

esp_err_t as5600_is_magnet_too_strong(as5600_handle_t *handle, bool *too_strong)
{
    if (handle == NULL || too_strong == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t status;
    esp_err_t ret = as5600_get_status(handle, &status);
    
    if (ret == ESP_OK) {
        *too_strong = (status & (1 << 3)) ? true : false;
    }
    
    return ret;
}

esp_err_t as5600_is_magnet_too_weak(as5600_handle_t *handle, bool *too_weak)
{
    if (handle == NULL || too_weak == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t status;
    esp_err_t ret = as5600_get_status(handle, &status);
    
    if (ret == ESP_OK) {
        *too_weak = (status & (1 << 4)) ? true : false;
    }
    
    return ret;
}

esp_err_t as5600_is_magnet_detected(as5600_handle_t *handle, bool *detected)
{
    if (handle == NULL || detected == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t status;
    esp_err_t ret = as5600_get_status(handle, &status);
    
    if (ret == ESP_OK) {
        *detected = (status & (1 << 5)) ? true : false;
    }
    
    return ret;
}

esp_err_t as5600_get_gain(as5600_handle_t *handle, uint8_t *gain)
{
    if (handle == NULL || gain == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    return as5600_read_register(handle, AS5600_REG_AGC, gain);
}

esp_err_t as5600_get_magnitude(as5600_handle_t *handle, uint16_t *magnitude)
{
    if (handle == NULL || magnitude == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    return as5600_read_register_16(handle, AS5600_REG_MAGNITUDE_MSB, 
                                   AS5600_REG_MAGNITUDE_LSB, magnitude);
}

esp_err_t as5600_set_power_mode(as5600_handle_t *handle, as5600_power_mode_t power_mode)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (power_mode > AS5600_POWER_MODE_LPM3) {
        ESP_LOGE(TAG, "Invalid power mode");
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t current_conf;
    esp_err_t ret = as5600_read_register(handle, AS5600_REG_CONF_LSB, &current_conf);
    if (ret != ESP_OK) {
        return ret;
    }
    
    uint8_t new_conf = (current_conf & 0b11111100) | power_mode;
    
    ret = as5600_write_register(handle, AS5600_REG_CONF_LSB, new_conf);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Verify write
    uint8_t verify;
    ret = as5600_read_register(handle, AS5600_REG_CONF_LSB, &verify);
    if (ret != ESP_OK || verify != new_conf) {
        ESP_LOGE(TAG, "Failed to set power mode");
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

esp_err_t as5600_set_hysteresis(as5600_handle_t *handle, as5600_hysteresis_t hysteresis)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (hysteresis > AS5600_HYSTERESIS_3LSB) {
        ESP_LOGE(TAG, "Invalid hysteresis");
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t current_conf;
    esp_err_t ret = as5600_read_register(handle, AS5600_REG_CONF_LSB, &current_conf);
    if (ret != ESP_OK) {
        return ret;
    }
    
    uint8_t new_conf = (current_conf & 0b11110011) | (hysteresis << 2);
    
    ret = as5600_write_register(handle, AS5600_REG_CONF_LSB, new_conf);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Verify write
    uint8_t verify;
    ret = as5600_read_register(handle, AS5600_REG_CONF_LSB, &verify);
    if (ret != ESP_OK || verify != new_conf) {
        ESP_LOGE(TAG, "Failed to set hysteresis");
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

esp_err_t as5600_set_output_stage(as5600_handle_t *handle, as5600_output_stage_t output_stage)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (output_stage > AS5600_OUTPUT_STAGE_DIGITAL_PWM) {
        ESP_LOGE(TAG, "Invalid output stage");
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t current_conf;
    esp_err_t ret = as5600_read_register(handle, AS5600_REG_CONF_LSB, &current_conf);
    if (ret != ESP_OK) {
        return ret;
    }
    
    uint8_t new_conf = (current_conf & 0b11001111) | (output_stage << 4);
    
    ret = as5600_write_register(handle, AS5600_REG_CONF_LSB, new_conf);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Verify write
    uint8_t verify;
    ret = as5600_read_register(handle, AS5600_REG_CONF_LSB, &verify);
    if (ret != ESP_OK || verify != new_conf) {
        ESP_LOGE(TAG, "Failed to set output stage");
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

esp_err_t as5600_set_pwm_frequency(as5600_handle_t *handle, uint8_t frequency)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (frequency > 3) {
        ESP_LOGE(TAG, "Invalid PWM frequency");
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t current_conf;
    esp_err_t ret = as5600_read_register(handle, AS5600_REG_CONF_LSB, &current_conf);
    if (ret != ESP_OK) {
        return ret;
    }
    
    uint8_t new_conf = (current_conf & 0b00111111) | (frequency << 6);
    
    ret = as5600_write_register(handle, AS5600_REG_CONF_LSB, new_conf);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Verify write
    uint8_t verify;
    ret = as5600_read_register(handle, AS5600_REG_CONF_LSB, &verify);
    if (ret != ESP_OK || verify != new_conf) {
        ESP_LOGE(TAG, "Failed to set PWM frequency");
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

esp_err_t as5600_set_slow_filter(as5600_handle_t *handle, as5600_slow_filter_t slow_filter)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (slow_filter > AS5600_SLOW_FILTER_2X) {
        ESP_LOGE(TAG, "Invalid slow filter");
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t current_conf;
    esp_err_t ret = as5600_read_register(handle, AS5600_REG_CONF_MSB, &current_conf);
    if (ret != ESP_OK) {
        return ret;
    }
    
    uint8_t new_conf = (current_conf & 0b11111100) | slow_filter;
    
    ret = as5600_write_register(handle, AS5600_REG_CONF_MSB, new_conf);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Verify write
    uint8_t verify;
    ret = as5600_read_register(handle, AS5600_REG_CONF_MSB, &verify);
    if (ret != ESP_OK || verify != new_conf) {
        ESP_LOGE(TAG, "Failed to set slow filter");
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

esp_err_t as5600_set_fast_filter_threshold(as5600_handle_t *handle, as5600_fast_filter_threshold_t threshold)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (threshold > AS5600_FAST_FILTER_THRESHOLD_10LSB) {
        ESP_LOGE(TAG, "Invalid fast filter threshold");
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t current_conf;
    esp_err_t ret = as5600_read_register(handle, AS5600_REG_CONF_MSB, &current_conf);
    if (ret != ESP_OK) {
        return ret;
    }
    
    uint8_t new_conf = (current_conf & 0b11000111) | (threshold << 3);
    
    ret = as5600_write_register(handle, AS5600_REG_CONF_MSB, new_conf);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Verify write
    uint8_t verify;
    ret = as5600_read_register(handle, AS5600_REG_CONF_MSB, &verify);
    if (ret != ESP_OK || verify != new_conf) {
        ESP_LOGE(TAG, "Failed to set fast filter threshold");
        return ESP_FAIL;
    }
    
    return ESP_OK;
}
