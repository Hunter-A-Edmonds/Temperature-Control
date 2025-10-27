// Include necessary libraries
#include <TM1637Display.h> // Library for the 4-digit 7-segment display
#include <max6675.h>      // Thermocouple Library

// --- Pin Definitions ---
// TM1637 Display 1 Pins
const int CLK1 = A4; // Clock pin for the first TM1637
const int DIO1 = A5; // Data I/O pin for the first TM1637
const int CLK2 = 8; // Clock pin for the second TM1637
const int DIO2 = 9; // Data I/O pin for the second TM1637

const int thermocoupleSO = 12; // SPI MISO
const int thermocoupleCS = 10; // SPI Chip Select
const int thermocoupleSCK = 13; // SPI Clock

const int switchPin = 2;  // Pin for the mode selection switch
const int potPin = A0;    // Analog pin for potentiometer
const int relayPin = 7;   // PWM pin for Solid State Relay

// --- TM1637 Display Instances ---
TM1637Display display1(CLK1, DIO1);
TM1637Display display2(CLK2, DIO2);

// --- Manual Segment Definitions for Characters ---
const uint8_t SEG_E_ = 0b01111001; // Character 'E'
const uint8_t SEG_r_ = 0b01010000; // Character 'r'
const uint8_t SEG_C_ = 0b00111001; // Character 'C'
const uint8_t errMessage[] = {SEG_E_, SEG_r_, SEG_r_};

// --- Thermocouple Configuration ---
MAX6675 thermocouple(thermocoupleSCK, thermocoupleCS, thermocoupleSO);

// --- Global Variables ---
float selectedTempC = 25.0; // Default selected temperature in Celsius
float actualTempC = 0.0;    // Actual temperature from thermocouple
bool HEAT_STATUS = false; // Mode is now controlled directly by the switch

// Temperature range for potentiometer
const float minTempPot = 35.0;
const float maxTempPot = 300.0;

// --- Setup Function ---
void setup() {
  digitalWrite(switchPin, LOW);
  pinMode(switchPin, INPUT); // Set the switch pin as input
  pinMode(potPin, INPUT);
  pinMode(relayPin, OUTPUT);

  digitalWrite(relayPin, LOW);
  
  // Initialize both TM1637 displays
  display1.setBrightness(0x0f);
  display1.clear();
  display2.setBrightness(0x0f);
  display2.clear();

  // Check for thermocouple connection
  delay(500);
  if (isnan(thermocouple.readCelsius())) {
    display1.setSegments(errMessage); // Display "Err" on both screens
    display2.setSegments(errMessage);
    for (;;); // Halt execution
  }
}

// --- Main Loop ---
void loop() {
  handleSwitch(); // Check the state of the switch

  readTemperatures();

  if (HEAT_STATUS == true) {
    controlHeater();
  }
  else {
    handlePotentiometer();
    digitalWrite(relayPin, LOW);
  }
  updateDisplays(); // Update both displays

  delay(150); // Main loop delay
}

// --- Helper Functions ---

// Read the state of the switch to determine the mode
void handleSwitch() {
  if (digitalRead(switchPin) == HIGH)
  {
    HEAT_STATUS = true;
  }
  else  HEAT_STATUS = false;
}

// Read temperature from the thermocouple
void readTemperatures() {
  actualTempC = thermocouple.readCelsius();
}

// Adjust selected temperature with potentiometer
void handlePotentiometer() {
  selectedTempC = ((float)analogRead(potPin)*265.f/1024.f)+35.f;
}

// Control the heater with the relay
void controlHeater() {
  if (HEAT_STATUS == true) {
    if (actualTempC < selectedTempC - 0.5)
    {
      digitalWrite(relayPin, HIGH);
    }
    else
    {
    digitalWrite(relayPin, LOW);
    }
  }
  else
    {
    digitalWrite(relayPin, LOW);
    }
}

// Update both TM1637 displays
void updateDisplays() {

    int set_temp = (int)selectedTempC;
    uint8_t segments_c1[] = {SEG_C_};      
    display1.showNumberDec(set_temp, false, 3, 0);
    display1.setSegments(segments_c1, 1, 3);
    int actual_temp = (int)actualTempC;
    uint8_t segments_c2[] = {SEG_C_};
    display2.showNumberDec(actual_temp, false, 3, 0);
    display2.setSegments(segments_c2, 1, 3);
}