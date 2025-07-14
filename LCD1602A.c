#include "LCD1602A.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "sys/time.h"
#include "sam.h"
#include "Plants_definitions.h"
//#include "de"

// LCD display control states
uint8_t _displaycontrol;
uint8_t _displaymode;



// Current plant selection (default to first plant)
int current_plant_index = 0;

// Number of plants in the lookup table
#define NUM_PLANTS (sizeof(PLANT_THRESHOLDS) / sizeof(PLANT_THRESHOLDS[0]))

// Helper functions for LCD
static void lcd_pulse_enable(void) {
    LCD_EN_Set();
//    PORT->Group[0].OUTSET.reg = (1ul << LCD_EN_PIN);
    delay_us(1);    // Enable pulse must be > 450ns
    LCD_EN_Clear();
//    PORT->Group[0].OUTCLR.reg = (1ul << LCD_EN_PIN);
    delay_us(100);  // Commands need > 37us to settle
}

static void lcd_write4bits(uint8_t value) {
     // Set individual data pins
    if (value & 0x01)
        LCD_D4_Set();
    else
        LCD_D4_Clear();
        
    if (value & 0x02)
        LCD_D5_Set();
    else
        LCD_D5_Clear();
        
    if (value & 0x04)
        LCD_D6_Set();
    else
        LCD_D6_Clear();
        
    if (value & 0x08)
        LCD_D7_Set();
    else
        LCD_D7_Clear();
    
    lcd_pulse_enable();
}

static void lcd_send(uint8_t value, uint8_t mode) {
    if (mode)
        LCD_RS_Set();
    else
        LCD_RS_Clear();
        
    // Write in 4-bit mode
    lcd_write4bits(value >> 4);
    lcd_write4bits(value);
}

void lcd_command(uint8_t command) {
    lcd_send(command, 0);
}

void lcd_write(uint8_t value) {
    lcd_send(value, 1);
}

void lcd_clear(void) {
    lcd_command(LCD_CLEARDISPLAY);
    delay_ms(2); // This command takes a long time
}

void lcd_home(void) {
    lcd_command(LCD_RETURNHOME);
    delay_ms(2); // This command takes a long time
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    int row_offsets[] = { 0x00, 0x40 };
    if (row > 1) row = 1;
    lcd_command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void lcd_display(void) {
    _displaycontrol |= LCD_DISPLAYON;
    lcd_command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void lcd_no_display(void) {
    _displaycontrol &= ~LCD_DISPLAYON;
    lcd_command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void lcd_init(void) {
    // Configure pins as outputs

    // Wait for LCD to initialize
    delay_ms(50);
    
    // Start in 8-bit mode, try to set to 4-bit mode
//    PORT->Group[0].OUTCLR.reg = (1ul << LCD_RS_PIN);
//    PORT->Group[0].OUTCLR.reg = (1ul << LCD_EN_PIN);
//    
    // Initialization sequence
    lcd_write4bits(0x03);  // Function set: 8-bit interface
    delay_ms(5);           // Wait min 4.1ms
    
    lcd_write4bits(0x03);  // Function set: 8-bit interface
    delay_us(150);         // Wait min 100us
    
    lcd_write4bits(0x03);  // Function set: 8-bit interface
    delay_us(150);         // Wait min 100us
    
    lcd_write4bits(0x02);  // Function set: set to 4-bit interface
    delay_us(150);         // Wait min 100us
    
    // Now in 4-bit mode, set up the display
    lcd_command(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);
    
    // Turn on the display with no cursor or blinking
    _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    lcd_display();
    
    // Initialize display mode
    _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    lcd_command(LCD_ENTRYMODESET | _displaymode);
    
    // Clear the display
    lcd_clear();
}

void lcd_print(const char* str) {
    while (*str) {
        lcd_write(*str++);
    }
}


// Helper mapping function
int map(int x, int in_min, int in_max, int out_min, int out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Function to get moisture status for a given plant and moisture reading
MoistureStatus getMoistureStatus(const char* plant_name, int moisture_percent) {
    // Search for the plant in our lookup table
    for (int i = 0; i < NUM_PLANTS; i++) {
        if (strcmp(PLANT_THRESHOLDS[i].name, plant_name) == 0) {
            if (moisture_percent < PLANT_THRESHOLDS[i].moisture_low) {
                return MOISTURE_TOO_LOW;
            } else if (moisture_percent > PLANT_THRESHOLDS[i].moisture_high) {
                return MOISTURE_TOO_HIGH;
            } else if (moisture_percent >= PLANT_THRESHOLDS[i].moisture_ideal_low && 
                      moisture_percent <= PLANT_THRESHOLDS[i].moisture_ideal_high) {
                return MOISTURE_IDEAL;
            } else {
                // In the transition zones (approaching ideal but not quite there)
                return MOISTURE_IDEAL; 
            }
        }
    }
    
    // Plant not found in lookup table
    return PLANT_NOT_FOUND;
}

// Function to display status message on LCD
void displayMessage(const char* format, ...) {
    char buffer[33]; // 16x2 LCD can display 32 characters plus null terminator
    va_list args;
    
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    lcd_clear();
    
    // Handle multi-line display
    char* line2 = NULL;
    for (int i = 0; buffer[i] != '\0'; i++) {
        if (buffer[i] == '\n') {
            buffer[i] = '\0';
            line2 = &buffer[i + 1];
            break;
        }
    }
    
    lcd_set_cursor(0, 0);
    lcd_print(buffer);
    
    if (line2) {
        lcd_set_cursor(0, 1);
        lcd_print(line2);
    }
}

void updateMoistureStatusDisplay(const char* plant_name, int moisture_percent) {
    MoistureStatus status = getMoistureStatus(plant_name, moisture_percent);
    
    switch (status) {
        case MOISTURE_TOO_LOW:
            displayMessage("%s: %d%%\nTOO DRY! WATER", plant_name, moisture_percent);
            break;
        case MOISTURE_IDEAL:
            displayMessage("%s: %d%%\nMOISTURE IDEAL", plant_name, moisture_percent);
            break;
        case MOISTURE_TOO_HIGH:
            displayMessage("%s: %d%%\nTOO WET!", plant_name, moisture_percent);
            break;
        case PLANT_NOT_FOUND:
            displayMessage("Unknown plant:\n%s", plant_name);
            break;
    }
}

void cyclePlantSelection() {
    // Move to next plant
    current_plant_index = (current_plant_index + 1) % NUM_PLANTS;
}