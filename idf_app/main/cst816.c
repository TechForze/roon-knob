#include "cst816.h"

#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"

#include <string.h>

static const char *TAG = "cst816";

// Hardware configuration
#define CST816_I2C_PORT        I2C_NUM_0
#define CST816_I2C_ADDR        0x15
#define CST816_I2C_SDA_GPIO    11
#define CST816_I2C_SCL_GPIO    12
#define CST816_I2C_FREQ_HZ     300000  // 300 kHz

// I2C timeout
#define I2C_TIMEOUT_MS         1000

// CST816 register addresses
#define CST816_REG_STATUS      0x00
#define CST816_REG_TOUCH_NUM   0x02
#define CST816_REG_XPOS_H      0x03
#define CST816_REG_XPOS_L      0x04
#define CST816_REG_YPOS_H      0x05
#define CST816_REG_YPOS_L      0x06

static bool s_initialized = false;

/**
 * @brief Write data to CST816 register
 */
static esp_err_t cst816_i2c_write(uint8_t reg, uint8_t *data, uint8_t len) {
    uint8_t *write_buf = (uint8_t *)malloc(len + 1);
    if (!write_buf) {
        return ESP_ERR_NO_MEM;
    }

    write_buf[0] = reg;
    memcpy(write_buf + 1, data, len);

    esp_err_t ret = i2c_master_write_to_device(
        CST816_I2C_PORT,
        CST816_I2C_ADDR,
        write_buf,
        len + 1,
        pdMS_TO_TICKS(I2C_TIMEOUT_MS)
    );

    free(write_buf);
    return ret;
}

/**
 * @brief Read data from CST816 register
 */
static esp_err_t cst816_i2c_read(uint8_t reg, uint8_t *data, uint8_t len) {
    return i2c_master_write_read_device(
        CST816_I2C_PORT,
        CST816_I2C_ADDR,
        &reg,
        1,
        data,
        len,
        pdMS_TO_TICKS(I2C_TIMEOUT_MS)
    );
}

bool cst816_init(void) {
    ESP_LOGI(TAG, "Initializing CST816 touch controller");

    // Configure I2C
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = CST816_I2C_SDA_GPIO,
        .scl_io_num = CST816_I2C_SCL_GPIO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = CST816_I2C_FREQ_HZ,
        .clk_flags = 0,
    };

    esp_err_t ret = i2c_param_config(CST816_I2C_PORT, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C param config failed: %s", esp_err_to_name(ret));
        return false;
    }

    ret = i2c_driver_install(CST816_I2C_PORT, conf.mode, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(ret));
        return false;
    }

    // Switch CST816 to normal mode (wake up from sleep)
    uint8_t mode = 0x00;
    ret = cst816_i2c_write(CST816_REG_STATUS, &mode, 1);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to write to CST816, but continuing: %s", esp_err_to_name(ret));
        // Don't fail here - some devices may not respond to this command
    }

    s_initialized = true;
    ESP_LOGI(TAG, "CST816 initialized successfully (I2C: SDA=%d, SCL=%d, ADDR=0x%02x)",
             CST816_I2C_SDA_GPIO, CST816_I2C_SCL_GPIO, CST816_I2C_ADDR);
    return true;
}

bool cst816_get_touch(uint16_t *x, uint16_t *y) {
    if (!s_initialized) {
        ESP_LOGW(TAG, "CST816 not initialized");
        return false;
    }

    if (!x || !y) {
        return false;
    }

    // Read 7 bytes starting from register 0x00
    // [0] = status
    // [1] = gesture
    // [2] = touch count
    // [3] = X high byte
    // [4] = X low byte
    // [5] = Y high byte
    // [6] = Y low byte
    uint8_t data[7] = {0};
    esp_err_t ret = cst816_i2c_read(CST816_REG_STATUS, data, 7);
    if (ret != ESP_OK) {
        // Don't log every read failure - too noisy
        return false;
    }

    uint8_t touch_count = data[2];
    if (touch_count == 0) {
        return false;  // No touch detected
    }

    // Extract coordinates (12-bit values)
    *x = ((uint16_t)(data[3] & 0x0F) << 8) | (uint16_t)data[4];
    *y = ((uint16_t)(data[5] & 0x0F) << 8) | (uint16_t)data[6];

    return true;
}
