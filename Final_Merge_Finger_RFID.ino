
#include <Adafruit_Fingerprint.h>
#include <ArduinoJson.h>

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial3);
StaticJsonDocument<64> doc; // Json Document for transmitting JSON Data to Serial Comm. Devices

String inputString = "";        // String to hold incoming serial data
boolean stringComplete = false; // Flag to indicate if a complete string is received

void jsonify(const String key, const String val)
{
  doc[key] = val;
}

void readCommand()
{
  while (Serial.available() > 0 && !stringComplete)
  {
    char incomingChar = Serial.read();
    if (incomingChar == '\r')
    {
      // Ignore '\r'
    }
    else if (incomingChar == '\n')
    {
      // If '\n' is received, mark the string as complete
      stringComplete = true;
    }
    else
    {
      // Accumulate characters in the inputString
      inputString += incomingChar;
    }
  }

  // If a complete string is received
  if (stringComplete)
  {
    // Perform actions based on the received string
    switchString(inputString);

    // Clear the inputString and reset the flag
    inputString = "";
    stringComplete = false;
  }
}

void enrollFinger()
{
  //
}

void verifyFinger()
{
  //
}

void registerRfidCard()
{
  //
}

void readRfidCard()
{
  //
}

void switchString(String str)
{
  // Compare the received string to predefined string literals
  if (str.equals("enroll"))
  {
    // Serial.println("Performing enrollment...");
  }
  else if (str.equals("verify"))
  {
    // Serial.println("Performing verification...");
  }
  else if (str.equals("register"))
  {
    // Serial.println("Performing registration...");
  }
  else if (str.equals("read"))
  {
    // Serial.println("Performing read operation...");
  }
}

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ; // For Yun/Leo/Micro/Zero/...
  delay(100);
  finger.begin(57600);

  if (!finger.verifyPassword())
  {
    doc["message"] = "Found fingerprint sensor";
  }
}

void loop()
{
  //
}