/**
 * @file pump_control.c
 * @brief Implementation for controlling a DC pump via PWM and tracking volume.
 */

#include "pump_control.h" // Include the public API header

#include <stdio.h> // For printf debugging (optional, ensure UART is set up)
#include <math.h>  // For fabs in interpolation

// --- Atmel Start / Harmony Includes (Adapt as necessary) ---
// Ensure these match your project's framework includes.
#include <atmel_start.h> // Or your specific Harmony/ASF top-level include
#include <hal_tcc.h>    // Header for TCC functions

// --- Configuration (MUST BE ADJUSTED based on MHC/START setup) ---
// These MUST match the TCC peripheral settings configured in Harmony/MCC

// Define which TCC instance is used for the pump PWM
#define PUMP_TCC_INSTANCE    TCC0 // Example: Using TCC0 instance

// Define the TCC channel (Waveform Output - WO[x]) connected to the pump driver pin.
// This corresponds to the Compare Channel (CC[x]) register used.
#define PUMP_TCC_CHANNEL     0    // Example: Using CC[0] and WO[0]

// Define the PWM Period value set in the TCC configuration (PER register).
// This determines the PWM frequency and resolution.
// Calculation: PER = (TCC_Clock_Hz / Target_PWM_Frequency_Hz) - 1
// Example: TCC Clock = 6MHz (48MHz/8). Target PWM Freq = 5kHz.
// PER = (6,000,000 / 5000) - 1 = 1199
#define PUMP_PWM_PERIOD      1199

// --- Calibration Data ---
// Structure to hold a calibration point (Duty Cycle vs. Flow Rate)
typedef struct {
    float duty_cycle_percent;   // PWM Duty Cycle (%)
    float flow_rate_ml_per_sec; // Measured Flow Rate (mL/second) at this duty cycle
} PumpCalibrationPoint;

// !! IMPORTANT !!
// Replace these example values with your actual measured calibration data!
#define NUM_CALIBRATION_POINTS 5
static const PumpCalibrationPoint calibration_table[NUM_CALIBRATION_POINTS] = {
    // { Duty Cycle %, Flow Rate mL/s }
    { 20.0f, 0.8f },   // Example: At 20% duty, measured 0.8 mL/sec
    { 40.0f, 1.9f },   // Example: At 40% duty, measured 1.9 mL/sec
    { 60.0f, 3.1f },   // Example: At 60% duty, measured 3.1 mL/sec
    { 80.0f, 4.5f },   // Example: At 80% duty, measured 4.5 mL/sec
    { 100.0f, 5.8f }   // Example: At 100% duty, measured 5.8 mL/sec
};

// --- Module State Variables (Private) ---
static bool pump_is_active = false;                 // Is the pump currently supposed to be running?
static uint32_t current_pump_cc_value = 0;          // Current TCC Compare Channel value (0 to PUMP_PWM_PERIOD)
static float total_volume_dispensed_ml = 0.0f;      // Accumulated volume since last reset
static uint32_t pump_run_start_ms = 0;              // Timestamp (in ms) when the current run interval started
static bool is_tracking_pump_run = false;           // Flag indicating if we are currently timing a run interval

// --- External Dependencies (Assumed to exist) ---
// Requires a function returning monotonically increasing milliseconds.
// Ensure SysTick or another timer is configured to provide this.
extern uint32_t GetTickMs(void); // Provided by your system (e.g., SysTick handler)

// --- Private Helper Functions ---

/**
 * @brief Gets the estimated flow rate for a given duty cycle using the calibration table.
 * @param duty_cycle_percent The duty cycle percentage (0-100).
 * @return Estimated flow rate in mL/second. Performs linear interpolation/extrapolation.
 */
static float get_flow_rate_ml_per_sec(float duty_cycle_percent) {
    // Handle edge cases: below min or above max calibration point
    if (duty_cycle_percent <= calibration_table[0].duty_cycle_percent) {
        // Linearly extrapolate from the lowest point down to 0%
         if (calibration_table[0].duty_cycle_percent > 0.01f) { // Avoid division by zero
             return calibration_table[0].flow_rate_ml_per_sec * (duty_cycle_percent / calibration_table[0].duty_cycle_percent);
         } else {
             return 0.0f; // If lowest point is 0% or less, flow is 0
         }
    }
    if (duty_cycle_percent >= calibration_table[NUM_CALIBRATION_POINTS - 1].duty_cycle_percent) {
        // Above highest point, return the max calibrated flow rate
        return calibration_table[NUM_CALIBRATION_POINTS - 1].flow_rate_ml_per_sec;
    }

    // Find the two calibration points surrounding the target duty cycle for interpolation
    for (int i = 0; i < NUM_CALIBRATION_POINTS - 1; i++) {
        if (duty_cycle_percent >= calibration_table[i].duty_cycle_percent && duty_cycle_percent <= calibration_table[i+1].duty_cycle_percent) {
            // Interpolate between point i and i+1
            float d1 = calibration_table[i].duty_cycle_percent;
            float r1 = calibration_table[i].flow_rate_ml_per_sec;
            float d2 = calibration_table[i+1].duty_cycle_percent;
            float r2 = calibration_table[i+1].flow_rate_ml_per_sec;

            // Avoid division by zero if calibration points are identical
            if (fabs(d2 - d1) < 0.01f) {
                 return r1;
            }
            // Linear interpolation formula: R = R1 + ((D - D1) * (R2 - R1)) / (D2 - D1)
            return r1 + ((duty_cycle_percent - d1) * (r2 - r1)) / (d2 - d1);
        }
    }
    // Fallback (should not be reached if logic is correct)
    return calibration_table[NUM_CALIBRATION_POINTS - 1].flow_rate_ml_per_sec;
}

