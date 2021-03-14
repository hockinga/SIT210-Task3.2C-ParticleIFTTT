// This #include statement was automatically added by the Particle IDE.
#include <BH1750Lib.h>


// Change these values
int pub_time = 7200000;         // Total sunlight exposure before publishing the initial event (millis), 7200000 = 2hrs
const int INT_TIME = 1200000;   // Sun exposure intervals required for each additional event after the initial event (millis), 1200000 = 20min
const int DELAY_TIME = 30000;   // Time between readings (millis), 30000 = 30s
const int LIGHT_THRESH = 10000; // Light level required to be considered sunlight (lux)


// Other constants and variables
const int BUILTIN_LED = D7;     // Builtin led pin location
BH1750Lib lightSensor;          // Initialise light sensor
int total_time = 0;             // Time terrarium has been exposed to sunlight (millis)
int prev_light = 0;             // Previous light reading (lux)
int prev_time = 0;              // Time elapsed at previous reading (millis)
bool is_sunlight = false;       // Current sunlight status (after each 2 readings: true = 2x pos, false = 2x neg, unchanged = 1x pos and neg)


/**
 * Sets up the builtin LED, begins the light sensor and exposes the total sunlight time to the cloud
 */
void setup() {

    // Setup builtin LED
    pinMode(D7, OUTPUT);

    // Start light sensor
    lightSensor.begin(BH1750LIB_MODE_CONTINUOUSHIGHRES);

    // Setup cloud GET access
    Particle.variable("current_sunlight_time", total_time);
}


/**
 * Repeatedly read, update and post the total sunlight exposure
 */
void loop() {

    // Wait for the next reading
    delay(DELAY_TIME);

    // Read light levels
    uint16_t light = lightSensor.lightLevel();

    // Updates sunlight and time variables
    update(light);

    // Notify of an update locally
    blink(BUILTIN_LED);
}


/**
 * Updates the total sunlight time, sunlight status, previous time and light variables and posts when necessary
 *
 * @param light the current sunlight reading (lux)
 */
void update(int light) {

    // Current time elapsed
    int current_time = millis();

    // Skip invalid readings
    if (!is_valid(light)) {
        return;
    }

    // Update sunlight status based on 2x recent readings
    update_is_sunlight(light);

    // Increment total time
    if (is_sunlight) {
        total_time += current_time - prev_time;

        // Publish if total exposure is reached
        if (total_time >= pub_time) {
            publish_time();
        }
    }

    // Update previous values
    update_prev(current_time, light);
}


/**
 * Publishes the total sun exposure time and sets up the required time for the next event
 */
void publish_time() {

    // Publish total time in minutes
    Particle.publish("sunlight_time", String(total_time / 60000), PRIVATE);

    // Increment required time for the next event
    pub_time += INT_TIME;
}


/**
 * Updates whether the device is considered to be detecting sunlight.
 * Following 2 readings above the light threshold, this is deemed true.
 * Following 2 reading below the light threshold, this is deemed false.
 * Following 2 mixed readings, the status is left unchanged until more data is received.
 *
 * @param light the current light reading (lux)
 */
void update_is_sunlight(int light) {

    // Update sunlight status after 2 consistent readings
    if (light >= LIGHT_THRESH && prev_light >= LIGHT_THRESH) {

        // 2x pos readings
        is_sunlight = true;

    } else if (light < LIGHT_THRESH && prev_light < LIGHT_THRESH) {

        // 2x neg readings
        is_sunlight = false;

    }

    // Leave unchanged otherwise
}


/**
 * Blinks an LED for one second
 *
 * @param led the LED to blink
 */
void blink(int led) {
    digitalWrite(led, HIGH);
    delay(1000);
    digitalWrite(led, LOW);
}


/**
 * Updates the previous time and light variables
 *
 * @param current_time the new time elapsed (millis)
 * @param light the new light reading (lux)
 */
void update_prev(int current_time, int light) {
    prev_time = current_time;
    prev_light = light;
}


/**
 * Returns whether or not a light reading is a valid number
 *
 * @param light the light reading (lux)
 * @return whether or not the light reading is a valid value
 */
bool is_valid(int light) {

    // 54612 is a known error value
    return light != 54612;
}