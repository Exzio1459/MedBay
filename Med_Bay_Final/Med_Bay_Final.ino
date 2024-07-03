#include <virtuabotixRTC.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// Define pin mappings for RTC, LCD, Keypad, Stepper, Servo, IR Sensor, and DFPlayer
const int RTC_CLK_PIN = 23;
const int RTC_DATA_PIN = 25;
const int RTC_RST_PIN = 27;
const int LCD_ADDRESS = 0x27;
const int LCD_WIDTH = 16;
const int LCD_HEIGHT = 2;
const int ROWS = 4;
const int COLS = 4;
const byte rowPins[ROWS] = {22, 24, 26, 28};
const byte colPins[COLS] = {30, 32, 34, 36};
const char hexaKeys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
#define dirPin 3    // Stepper direction pin
#define stepPin 2   // Stepper step pin
#define SERVO_PIN 7 // Servo pin
#define SERVO_PIN_2 6 // Second servo pin for liquid dispensing
#define IR_SENSOR_PIN A0      // IR sensor pin
#define LIQUID_DISPENSER_PIN 8 // Pin for liquid dispenser (replace with your pin)
#define DFPLAYER_RX 11
#define DFPLAYER_TX 10

// Define step counts for the slots (adjust based on your setup)
#define SLOT1_STEPS 67
#define SLOT2_STEPS 133
#define SLOT3_STEPS 196

#define SERVO_SPEED 4 // Adjust this value for desired rotation speed

// Create objects for RTC, LCD, Keypad, Servo, and DFPlayer
virtuabotixRTC myRTC(RTC_CLK_PIN, RTC_DATA_PIN, RTC_RST_PIN);
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_WIDTH, LCD_HEIGHT);
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
Servo myServo;
Servo liquidServo; // Second servo for liquid dispensing
SoftwareSerial softwareSerial(DFPLAYER_RX, DFPLAYER_TX);
DFRobotDFPlayerMini player;

// Variables to store entered values
int timers[4] = {0, 0, 0, 0}; // Array to store timers for each slot
int numPills[3] = {0, 0, 0};   // Array to store number of pills needed for each slot (without the fourth slot)
float mlValues[4] = {0.0, 0.0, 0.0, 0.0}; // Array to store ml values for each slot
int currentPosition = 0;       // Stepper motor current position
int objectCount = 0;            // Variable to store the count of detected objects

// Function prototypes
void displayTime();
void enterNumPills();
void enterTimers();
void enterml();
void dispensePills();
void dispenseFromSlots(int slotNumber);
void moveToPosition(int targetPosition);
void stopServo();
void dispenseLiquid(float mlValue);
void dispenseLiquidWithServo();
void playNotificationSound();

void setup()
{
    Serial.begin(9600);
    softwareSerial.begin(9600);

    myRTC.setDS1302Time(30, 13, 20, 3, 17, 2, 2024); // Set initial time
    lcd.init();
    lcd.backlight();

    // Define pins as outputs
    pinMode(dirPin, OUTPUT);
    pinMode(stepPin, OUTPUT);
    pinMode(IR_SENSOR_PIN, INPUT);
    pinMode(LIQUID_DISPENSER_PIN, OUTPUT); // Set liquid dispenser pin as output

    // Attach servos to pins
    myServo.attach(SERVO_PIN);
    myServo.write(0); // Initial position at 0 degrees

    liquidServo.attach(SERVO_PIN_2); // Attach second servo for liquid dispensing
    liquidServo.write(150); // Initial position at 0 degrees
    delay(1000); // Wait for 1 second before starting rotation

    // Initialize DFPlayer Mini
    if (!player.begin(softwareSerial)) {
        while (1) {
            Serial.println("Connecting to DFPlayer Mini failed!");
            delay(1000);
        }
    }
    Serial.println("Connected to DFPlayer Mini.");
    player.volume(30); // Set volume (0 - 30)

    // Move stepper to initial position (0)
    moveToPosition(0);
}

