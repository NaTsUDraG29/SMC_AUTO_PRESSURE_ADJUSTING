#ifndef TPS55288_H
#define TPS55288_H

// #include "pico/stdlib.h"
// #include "hardware/i2c.h"

// Default I2C 7-bit address (can be changed via ADDR pin/mode reg)
#define TPS55288_I2C_ADDR 0x74

// === Register Map ===
#define TPS55288_REG_VREF_LSB   0x00   // REF DAC LSB
#define TPS55288_REG_VREF_MSB   0x01   // REF DAC MSB
#define TPS55288_REG_IOUT_LIMIT 0x02   // Current limit
#define TPS55288_REG_VOUT_SR    0x03   // Slew rate / OCP delay
#define TPS55288_REG_VOUT_FS    0x04   // Feedback source / ratio
#define TPS55288_REG_CDC        0x05   // Cable drop compensation
#define TPS55288_REG_MODE       0x06   // Mode control
#define TPS55288_REG_STATUS     0x07   // Status & fault flags


// Mode bits (example, confirm with datasheet!)
#define TPS55288_MODE_ENABLE    0x01
#define TPS55288_MODE_DISABLE   0x00




// === CONTROL BITS ===
// MODE register (0x06)
#define TPS55288_MODE_OE        (1 << 0)   // Output Enable
#define TPS55288_MODE_VCC_EXT   (1 << 1)   // Use external VCC
#define TPS55288_MODE_PWM       (1 << 4)   // Force PWM mode
#define TPS55288_MODE_PFM       (0 << 4)   // Auto PFM/PWM
#define TPS55288_MODE_DISCHG    (1 << 6)   // Output discharge enable

// STATUS register (0x07)
#define TPS55288_STAT_BUCK      (1 << 0)
#define TPS55288_STAT_BOOST     (1 << 1)
#define TPS55288_STAT_BUCKBOOST (1 << 2)
#define TPS55288_STAT_SCP       (1 << 4)
#define TPS55288_STAT_OCP       (1 << 5)
#define TPS55288_STAT_OVP       (1 << 6)


// --- API ---
void tps55288_init(void);   // init I2C if needed
void tps55288_enable(void);
void tps55288_disable(void);
void tps55288_set_voltage_mv(uint16_t millivolts);
uint16_t tps55288_read_vout_mv(void);


#endif


