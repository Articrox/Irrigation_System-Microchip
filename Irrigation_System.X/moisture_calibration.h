#ifndef MOISTURE_CALIBRATION_H
#define MOISTURE_CALIBRATION_H

#include <stdint.h>
#include <stdbool.h>
#define CALIBRATION_FLASH_ADDRESS ((uint32_t)0x00001000) // Replace with your actual address
// Magic number for data validation (optional but recommended)
#define CALIBRATION_MAGIC_NUMBER 0xCA11B8A7 // Changed magic number for distinction

// Calibration States
typedef enum {
    CALIBRATION_IDLE,
    CALIBRATION_DRY_WAIT,
    CALIBRATION_DRY_RECORD,
    CALIBRATION_WET_WAIT,
    CALIBRATION_WET_RECORD,
    CALIBRATION_COMPLETE
} CalibrationState;

// Calibration Context Structure
typedef struct {
    CalibrationState current_state;
    uint16_t dry_calibration_value;
    uint16_t wet_calibration_value;
    uint8_t calibration_attempts;
} CalibrationContext;

// Button state tracking
typedef enum {
    BUTTON_RELEASED,
    BUTTON_PRESSED,
    BUTTON_HANDLED
} ButtonState;

static ButtonState current_button_state = BUTTON_RELEASED;

// Function Prototypes
void calibration_init(void);
bool calibration_process(void);
bool get_calibration_status(void);
void get_calibration_values(uint16_t* dry_value, uint16_t* wet_value);

#endif // MOISTURE_CALIBRATION_H