void loop()
{
    displayTime(); // Continuously update and display time

    char customKey = customKeypad.getKey(); // Check for keypad input
    if (customKey != NO_KEY)
    {
        switch (customKey)
        {
        case 'A':
            enterNumPills(); // Enter number of pills on A press
            break;
        case 'B':
            enterTimers(); // Enter timers on B press
            break;
        case 'C':
            enterml(); // Enter ml on C press
            break;
        case 'D':
            dispensePills(); // Dispense pills on D press
            playNotificationSound(); // Play notification sound after dispensing
            break;
        case '*':
            // Handle * key press (if needed)
            break;
        case '#':
            // Handle # key press (if needed)
            break;
        default:
            // Handle unexpected key presses here
            break;
        }
    }
}

void displayTime()
{
    myRTC.updateTime(); // Update time from RTC

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(myRTC.dayofmonth);
    lcd.print("/");
    lcd.print(myRTC.month);
    lcd.print("/");
    lcd.print(myRTC.year);
    lcd.setCursor(0, 1);
    lcd.print(myRTC.hours);
    lcd.print(":");
    lcd.print(myRTC.minutes);
    lcd.print(":");
    lcd.print(myRTC.seconds);
    delay(1000);
}

void enterNumPills()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No Of Pills");

    for (int i = 0; i < 3; i++) // Loop for 3 slots (instead of 4)
    {
        String slotMessage = "Slot " + String(i + 1) + ": ";
        lcd.setCursor(0, 1);
        lcd.print(slotMessage);
        String currentValue = "";
        char key;
        do
        {
            key = customKeypad.getKey();
            if (key != NO_KEY && isDigit(key))
            {
                currentValue += key;
                lcd.print(key);
            }
        } while (key != '#' && currentValue.length() < 5);
        numPills[i] = currentValue.toInt();
    }

    // Display entered number of pills
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No of pills set:");
    for (int i = 0; i < 3; i++) // Display for 3 slots (instead of 4)
    {
        String slotPills = "Slot " + String(i + 1) + ": " + String(numPills[i]);
        lcd.setCursor(0, i + 1);
        lcd.print(slotPills);
    }
    delay(3000);
}

void enterTimers()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter time");

    for (int i = 0; i < 4; i++)
    { // Loop for 4 slots
        String slotMessage = "Slot " + String(i + 1) + " : ";
        lcd.setCursor(0, 1);
        lcd.print(slotMessage);
        String currentValue = "";
        char key;
        do
        {
            key = customKeypad.getKey();
            if (key != NO_KEY && isDigit(key))
            {
                currentValue += key;
                lcd.print(key);
            }
        } while (key != '#' && currentValue.length() < 5);
        timers[i] = currentValue.toInt();
    }

    // Display entered timers
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Timers set:");
    for (int i = 0; i < 4; i++)
    { // Display for 4 slots
        String slotTimer = "Slot " + String(i + 1) + ": " + String(timers[i]) + "s";
        lcd.setCursor(0, i + 1);
        lcd.print(slotTimer);
    }
    delay(3000);
}

void enterml()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Amount in ml:");

    String slotMessage = "Slot 4: ";
    lcd.setCursor(0, 1);
    lcd.print(slotMessage);
    String currentValue = "";
    char key;
    do
    {
        key = customKeypad.getKey();
        if (key != NO_KEY && (isDigit(key) || key == '.')) // Allowing digits and decimal point
        {
            currentValue += key;
            lcd.print(key);
        }
    } while (key != '#' && currentValue.length() < 6); // Increase length to allow for decimal point
    mlValues[3] = currentValue.toFloat(); // Convert string to float and store in slot 4

    // Display entered ml value
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ml value set:");
    String slotMl = "Slot 4: " + String(mlValues[3]) + "ml";
    lcd.setCursor(0, 1);
    lcd.print(slotMl);
    delay(3000);
}