/**
 * @brief Calculates the current duty cycle percentage based on the CC value.
 * @return Duty cycle percentage (0.0 to 100.0).
 */
static float get_current_duty_percentage(void) {
    // PWM Duty Cycle = CCx / (PER + 1)
    if ((PUMP_PWM_PERIOD + 1) == 0) return 0.0f; // Avoid division by zero
    // Ensure calculation uses float for precision
    return ((float)current_pump_cc_value * 100.0f) / (float)(PUMP_PWM_PERIOD + 1);
}

/**
 * @brief Starts tracking the volume for the current pump run interval.
 * Records the start time using GetTickMs().
 */
static void pump_start_tracking(void) {
    // Only start tracking if the pump is supposed to be active and we aren't already tracking
    if (pump_is_active && !is_tracking_pump_run) {
        pump_run_start_ms = GetTickMs(); // Record start time
        is_tracking_pump_run = true;
        // printf("DEBUG: Started volume tracking at %lu ms\n", (long unsigned int)pump_run_start_ms);
    }
}

/**
 * @brief Stops tracking the current interval, calculates the volume dispensed
 * during that interval, and adds it to the total accumulated volume.
 */
static void pump_stop_tracking(void) {
    if (is_tracking_pump_run) {
        uint32_t end_ms = GetTickMs();
        uint32_t elapsed_ms;

        // Calculate elapsed time, handling timer wrap-around (assuming 32-bit ms timer)
        if (end_ms >= pump_run_start_ms) {
            elapsed_ms = end_ms - pump_run_start_ms;
        } else {
            // Timer wrapped around (e.g., overflowed from 0xFFFFFFFF to 0)
            elapsed_ms = (0xFFFFFFFF - pump_run_start_ms) + end_ms + 1;
        }

        float elapsed_seconds = (float)elapsed_ms / 1000.0f;

        // Get the duty cycle % that was active during this run interval
        float active_duty_percent = get_current_duty_percentage();

        // Get the corresponding flow rate from calibration data
        float flow_rate = get_flow_rate_ml_per_sec(active_duty_percent);

        // Calculate volume dispensed during this interval
        float volume_interval_ml = flow_rate * elapsed_seconds;

        // Add to the total accumulated volume
        total_volume_dispensed_ml += volume_interval_ml;

        is_tracking_pump_run = false; // Mark tracking as stopped for this interval

        // Optional Debug Print
        printf("DEBUG: Tracked interval: %.3f s @ %.1f%% (%.3f mL/s). Added: %.3f mL. New Total: %.3f mL\n",
               elapsed_seconds, active_duty_percent, flow_rate, volume_interval_ml, total_volume_dispensed_ml);
    }
}


// --- Public API Function Implementations ---

void pump_init(void) {
    // Assumes TCC peripheral (e.g., TCC0) and GPIO pin muxing
    // are already configured and enabled by the framework (MCC/Harmony).

    // Ensure pump starts OFF by setting duty cycle (CC value) to 0.
    // The specific HAL function might vary slightly (e.g., tcc_set_pwm_compare).
    // Cast PUMP_TCC_INSTANCE to the required struct type if necessary.
    tcc_set_compare_value((struct tcc_module *)&PUMP_TCC_INSTANCE, PUMP_TCC_CHANNEL, 0);

    // Optional: Verify TCC counter is running (usually enabled by framework init)
    // if (!tcc_is_enabled(&PUMP_TCC_INSTANCE)) { /* Log error or handle */ }

    // Initialize state variables
    pump_is_active = false;
    current_pump_cc_value = 0;
    total_volume_dispensed_ml = 0.0f;
    is_tracking_pump_run = false;

    printf("Pump control initialized. TCC Instance: %p, Channel: %d, Period: %lu\n",
           (void *)&PUMP_TCC_INSTANCE, PUMP_TCC_CHANNEL, (long unsigned int)PUMP_PWM_PERIOD);
}

