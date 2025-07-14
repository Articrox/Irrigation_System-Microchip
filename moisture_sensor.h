#ifndef MOISTURE_SENSOR_H
#define MOISTURE_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

// Moisture Sensor Configuration
#define MOISTURE_ADC_RESOLUTION    (4096)  // 12-bit resolution
#define UART_BUFFER_SIZE           (64)    // Buffer for UART message
#define ADC_VREF                (1650)   //1650 mV (1.65V)

extern volatile uint32_t systemTicks;
extern uint32_t input_voltage;
extern uint16_t dry_calibration_value;
extern uint16_t wet_calibration_value;
extern bool calibration_completed;
// Moisture Sensor State Machine States
typedef enum {
    MOISTURE_STATE_IDLE,
    MOISTURE_STATE_INIT_MEASUREMENT,
    MOISTURE_STATE_WAIT_CONVERSION,
    MOISTURE_STATE_PROCESS_DATA,
    MOISTURE_STATE_SEND_UART,
    MOISTURE_STATE_WAIT_TIMER
} MoistureSensorState;

// Moisture Sensor Context Structure
typedef struct {
    MoistureSensorState current_state;
    uint16_t moisture_raw_value;      // Raw 12-bit ADC value
    uint16_t moisture_percentage;     // Converted to percentage
    uint32_t measurement_start_time;
    uint32_t wait_timer_duration;     // Configurable measurement interval
    bool conversion_complete;
    char *uart_message_buffer;
    char *display_message_buffer;
} MoistureSensorContext;

// Function Prototypes
void moisture_sensor_state_machine_init(MoistureSensorContext* context);
void moisture_sensor_state_machine_run(MoistureSensorContext* context);
void moisture_sensor_calibrate(MoistureSensorContext* context, 
                                uint16_t dry_calibration_value, 
                                uint16_t wet_calibration_value);

#endif // MOISTURE_SENSOR_H