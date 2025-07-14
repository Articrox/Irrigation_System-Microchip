#include "moisture_sensor.h"
//#include "core_cm0plus.h"
#include <stdio.h>

// External Helper Function Prototypes 
// (These should match the declarations in your driver files)
extern void ADC_ConversionStart(void);
extern bool ADC_ConversionStatusGet(void);
extern uint16_t ADC_ConversionResultGet(void);
//extern void uart_send_string(const char* str);
//extern uint32_t get_system_time_ms(void);


// Calibration and Conversion Function
void moisture_sensor_calibrate(MoistureSensorContext* context, 
                                uint16_t dry_calibration_value, 
                                uint16_t wet_calibration_value) {
    // Convert raw ADC value to percentage
    if (context->moisture_raw_value >= dry_calibration_value) {
        context->moisture_percentage = 0;
    } else if (context->moisture_raw_value <= wet_calibration_value) {
        context->moisture_percentage = 100;
    } else {
        // Linear interpolation between dry and wet values
        context->moisture_percentage = (
            (context->moisture_raw_value - dry_calibration_value) * 100 / 
            (wet_calibration_value - dry_calibration_value)
        );
    }
}

// Initialize the Moisture Sensor State Machine
void moisture_sensor_state_machine_init(MoistureSensorContext* context) {
    context->current_state = MOISTURE_STATE_IDLE;
    context->moisture_raw_value = 0;
    context->moisture_percentage = 0;
    context->measurement_start_time = 0;
    context->wait_timer_duration = 300; // Default 30 seconds between measurements
    context->conversion_complete = false;
}

// Main State Machine Run Function
void moisture_sensor_state_machine_run(MoistureSensorContext* context) {
    uint32_t current_time = systemTicks;
    
    switch (context->current_state) {
        case MOISTURE_STATE_IDLE:
            // Transition to start measurement
            context->current_state = MOISTURE_STATE_INIT_MEASUREMENT;
            break;

        case MOISTURE_STATE_INIT_MEASUREMENT:
            // Start ADC conversion (SW trigger)
            ADC_ConversionStart();
            context->measurement_start_time = current_time;
            context->current_state = MOISTURE_STATE_WAIT_CONVERSION;
            break;

        case MOISTURE_STATE_WAIT_CONVERSION:
            // Check if conversion is complete
            if (ADC_ConversionStatusGet()) {
                context->current_state = MOISTURE_STATE_PROCESS_DATA;
            }
            break;

        case MOISTURE_STATE_PROCESS_DATA:
            // Read moisture value from ADC
            context->moisture_raw_value = ADC_ConversionResultGet();
            input_voltage = context->moisture_raw_value * ADC_VREF / 4095U;
            
            // Perform moisture percentage conversion 
            // Note: You'll need to provide calibration values specific to your sensor
            moisture_sensor_calibrate(context, 
                                      dry_calibration_value,   // dry_calibration_value 
                                      wet_calibration_value); // wet_calibration_value

            context->current_state = MOISTURE_STATE_SEND_UART;
            break;

        case MOISTURE_STATE_SEND_UART:
            // Prepare and send UART message
            snprintf(context->uart_message_buffer, 
                     UART_BUFFER_SIZE, 
                     "Moisture: %d%% (Raw: %d)\r\n", 
                     context->moisture_percentage,
                     context->moisture_raw_value);
            
            
            printf(context->uart_message_buffer);
            snprintf(context->display_message_buffer,
                    UART_BUFFER_SIZE,
                    "ADC Count = 0x%x \n Vadc = %d.%03d V ",
                    context->moisture_raw_value,
                    (int)(input_voltage/1000),
                    (int)(input_voltage%1000));
            printf(context->display_message_buffer,
                    UART_BUFFER_SIZE,
                    "ADC Count = 0x%x \n Vadc = %d.%03d V ",
                    context->moisture_raw_value,
                    (int)(input_voltage/1000),
                    (int)(input_voltage%1000));
            
            context->measurement_start_time = current_time;
            context->current_state = MOISTURE_STATE_WAIT_TIMER;
            break;

        case MOISTURE_STATE_WAIT_TIMER:
            // Wait for specified duration before next measurement
            if (current_time - context->measurement_start_time >= context->wait_timer_duration) {
                context->current_state = MOISTURE_STATE_IDLE;
            }
            break;
    }
}