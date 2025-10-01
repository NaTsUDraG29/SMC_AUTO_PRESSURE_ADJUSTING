#include "TPS55288.h"

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "tps55288.h"

/**************** */

#define I2C_PORT i2c0

// Assume you already set up i2c0 in main.c


static void tps55288_write(uint8_t reg, uint8_t data) {
    uint8_t buf[2] = {reg, data};
    i2c_write_blocking(I2C_PORT, TPS55288_I2C_ADDR, buf, 2, false);
}

static uint8_t tps55288_read(uint8_t reg) 
{
    uint8_t val;
    i2c_write_blocking(I2C_PORT, TPS55288_I2C_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, TPS55288_I2C_ADDR, &val, 1, false);
    return val;
}

// --- Public API ---
void tps55288_init(void) {
    // I2C setup should be done in main.c, here just sanity if needed
    tps55288_enable();

}

void tps55288_enable(void) {
    tps55288_write(TPS55288_REG_MODE, TPS55288_MODE_ENABLE);
}

void tps55288_disable(void) {
    tps55288_write(TPS55288_REG_MODE, TPS55288_MODE_DISABLE);
}

void tps55288_set_voltage_mv(uint16_t millivolts) {
    if (millivolts < 600)  millivolts = 600;
    if (millivolts > 14000) millivolts = 14000;

    // Step size = 12.5mV (10-bit code)
    uint16_t code = (uint16_t)roundf((float)millivolts / 12.5f);

    uint8_t lsb = code & 0xFF;
    uint8_t msb = (code >> 8) & 0x03; // only lower 2 bits valid

    tps55288_write(TPS55288_REG_VREF_LSB, lsb);
    tps55288_write(TPS55288_REG_VREF_MSB, msb);
}

uint16_t tps55288_read_vout_mv(void) {
    uint8_t lsb = tps55288_read(TPS55288_REG_VREF_LSB);
    uint8_t msb = tps55288_read(TPS55288_REG_VREF_MSB);

    uint16_t code = ((uint16_t)msb << 8) | lsb;

    // Datasheet: LSB = 12.5mV (example, check!!)
    return (uint16_t)(code * 12.5f);
}
