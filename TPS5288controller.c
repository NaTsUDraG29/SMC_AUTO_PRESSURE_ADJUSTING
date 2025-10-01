#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "tps55288.h"   // your driver header
#include <string.h>
#include <math.h>
#include "hardware/watchdog.h"
#include "hardware/adc.h"
#include "hardware/structs/adc.h"


// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
#define BOOST_ENABLE 26

bool boostStatus = false;

// Lookup table for voltage → pressure
static const float voltage_values[]  = {0.401221,0.401221,0.400415,0.402026,0.40443, 0.409277, 0.411694, 0.420557, 0.427808, 0.436670, 0.444727,
                                0.452783, 0.460840, 0.468091, 0.479370, 0.487427, 0.497095, 0.504346,
                                0.510791, 0.522070, 0.530933, 0.538184, 0.546240, 0.554297, 0.563159,
                                0.573633, 0.580884, 0.588940, 0.595386, 0.605859, 0.615527, 0.622778,
                                0.631641, 0.638892, 0.646948, 0.656616, 0.664673, 0.672729, 0.681592,
                                0.689648, 0.699316, 0.706567, 0.715430, 0.725098, 0.732349, 0.742822,
                                0.750073, 0.758130, 0.766992, 0.775049,0.792773, 0.800830, 0.809692,
                                0.820166, 0.829028, 0.836279, 0.845142, 0.854004, 0.862061, 0.872534,
                                0.878980 ,0.887842, 0.897510, 0.95566,  0.916040, 0.922485, 0.931348,
                                0.939404, 0.948267, 0.958740, 0.966797, 0.974048 ,0.984521, 0.991772,
                                1.000635, 1.007886, 1.017554, 1.027222, 1.033667, 1.044141, 1.050586,
                                1.0586443, 1.069116 ,1.076367, 1.085230, 1.094092, 1.102954, 1.111816,
                                1.119067, 1.128735, 1.136792, 1.144849, 1.155322,  1.161768,
                                1.171435, 1.179492, 1.187549, 1.199634, 1.206885, 1.213330, 1.221387};

                                
static const float pressure_values[] =  {0.39,0.39,0.39,0.39,0.41, 0.45, 0.51, 0.56, 0.61, 0.67, 0.72, 0.79, 0.84 , 0.88,
                                 0.95, 1, 1.06, 1.11, 1.16, 1.23, 1.29, 1.34, 1.39, 1.43,
                                 1.51, 1.56, 1.62, 1.67 , 1.71, 1.78, 1.84, 1.89 , 1.95, 1.99,
                                 2.05, 2.12, 2.16, 2.22, 2.28, 2.32, 2.4, 2.44 , 2.5, 2.55, 2.61, 2.68, 2.72, 2.78, 2.83, 2.88,2.96
                                 ,2.96, 3, 3.05, 3.12, 3.17, 3.24, 3.28, 3.34, 3.39,3.45, 3.52, 3.56,3.62, 3.68, 3.73,3.8,3.84,3.9, 3.95
                                 ,4.01,4.08, 4.12, 4.18, 4.24, 4.29, 4.35, 4.4, 4.46, 4.52, 4.57,4.62, 4.68, 4.74,4.8,4.85,4.91,4.95,5.02,5.08,5.13
                                 ,5.18, 5.24, 5.3, 5.37, 5.41};

const int num_points = sizeof(voltage_values) / sizeof(voltage_values[0]);



// ----------------- HELPERS -----------------
GPIO_init()
{
    gpio_init(BOOST_ENABLE);
    gpio_set_dir(BOOST_ENABLE, GPIO_OUT);
    gpio_put(BOOST_ENABLE, true);
}
float map_voltage_to_pressure(float voltage) 
{
    if (voltage <= voltage_values[0]) return pressure_values[0];
    if (voltage >= voltage_values[num_points-1]) return pressure_values[num_points-1];

    for (int i = 0; i < num_points - 1; i++) {
        if (voltage >= voltage_values[i] && voltage <= voltage_values[i+1]) {
            float slope = (pressure_values[i+1] - pressure_values[i]) /
                          (voltage_values[i+1] - voltage_values[i]);
            return pressure_values[i] + slope * (voltage - voltage_values[i]);
        }
    }
    return -1.0; // error
}


void enable_boost_circuit() {
    gpio_put(BOOST_ENABLE, 1);
    boostStatus = true;
    printf("BOOST circuit ENABLED\n");
}

void disable_boost_circuit() {
    gpio_put(BOOST_ENABLE, 0);
    boostStatus = false;
    printf("BOOST circuit DISABLED\n");
}

void handle_voltage_set() 
{
    int i = 0;
    char input[10];
    while((input[i++] != '\0') || (input[i] != '\n') || (input[i] != '\r'))
    {
        input[i] = getchar_timeout_us(100);
    } 
    int voltage = atoi(input);

    if(voltage < voltage_values[0])
    {
        //out of bounds
        voltage = voltage_values[0];
    }
    else if(voltage > voltage_values[num_points-1])
    {
        voltage = voltage_values[num_points-1];
    }
    else
    {
        //within bounds
        //calculate the value to be set in the register of Vref
        uint16_t voltage_reg_value = (uint16_t)((voltage - 0.88) / 0.02);
        tps55288_set_voltage_mv(voltage_reg_value); 
        printf("Set output voltage to %d V\n", voltage);
    }

}

void handle_voltage_get() 
{
    uint16_t code = tps55288_read_vout_mv();
    float actual_v = (0.88 + (code * 0.02)); 
    printf("Current output voltage: %.2f V (code %u)\n", actual_v, code);
}

void handle_measurement() 
{
    uint16_t raw = adc_read();
    float v = raw * 3.3f / (1 << 12);  // convert ADC → volts
    float p = map_voltage_to_pressure(v);
    printf("ADC: %u, Voltage: %.3f V, Pressure: %.2f bar\n", raw, v, p);
}

int main()
{
    stdio_init_all();
    GPIO_init();
    adc_init();
    adc_gpio_init(27);
    adc_select_input(1);

    tps55288_init();
    disable_boost_circuit();

    printf("TPS55288 Control Interface Ready\n");

    while (1) 
    {
        int c = getchar_timeout_us(0); // non-blocking
        if (c == PICO_ERROR_TIMEOUT) {
            tight_loop_contents(); // idle
            continue;
        }

        switch (c) 
        {
            case 'e': case 'E': enable_boost_circuit(); 
                                break;
            
            case 'd': case 'D': disable_boost_circuit(); 
                                break;

            case 'm': case 'M': handle_measurement();
                                break;

            case 'v': case 'V': handle_voltage_set(); 
                                break;

            case 'g': case 'G': handle_voltage_get(); 
                                break;

            case '?': printf("ID: TPS55288 controller, FW v1.0\n"); 
                                break;

            default: printf("Unknown command: %c\n", c); 
                                break;
        }
    }
}
