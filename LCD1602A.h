#ifndef IRRIGATION_SYSTEM_H
#define IRRIGATION_SYSTEM_H

#include <stdint.h>
#include "../src/config/default/peripheral/port/plib_port.h"
// LCD pin definitions - adjust according to your specific connections
//#define LCD_RS_PIN      PIN_PA08  // Register Select pin
//#define LCD_EN_PIN      PIN_PA09  // Enable pin
//#define LCD_D4_PIN      LCD_D4_PIN  // Data pin 4
//#define LCD_D5_PIN      LCD_D5_PIN  // Data pin 5
//#define LCD_D6_PIN      LCD_D6_PIN  // Data pin 6
//#define LCD_D7_PIN      LCD_D7_PIN  // Data pin 7

// Moisture sensor pin (analog)
#define MOISTURE_SENSOR_PIN PIN_PA05  // A0 on SAMD21 Nano

// LCD commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// LCD flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// LCD flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// LCD flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// LCD flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// Plant Moisture Thresholds Lookup Table
typedef struct {
    const char* name;
    int moisture_low;  // Lower threshold (%)
    int moisture_ideal_low;  // Lower bound of ideal range (%)
    int moisture_ideal_high; // Upper bound of ideal range (%)
    int moisture_high;  // Upper threshold (%)
} PlantMoistureThresholds;

// Moisture status enumeration
typedef enum {
    MOISTURE_TOO_LOW,
    MOISTURE_IDEAL,
    MOISTURE_TOO_HIGH,
    PLANT_NOT_FOUND
} MoistureStatus;

// LCD functions
void lcd_init(void);
void lcd_command(uint8_t command);
void lcd_write(uint8_t value);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_print(const char* str);
void lcd_clear(void);
void lcd_home(void);
void lcd_display(void);
void lcd_no_display(void);

// ADC/Sensor functions
void setupADC(void);
int readMoistureSensor(void);
int map(int x, int in_min, int in_max, int out_min, int out_max);

// Plant monitoring functions
MoistureStatus getMoistureStatus(const char* plant_name, int moisture_percent);
void updateMoistureStatusDisplay(const char* plant_name, int moisture_percent);
void cyclePlantSelection(void);
void displayMessage(const char* format, ...);

// External variables
extern const PlantMoistureThresholds PLANT_THRESHOLDS[];
extern int current_plant_index;

#endif // IRRIGATION_SYSTEM_H