void pump_activate(float percentage) {
    uint32_t new_cc_value;

    // Sanitize input percentage (clamp between 0.0 and 100.0)
    if (percentage < 0.0f) percentage = 0.0f;
    if (percentage > 100.0f) percentage = 100.0f;

    // Calculate the required TCC Compare Channel (CCx) value based on percentage.
    // Duty Cycle = CCx / (PER + 1) => CCx = Duty Cycle * (PER + 1)
    // Add 0.5f for proper rounding before casting to integer.
    new_cc_value = (uint32_t)(((percentage / 100.0f) * (float)(PUMP_PWM_PERIOD + 1)) + 0.5f);

    // Clamp CC value to the maximum possible (which is PER, as counter goes 0..PER)
    if (new_cc_value > PUMP_PWM_PERIOD) {
         new_cc_value = PUMP_PWM_PERIOD;
    }
    // Ensure 100% really results in PER value, handling potential float rounding issues.
    if (percentage >= 99.99f && new_cc_value < PUMP_PWM_PERIOD) {
         new_cc_value = PUMP_PWM_PERIOD;
    }

    // --- Volume Tracking Logic ---
    // If the pump was running at a different speed, stop the previous tracking interval first.
    if (is_tracking_pump_run && new_cc_value != current_pump_cc_value) {
        pump_stop_tracking();
    }

    // --- Set PWM Duty Cycle ---
    // Update the TCC compare register using the HAL function.
    tcc_set_compare_value((struct tcc_module *)&PUMP_TCC_INSTANCE, PUMP_TCC_CHANNEL, new_cc_value);
    uint32_t old_cc_value = current_pump_cc_value; // Store old value for state change check
    current_pump_cc_value = new_cc_value;          // Update current CC value state

    // --- Update State & Start/Stop Tracking ---
    if (new_cc_value > 0) {
        // Pump should be active
        bool needs_tracking_start = !pump_is_active; // Start if previously inactive
        pump_is_active = true;

        // Start tracking if it's newly activated OR if the speed changed (stop was called above)
        if (needs_tracking_start || (old_cc_value != new_cc_value && !is_tracking_pump_run)) {
             pump_start_tracking();
        }
        // printf("Pump activated/adjusted to %.1f%% duty (CC=%lu)\n", get_current_duty_percentage(), (long unsigned int)current_pump_cc_value);

    } else {
        // Duty cycle is 0, pump should be off
        if (pump_is_active) {
            // If it was running, stop tracking (might have already been stopped if speed changed to 0)
             if (is_tracking_pump_run) {
                 pump_stop_tracking();
             }
            pump_is_active = false; // Mark as inactive
            // printf("Pump deactivated (0%% duty cycle).\n");
        }
        // Ensure tracking is stopped if percentage is 0
        is_tracking_pump_run = false;
    }
}

void pump_deactivate(void) {
    // Simply call activate with 0% to handle state and volume tracking consistently.
    pump_activate(0.0f);
}

void pump_adjust_flow(float percentage) {
    // This is functionally identical to activating at a new percentage.
    // printf("Adjusting pump flow to %.1f%%...\n", percentage);
    pump_activate(percentage);
}

bool pump_get_status(void) {
    // Returns the intended state based on the last command.
    return pump_is_active;
}

float pump_get_total_volume_ml(void) {
    float current_interval_volume = 0.0f;

    // If the pump is currently running, calculate the estimated volume dispensed
    // in the *current, ongoing* interval up to this exact moment.
    if (is_tracking_pump_run) {
        uint32_t current_ms = GetTickMs();
        uint32_t elapsed_ms;
        // Calculate elapsed time for the current interval, handling wrap-around
         if (current_ms >= pump_run_start_ms) {
            elapsed_ms = current_ms - pump_run_start_ms;
         } else {
            elapsed_ms = (0xFFFFFFFF - pump_run_start_ms) + current_ms + 1;
         }
         float elapsed_seconds = (float)elapsed_ms / 1000.0f;
         float active_duty_percent = get_current_duty_percentage();
         float flow_rate = get_flow_rate_ml_per_sec(active_duty_percent);
         current_interval_volume = flow_rate * elapsed_seconds;
    }

    // Return the total accumulated from completed intervals PLUS the calculated volume from the current running interval.
    return total_volume_dispensed_ml + current_interval_volume;
}

void pump_reset_total_volume(void) {
     // printf("Resetting total volume dispensed.\n");
     // If pump is currently running, stop the current tracking interval first
     // to add its contribution before resetting the total accumulator.
     if (is_tracking_pump_run) {
          pump_stop_tracking();
          // If the pump should continue running after the reset, we need
          // to immediately start tracking a new interval from time 0.
          pump_start_tracking(); // Restart tracking for the ongoing run
     }
    // Reset the accumulator for completed intervals.
    total_volume_dispensed_ml = 0.0f;
    // printf("Total volume reset to 0.0 mL.\n");
}
