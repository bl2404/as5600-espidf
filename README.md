# AS5600 Library for ESP-IDF

A complete and easy-to-use ESP-IDF component library for the AS5600 12-bit magnetic rotary position sensor. This library is ported from the Arduino AS5600 library with full ESP-IDF compatibility.

## Features

- ✅ Read raw angle (12-bit: 0-4095)
- ✅ Read angle in degrees (0-360°)
- ✅ Magnet detection and status checking
- ✅ AGC (Automatic Gain Control) reading
- ✅ Magnitude reading
- ✅ Power mode configuration
- ✅ Hysteresis configuration
- ✅ Output stage configuration
- ✅ Filter configuration
- ✅ Continuous rotation tracking support
- ✅ Simple C API
- ✅ Full error handling

## Hardware Requirements

- ESP32 (or any ESP-IDF compatible board)
- AS5600 Magnetic Rotary Position Sensor
- Diametric magnet (typically included with AS5600 module)

## Wiring

| AS5600 Pin | ESP32 Pin | Description |
|------------|-----------|-------------|
| VCC | 3.3V or 5V | Power supply |
| GND | GND | Ground |
| SDA | GPIO 21 | I2C Data (configurable) |
| SCL | GPIO 22 | I2C Clock (configurable) |
| DIR | - | Optional: direction pin |
| OUT | - | Optional: analog/PWM output |
| PGO | - | Optional: programming pin |

## Installation & Integration

### Method 1: Add as ESP-IDF Component (Recommended)

1. **Copy the component to your project:**

```bash
# Navigate to your ESP-IDF project
cd your_project_directory

# Create components directory if it doesn't exist
mkdir -p components

# Copy the AS5600 library
cp -r /path/to/as5600_espidf/components/as5600 components/
```

2. **That's it!** ESP-IDF will automatically detect and build the component.

### Method 2: Add as External Component

1. **Create `components` directory in your project root:**

```bash
mkdir -p components
```

2. **Create a symbolic link:**

```bash
cd components
ln -s /path/to/as5600_espidf/components/as5600 as5600
```

### Method 3: Use an Example Project

You can directly use one of the example projects:

```bash
# Copy an example
cp -r /path/to/as5600_espidf/examples/read_angle my_as5600_project
cd my_as5600_project

# Create components directory and copy library
mkdir -p components
cp -r /path/to/as5600_espidf/components/as5600 components/

# Build
idf.py build

# Flash
idf.py -p /dev/ttyUSB0 flash monitor
```

## Usage Example

### Basic Angle Reading

```c
#include "driver/i2c.h"
#include "as5600.h"

// Initialize I2C
i2c_config_t conf = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = 21,
    .scl_io_num = 22,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = 400000,
};
i2c_param_config(I2C_NUM_0, &conf);
i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);

// Initialize AS5600
as5600_config_t as5600_config = {
    .i2c_port = I2C_NUM_0,
    .dev_addr = AS5600_I2C_ADDRESS,
};

as5600_handle_t as5600;
as5600_init(&as5600_config, &as5600);

// Read angle
float degrees;
as5600_get_degrees(&as5600, &degrees);
printf("Angle: %.2f degrees\n", degrees);
```

## API Reference

### Initialization

```c
esp_err_t as5600_init(const as5600_config_t *config, as5600_handle_t *handle);
```

### Reading Functions

```c
// Get raw angle (0-4095)
esp_err_t as5600_get_raw_angle(as5600_handle_t *handle, uint16_t *raw_angle);

// Get scaled angle (0-4095)
esp_err_t as5600_get_angle(as5600_handle_t *handle, uint16_t *angle);

// Get angle in degrees (0-360)
esp_err_t as5600_get_degrees(as5600_handle_t *handle, float *degrees);

// Get position (alias for raw angle)
esp_err_t as5600_get_position(as5600_handle_t *handle, uint16_t *position);
```

### Status Functions

```c
// Check magnet detection
esp_err_t as5600_is_magnet_detected(as5600_handle_t *handle, bool *detected);
esp_err_t as5600_is_magnet_too_strong(as5600_handle_t *handle, bool *too_strong);
esp_err_t as5600_is_magnet_too_weak(as5600_handle_t *handle, bool *too_weak);

// Get status register
esp_err_t as5600_get_status(as5600_handle_t *handle, uint8_t *status);

// Get AGC and magnitude
esp_err_t as5600_get_gain(as5600_handle_t *handle, uint8_t *gain);
esp_err_t as5600_get_magnitude(as5600_handle_t *handle, uint16_t *magnitude);
```

### Configuration Functions

```c
esp_err_t as5600_set_power_mode(as5600_handle_t *handle, as5600_power_mode_t power_mode);
esp_err_t as5600_set_hysteresis(as5600_handle_t *handle, as5600_hysteresis_t hysteresis);
esp_err_t as5600_set_output_stage(as5600_handle_t *handle, as5600_output_stage_t output_stage);
esp_err_t as5600_set_pwm_frequency(as5600_handle_t *handle, uint8_t frequency);
esp_err_t as5600_set_slow_filter(as5600_handle_t *handle, as5600_slow_filter_t slow_filter);
esp_err_t as5600_set_fast_filter_threshold(as5600_handle_t *handle, as5600_fast_filter_threshold_t threshold);
```

