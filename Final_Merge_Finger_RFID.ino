#include <Adafruit_Fingerprint.h>
#include <ArduinoJson.h>

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial3);
StaticJsonDocument<64> jsonDoc; // Json Document for transmitting JSON Data to Serial Comm. Devices

String inputString = "";        // String to hold incoming serial data
boolean stringComplete = false; // Flag to indicate if a complete string is received

boolean fingerPrintConnected = false;
uint16_t id;

void sendJsonResponse(const char *action, const char *message, uint16_t id = UINT16_MAX)
{
  StaticJsonDocument<64> jsonDoc;
  jsonDoc["action"] = action;
  jsonDoc["message"] = message;

  if (id >= 0 && id != UINT16_MAX)
  {
    jsonDoc["id"] = id;
  }

  serializeJson(jsonDoc, Serial);
  Serial.println();
}

uint16_t enrollFinger()
{
  int p = -1;
  sendJsonResponse("enroll", "Enrolling fingerprint now...");
  sendJsonResponse("enroll", "Place finger on scanner");

  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      sendJsonResponse("enroll", "Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      sendJsonResponse("enroll", "Place finger");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      sendJsonResponse("enroll", "Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      sendJsonResponse("enroll", "Imaging error");
      break;
    default:
      sendJsonResponse("enroll", "Unknown error");
      break;
    }
  }

  p = finger.image2Tz(1);
  switch (p)
  {
  case FINGERPRINT_OK:
    sendJsonResponse("enroll", "Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    sendJsonResponse("enroll", "Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    sendJsonResponse("enroll", "Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    sendJsonResponse("enroll", "Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    sendJsonResponse("enroll", "Could not find fingerprint features");
    return p;
  default:
    sendJsonResponse("enroll", "Unknown error");
    return p;
  }

  sendJsonResponse("enroll", "Remove finger");
  delay(2000);
  p = 0;

  while (p != FINGERPRINT_NOFINGER)
  {
    p = finger.getImage();
  }

  sendJsonResponse("enroll", ("Enrolling ID #" + String(id)).c_str());
  p = -1;
  sendJsonResponse("enroll", "Place the same finger again");

  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      sendJsonResponse("enroll", "Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      sendJsonResponse("enroll", "Place finger again");
      continue;
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      sendJsonResponse("enroll", "Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      sendJsonResponse("enroll", "Imaging error");
      break;
    default:
      sendJsonResponse("enroll", "Unknown error");
      break;
    }
  }

  p = finger.image2Tz(2);
  switch (p)
  {
  case FINGERPRINT_OK:
    sendJsonResponse("enroll", "Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    sendJsonResponse("enroll", "Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    sendJsonResponse("enroll", "Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    sendJsonResponse("enroll", "Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    sendJsonResponse("enroll", "Could not find fingerprint features");
    return p;
  default:
    sendJsonResponse("enroll", "Unknown error");
    return p;
  }

  sendJsonResponse("enroll", ("Creating model for ID #" + String(id)).c_str());
  p = finger.createModel();

  if (p == FINGERPRINT_OK)
  {
    sendJsonResponse("enroll", "Prints matched!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    sendJsonResponse("enroll", "Communication error");
    return p;
  }
  else if (p == FINGERPRINT_ENROLLMISMATCH)
  {
    sendJsonResponse("enroll", "Fingerprints did not match");
    return p;
  }
  else
  {
    sendJsonResponse("enroll", "Unknown error");
    return p;
  }

  sendJsonResponse("enroll", ("ID #" + String(id)).c_str());

  p = finger.storeModel(id);

  if (p == FINGERPRINT_OK)
  {
    sendJsonResponse("enroll", "Stored!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    sendJsonResponse("enroll", "Communication error");
    return p;
  }
  else if (p == FINGERPRINT_BADLOCATION)
  {
    sendJsonResponse("enroll", "Could not store in that location");
    return p;
  }
  else if (p == FINGERPRINT_FLASHERR)
  {
    sendJsonResponse("enroll", "Error writing to flash");
    return p;
  }
  else
  {
    sendJsonResponse("enroll", "Unknown error");
    return p;
  }

  return 200;
}

uint16_t findFingerUser()
{
  uint16_t p = -1;
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      sendJsonResponse("verify", "Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      sendJsonResponse("verify", "No finger detected");
      continue;
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      sendJsonResponse("verify", "Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      sendJsonResponse("verify", "Imaging error");
      return p;
    default:
      sendJsonResponse("verify", "Unknown error");
      return p;
    }
  }

  // OK success!

  p = finger.image2Tz();
  switch (p)
  {
  case FINGERPRINT_OK:
    sendJsonResponse("verify", "Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    sendJsonResponse("verify", "Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    sendJsonResponse("verify", "Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    sendJsonResponse("verify", "Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    sendJsonResponse("verify", "Could not find fingerprint features");
    return p;
  default:
    sendJsonResponse("verify", "Unknown error");
    return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK)
  {
    if (finger.fingerID)
    {
      sendJsonResponse("verify", "Found a print match!");
    }
    else
    {
      sendJsonResponse("verify", "Did not find a match");
      return FINGERPRINT_NOTFOUND;
    }
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    sendJsonResponse("verify", "Communication error");
    return p;
  }
  else if (p == FINGERPRINT_NOTFOUND)
  {
    sendJsonResponse("verify", "Did not find a match");
    return p;
  }
  else
  {
    sendJsonResponse("verify", "Unknown error");
    return p;
  }

  // found a match!
  sendJsonResponse("verify", "Fingerprint Matched");

  return 200;
}

uint16_t verifyFinger()
{
  uint16_t p = -1;
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      sendJsonResponse("verify", "Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      sendJsonResponse("verify", "No finger detected");
      continue;
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      sendJsonResponse("verify", "Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      sendJsonResponse("verify", "Imaging error");
      return p;
    default:
      sendJsonResponse("verify", "Unknown error");
      return p;
    }
  }

  // OK success!

  p = finger.image2Tz();
  switch (p)
  {
  case FINGERPRINT_OK:
    sendJsonResponse("verify", "Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    sendJsonResponse("verify", "Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    sendJsonResponse("verify", "Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    sendJsonResponse("verify", "Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    sendJsonResponse("verify", "Could not find fingerprint features");
    return p;
  default:
    sendJsonResponse("verify", "Unknown error");
    return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK)
  {
    if (finger.fingerID == id)
    {
      sendJsonResponse("verify", "Found a print match!");
    }
    else
    {
      sendJsonResponse("verify", "Did not find a match");
      return FINGERPRINT_NOTFOUND;
    }
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    sendJsonResponse("verify", "Communication error");
    return p;
  }
  else if (p == FINGERPRINT_NOTFOUND)
  {
    sendJsonResponse("verify", "Did not find a match");
    return p;
  }
  else
  {
    sendJsonResponse("verify", "Unknown error");
    return p;
  }

  // found a match!
  sendJsonResponse("verify", "Fingerprint Matched");

  return 200;
}

void registerRfidCard()
{
  //
}

void readRfidCard()
{
  //
}

void switchString(String command)
{

  if (command == "enroll")
  {
    // Handle the "enroll" command
    uint16_t response = enrollFinger();
    if (response == 200)
    {
      sendJsonResponse("enroll", "Enrollment Successful", finger.fingerID);
    }
    else
    {
      sendJsonResponse("enroll", "Enrollment Failed");
    }
  }
  else if (command == "verify")
  {
    // Handle the "verify" command
    uint16_t response = verifyFinger();
    if (response == 200)
    {
      sendJsonResponse("verify", "Fingerprint matched", finger.fingerID);
    }
    else
    {
      sendJsonResponse("verify", "Verfication Failed");
    }
  }

  else if (command == "search")
  {
    uint16_t response = findFingerUser();

    if (response == 200)
    {
      sendJsonResponse("search", "Fingerprint matched", finger.fingerID);
    }
    else
    {
      sendJsonResponse("search", "verification Failed");
    }
  }
  else if (command == "register")
  {
    // Handle the "register" command
    sendJsonResponse("message", "Register command received");
    // Add your code for handling "register" here
  }
  else if (command == "read")
  {
    // Handle the "read" command
    sendJsonResponse("message", "Read command received");
    // Add your code for handling "read" here
  }
  else
  {
    // Invalid command
    sendJsonResponse("message", "Invalid command received");
    // You can add error handling or other actions here
  }
}

void readCommand()
{
  String command = Serial.readStringUntil('\n'); // Read the message until a newline character

  // Trim any leading or trailing whitespace
  command.trim();

  // Check the command against valid commands
  // Deserialize the JSON string
  if (command != "")
  {

    DeserializationError error = deserializeJson(jsonDoc, command);

    if (error)
    {
      sendJsonResponse("message", error.c_str());
      return;
    }

    command = jsonDoc["command"].as<String>();
    id = jsonDoc["id"].as<uint8_t>();

    switchString(command);
  }
}

bool checkFingerPrintConnection()
{
  if (!finger.verifyPassword())
  {
    fingerPrintConnected = false;
    sendJsonResponse("message", "Fingerprint sensor not found");

    return false;
  }

  fingerPrintConnected = true;
  return true;
}

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ; // For Yun/Leo/Micro/Zero/...
  delay(100);
  finger.begin(57600);

  checkFingerPrintConnection();
}

void loop()
{
  readCommand();
  /* if (checkFingerPrintConnection)
  {
    readCommand();
  } */
}