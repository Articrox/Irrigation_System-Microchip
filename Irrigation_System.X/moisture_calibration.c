#include "moisture_calibration.h"
#include "sam.h"
#include "peripheral/port/plib_port.h"
#include "definitions.h" // Or your specific NVM header
#include <string.h> // For memory operations if needed

// External function prototypes

extern void ADC_ConversionStart(void);
extern uint16_t ADC_ConversionResultGet(void);

// Global calibration context
static CalibrationContext calibration_ctx;
static bool calibration_complete = false;


// Check if button is pressed (active low)
static bool is_button_pressed(void) {
    // Adjust pin number as needed
    return SW0_Get();
}

// Function to save the relevant calibration data to flash
bool save_calibration_data(const CalibrationContext *calibration_data) {
    // We only want to save the dry and wet calibration values
    // along with the magic number for validation.
    struct {
        uint16_t dry_value;
        uint16_t wet_value;
        uint32_t magic_number;
    } data_to_save;  // a bit of a mystery, research more **
    //alt route, make a buffer array of the page size and initialize at 0 and
    //copy my values on buffer
    //how would this change by using arrays instead of structs

    data_to_save.dry_value = calibration_data->dry_calibration_value;
    data_to_save.wet_value = calibration_data->wet_calibration_value;
    data_to_save.magic_number = CALIBRATION_MAGIC_NUMBER;

    // Wait for any ongoing NVM operations to complete
    while (NVMCTRL_IsBusy());

    // Calculate the row address containing our storage address
    uint32_t row_address = CALIBRATION_FLASH_ADDRESS & ~(NVMCTRL_FLASH_ROWSIZE - 1);

    // Erase the flash row where we want to save the data
    NVMCTRL_RowErase(row_address);
    while (NVMCTRL_IsBusy());

    // Write the calibration data
    NVMCTRL_PageWrite((uint32_t *)&data_to_save, CALIBRATION_FLASH_ADDRESS);
    while (NVMCTRL_IsBusy());

    // Optionally, verify the write
    struct {
        uint16_t dry_value;
        uint16_t wet_value;
        uint32_t magic_number;
    } verified_data;
    memcpy(&verified_data, (const void *)CALIBRATION_FLASH_ADDRESS, sizeof(verified_data));

    if (verified_data.magic_number == CALIBRATION_MAGIC_NUMBER &&
        verified_data.dry_value == calibration_data->dry_calibration_value &&
        verified_data.wet_value == calibration_data->wet_calibration_value) {
        printf("Calibration data saved to flash.\r\n");
        return true;
    } else {
        printf("Error saving calibration data to flash!\r\n");
        return false;
    }
}
// Initialize calibration routine
void calibration_init(void) {
    // Initialize calibration context
    calibration_ctx.current_state = CALIBRATION_DRY_WAIT;
    calibration_ctx.dry_calibration_value = 0;
    calibration_ctx.wet_calibration_value = 0;
    calibration_ctx.calibration_attempts = 0;
    calibration_complete = false;
    current_button_state = BUTTON_RELEASED;
    
      // Attempt to load calibration data from flash
    if (load_calibration_data(&calibration_ctx)) {
        calibration_complete = true;
        printf("Using loaded calibration values (Dry: %d, Wet: %d).\r\n",
               calibration_ctx.dry_calibration_value, calibration_ctx.wet_calibration_value);
        calibration_ctx.current_state = CALIBRATION_COMPLETE; // Optionally skip calibration
    } else {
        printf("Starting new calibration.\r\n");
    }
}

// Main calibration process
bool calibration_process(void) {
    // Get current ADC value
    ADC_ConversionStart();
    uint16_t current_adc_value = ADC_ConversionResultGet();
    
    // Button state management
    bool button_currently_pressed = is_button_pressed();
    
    switch (calibration_ctx.current_state) {
        case CALIBRATION_DRY_WAIT:
            // Only print message once when entering this state
            if (current_button_state == BUTTON_RELEASED) {
                printf("Place sensor in DRY condition and press button\r\n");
                current_button_state = BUTTON_PRESSED;
            }
            if (button_currently_pressed && current_button_state == BUTTON_PRESSED) {
                // Wait for button release
                if (!is_button_pressed()) {
                    // Small delay to debounce
                    for(volatile int i = 0; i < 50000; i++) {}
                    
                    // Record dry calibration value
                    calibration_ctx.dry_calibration_value = current_adc_value;
                    calibration_ctx.current_state = CALIBRATION_WET_WAIT;
                    current_button_state = BUTTON_RELEASED;
                    
                    // Send confirmation
                    printf("Dry calibration recorded: %d\r\n", calibration_ctx.dry_calibration_value);
                }
            }
            break;
        
        case CALIBRATION_WET_WAIT:
             // Only print message once when entering this state
            if (current_button_state == BUTTON_RELEASED) {
                printf("Place sensor in WET condition and press button\r\n");
                current_button_state = BUTTON_PRESSED;
            }
            
            // Wait for button press and release
            if (button_currently_pressed && current_button_state == BUTTON_PRESSED) {
                // Wait for button release
                if (!is_button_pressed()) {
                    // Small delay to debounce
                    for(volatile int i = 0; i < 50000; i++) {}
                    
                    // Record wet calibration value
                    calibration_ctx.wet_calibration_value = current_adc_value;
                    calibration_ctx.current_state = CALIBRATION_COMPLETE;
                    current_button_state = BUTTON_RELEASED;
                    
                    // Send confirmation
                    printf("Wet calibration recorded: %d\r\n", calibration_ctx.wet_calibration_value);
                }
            }
            break;
        
        case CALIBRATION_COMPLETE:
             // Validate calibration values
            if (calibration_ctx.wet_calibration_value < calibration_ctx.dry_calibration_value) {
                printf("Calibration successful!\r\n");
                printf("Dry value: %d, Wet value: %d\r\n", 
                       calibration_ctx.dry_calibration_value, 
                       calibration_ctx.wet_calibration_value);
                calibration_complete = true;
                
                // Save the calibration data to flash
                save_calibration_data(&calibration_ctx);
                
                return true;
            } else {
                // Invalid calibration
                printf("Calibration failed. Retry.\r\n");
                calibration_ctx.current_state = CALIBRATION_DRY_WAIT;
                calibration_complete = false;
                current_button_state = BUTTON_RELEASED;
            }
            break;
        
        default:
            calibration_ctx.current_state = CALIBRATION_DRY_WAIT;            
            current_button_state = BUTTON_RELEASED;
            break;
    }
    
    return false;
}

// Get calibration status
bool get_calibration_status(void) {
    return calibration_complete;
}

// Retrieve calibration values
void get_calibration_values(uint16_t* dry_value, uint16_t* wet_value) {
    if (dry_value) *dry_value = calibration_ctx.dry_calibration_value;
    if (wet_value) *wet_value = calibration_ctx.wet_calibration_value;
}

// Function to load the calibration data from flash into the CalibrationContext
bool load_calibration_data(CalibrationContext *calibration_data) {
    struct {
        uint16_t dry_value;
        uint16_t wet_value;
        uint32_t magic_number;
    } loaded_data;
    memcpy(&loaded_data, (const void *)CALIBRATION_FLASH_ADDRESS, sizeof(loaded_data));

    if (loaded_data.magic_number == CALIBRATION_MAGIC_NUMBER) {
        calibration_data->dry_calibration_value = loaded_data.dry_value;
        calibration_data->wet_calibration_value = loaded_data.wet_value;
        printf("Calibration data loaded from flash.\r\n");
        return true;
    } else {
        printf("No valid calibration data found in flash.\r\n");
        return false;
    }
}