## Examples

Three complete examples are provided:

### 1. Read Angle (`examples/read_angle`)
Basic example showing how to read angle values in degrees and raw format.

**Build and run:**
```bash
cd examples/read_angle
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

### 2. Read Magnet Status (`examples/read_magnet`)
Shows how to check magnet detection, strength, and position quality.

**Build and run:**
```bash
cd examples/read_magnet
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

### 3. Continuous Rotation (`examples/continuous_rotation`)
Demonstrates tracking rotations beyond 360° with revolution counting.

**Build and run:**
```bash
cd examples/continuous_rotation
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

## Project Structure

```
as5600_espidf/
├── components/
│   └── as5600/
│       ├── include/
│       │   └── as5600.h          # Header file
│       ├── as5600.c               # Implementation
│       └── CMakeLists.txt         # Component build config
└── examples/
    ├── read_angle/                # Example 1: Basic angle reading
    │   ├── main/
    │   │   ├── read_angle.c
    │   │   └── CMakeLists.txt
    │   └── CMakeLists.txt
    ├── read_magnet/               # Example 2: Magnet status
    │   ├── main/
    │   │   ├── read_magnet.c
    │   │   └── CMakeLists.txt
    │   └── CMakeLists.txt
    └── continuous_rotation/       # Example 3: Revolution tracking
        ├── main/
        │   ├── continuous_rotation.c
        │   └── CMakeLists.txt
        └── CMakeLists.txt
```

## Troubleshooting

### No Magnet Detected
- Ensure magnet is placed correctly above the sensor
- Check magnet polarity (try flipping it)
- Verify magnet is within 2-3mm of the sensor

### Magnet Too Strong
- Move magnet farther from the sensor
- Try a weaker magnet

### Magnet Too Weak
- Move magnet closer to the sensor
- Ensure you're using a diametric magnet

### I2C Communication Errors
- Check wiring connections
- Verify I2C pull-up resistors (usually built-in on ESP32)
- Ensure AS5600 is powered (3.3V or 5V)
- Try reducing I2C clock speed to 100kHz

### Changing I2C Pins
Modify these lines in the example code:
```c
#define SDA_GPIO    21  // Change to your SDA pin
#define SCL_GPIO    22  // Change to your SCL pin
```

## Advanced Usage

### Continuous Rotation Tracking

For applications requiring unlimited rotation tracking:

```c
int32_t revolutions = 0;
int32_t last_position = 0;

while (1) {
    uint16_t current_position;
    as5600_get_position(&as5600, &current_position);
    
    // Detect wraparound
    if ((last_position - current_position) > 2047) {
        revolutions++;  // Forward rotation
    } else if ((last_position - current_position) < -2047) {
        revolutions--;  // Backward rotation
    }
    
    int64_t absolute_position = revolutions * 4096 + current_position;
    last_position = current_position;
}
```

See `examples/continuous_rotation` for complete implementation.

## Configuration Options

### Power Modes
- `AS5600_POWER_MODE_NORM` - Normal mode (default)
- `AS5600_POWER_MODE_LPM1` - Low Power Mode 1
- `AS5600_POWER_MODE_LPM2` - Low Power Mode 2
- `AS5600_POWER_MODE_LPM3` - Low Power Mode 3

### Hysteresis
- `AS5600_HYSTERESIS_OFF` - No hysteresis
- `AS5600_HYSTERESIS_1LSB` - 1 LSB hysteresis
- `AS5600_HYSTERESIS_2LSB` - 2 LSB hysteresis
- `AS5600_HYSTERESIS_3LSB` - 3 LSB hysteresis

### Output Stage
- `AS5600_OUTPUT_STAGE_ANALOG_FULL` - Full range analog output
- `AS5600_OUTPUT_STAGE_ANALOG_REDUCED` - Reduced range analog output
- `AS5600_OUTPUT_STAGE_DIGITAL_PWM` - Digital PWM output

## Technical Specifications

- **Resolution:** 12-bit (4096 positions per revolution)
- **Accuracy:** ±0.05° typical
- **I2C Address:** 0x36 (fixed)
- **Supply Voltage:** 3.3V - 5V
- **I2C Speed:** Up to 1MHz (tested at 400kHz)
- **Update Rate:** 1.5 kHz typical

## License

This library is released into the public domain. Original Arduino library by Kane C. Stoboi.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues.

## Credits

- Original Arduino library: [kanestoboi/AS5600](https://github.com/kanestoboi/AS5600)
- Ported to ESP-IDF: 2026

## Support

For issues, questions, or suggestions, please open an issue on the GitHub repository.