void dispensePills()
{
    for (int i = 0; i < 4; i++) { // Iterate over all slots
        if (timers[i] > 0) { // Check if a timer is set for the slot
            int slotNumber = i + 1;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Dispensing...");
            lcd.setCursor(0, 1);
            lcd.print("Slot " + String(slotNumber));

            delay(timers[i] * 1000); // Wait for the specified timer duration

            dispenseFromSlots(slotNumber); // Dispense pills for the current slot
        }
    }
}

void dispenseFromSlots(int slotNumber)
{
    int numPillsForSlot = (slotNumber <= 3) ? numPills[slotNumber - 1] : 0; // Use numPills[i] for slots 1-3, 0 for slot 4
    float mlValueForSlot = mlValues[slotNumber - 1];

    if (numPillsForSlot > 0 || mlValueForSlot > 0)
    {
        // Move stepper to the desired slot, except for slot 4
        if (slotNumber != 4)
        {
            int targetPosition = 0;
            switch (slotNumber)
            {
            case 1:
                targetPosition = SLOT1_STEPS;
                break;
            case 2:
                targetPosition = SLOT2_STEPS;
                break;
            case 3:
                targetPosition = SLOT3_STEPS;
                break;
            }

            moveToPosition(targetPosition);
        }

        if (numPillsForSlot > 0)
        {
            // Rotate servo based on IR sensor input and user value
            objectCount = 0; // Reset the object count
            while (objectCount < numPillsForSlot)
            {
                for (int angle = 0; angle <= 160; angle++)
                {
                    myServo.write(angle);
                    delay(SERVO_SPEED);
                    if (digitalRead(IR_SENSOR_PIN) == LOW)
                    {
                        objectCount++;
                        Serial.print("Object detected! Count: ");
                        Serial.println(objectCount);
                        delay(1000); // Wait for some time before continuing
                    }
                    if (objectCount == numPillsForSlot)
                    {
                        stopServo();
                        break;
                    }
                }

                for (int angle = 160; angle >= 0; angle--)
                {
                    myServo.write(angle);
                    delay(SERVO_SPEED);
                    if (digitalRead(IR_SENSOR_PIN) == LOW)
                    {
                        objectCount++;
                        Serial.print("Object detected! Count: ");
                        Serial.println(objectCount);
                        delay(1000); // Wait for some time before continuing
                    }
                    if (objectCount == numPillsForSlot)
                    {
                        stopServo();
                        break;
                    }
                }
            }

            stopServo();
            Serial.println("Pills dispensed for slot " + String(slotNumber) + ".");
        }

        if (mlValueForSlot > 0)
        {
            dispenseLiquidWithServo(); // Dispense liquid using the second servo
            Serial.println("Liquid dispensed for slot " + String(slotNumber) + ".");
        }
    }
}

void moveToPosition(int targetPosition)
{
    // Step the motor to the target position
    int stepsToMove = abs(targetPosition - currentPosition);
    for (int i = 0; i < stepsToMove; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(3000); // Adjust this delay for stepper speed
        digitalWrite(stepPin, LOW);
        delayMicroseconds(3000);
    }
    currentPosition = targetPosition;
    Serial.print("Stepper motor moved to position: ");
    Serial.println(currentPosition);
}

void stopServo()
{
    myServo.write(0); // Stop the servo
    Serial.println("Servo stopped.");
}

void dispenseLiquid(float mlValue)
{
    // Control the liquid dispenser pin based on the provided ml value
    digitalWrite(LIQUID_DISPENSER_PIN, HIGH); // Turn on the liquid dispenser
    delay(mlValue * 1000); // Delay based on the ml value (adjust the multiplier as needed)
    digitalWrite(LIQUID_DISPENSER_PIN, LOW); // Turn off the liquid dispenser
}

void dispenseLiquidWithServo()
{
    // Control the second servo to dispense liquid
    liquidServo.write(100); // Set the second servo to dispensing position
    delay(5000); // Adjust this delay as needed for liquid dispensing
    liquidServo.write(130); // Reset the second servo to initial position
}

void playNotificationSound()
{
    player.play(1); // Play sound effect 1 (adjust the number as needed)
    delay(4000); // Adjust delay between commands (optional)
}