/**
 * @file pump_control.h
 * @brief Public API for controlling a DC pump via PWM and tracking volume dispensed.
 *
 * Assumes a TCC module is configured for PWM generation and a millisecond
 * timer (like SysTick) is available and provides GetTickMs().
 * Requires a suitable driver circuit (e.g., MOSFET) between MCU and pump.
 *
 * @note Requires calibration data specific to the pump and setup, defined
 * in pump_control.c.
 */

#ifndef PUMP_CONTROL_H
#define PUMP_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

// --- Configuration (Adjust in pump_control.c if needed) ---
// These determine which TCC peripheral and channel are used, and the PWM resolution.
// They are defined in pump_control.c but mentioned here for awareness.
// #define PUMP_TCC_INSTANCE    TCC0 // Example TCC instance (defined in .c)
// #define PUMP_TCC_CHANNEL     0    // Example TCC channel (defined in .c)
// #define PUMP_PWM_PERIOD      1199 // Example PWM Period (defined in .c)


// --- Public API Functions ---

/**
 * @brief Initializes the pump control module.
 * Ensures the TCC peripheral is configured and enabled by the framework.
 * Sets the initial pump state to OFF and resets volume tracking.
 * Must be called once at startup.
 */
void pump_init(void);

/**
 * @brief Activates the pump or adjusts its flow to a specific level.
 * @param percentage Desired flow level as a percentage (0.0 to 100.0).
 * 0.0% turns the pump off. Values outside the range are clamped.
 */
void pump_activate(float percentage);

/**
 * @brief Deactivates the pump (sets PWM duty cycle to 0%).
 * Equivalent to calling pump_activate(0.0f).
 */
void pump_deactivate(void);

/**
 * @brief Adjusts the pump flow to the specified percentage.
 * This is functionally the same as pump_activate().
 * @param percentage Desired flow level (0.0 to 100.0).
 */
void pump_adjust_flow(float percentage);

/**
 * @brief Gets the current operational status of the pump.
 * @return true if the pump's target duty cycle is > 0, false otherwise.
 */
bool pump_get_status(void);

/**
 * @brief Gets the total estimated volume of liquid dispensed since the last reset.
 * Includes volume dispensed in the current interval if the pump is running.
 * Accuracy depends heavily on the calibration data in pump_control.c.
 * @return Total volume dispensed in milliliters (mL).
 */
float pump_get_total_volume_ml(void);

/**
 * @brief Resets the accumulated volume counter back to zero.
 * Call this periodically (e.g., weekly) as needed by the application logic.
 */
void pump_reset_total_volume(void);


#endif // PUMP_CONTROL_H
