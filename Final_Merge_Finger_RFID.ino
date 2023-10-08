#include <Adafruit_Fingerprint.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>
// For RFID
#define SDA_DIO 9
#define RESET_DIO 8

MFRC522 rfid(SDA_DIO, RESET_DIO); // Create MFRC522 instance

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial3);
StaticJsonDocument<64> jsonDoc; // Json Document for transmitting JSON Data to Serial Comm. Devices

boolean fingerPrintConnected = false;
uint16_t id;
String cardUID = ""; // String to store the card's UID

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

void sendJsonResponseCard(const char *action, const char *message, String cardId = "")
{
  StaticJsonDocument<64> jsonDoc;
  jsonDoc["action"] = action;
  jsonDoc["message"] = message;

  if (cardId != "")
  {
    jsonDoc["id"] = cardId;
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
      sendJsonResponse("enroll", "Image Taken");
      break;
    case FINGERPRINT_NOFINGER:
      sendJsonResponse("enroll", "Place Finger");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      sendJsonResponse("enroll", "Communication Error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      sendJsonResponse("enroll", "Imaging Error");
      break;
    default:
      sendJsonResponse("enroll", "Unknown Error");
      break;
    }
  }

  p = finger.image2Tz(1);
  switch (p)
  {
  case FINGERPRINT_OK:
    sendJsonResponse("enroll", "Image Converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    sendJsonResponse("enroll", "Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    sendJsonResponse("enroll", "Communication Error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    sendJsonResponse("enroll", "Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    sendJsonResponse("enroll", "Could not find fingerprint features");
    return p;
  default:
    sendJsonResponse("enroll", "Unknown Error");
    return p;
  }

  sendJsonResponse("enroll", "Remove Finger");
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
      sendJsonResponse("enroll", "Image Taken");
      break;
    case FINGERPRINT_NOFINGER:
      sendJsonResponse("enroll", "Place finger again");
      continue;
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      sendJsonResponse("enroll", "Communication Error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      sendJsonResponse("enroll", "Imaging Error");
      break;
    default:
      sendJsonResponse("enroll", "Unknown Error");
      break;
    }
  }

  p = finger.image2Tz(2);
  switch (p)
  {
  case FINGERPRINT_OK:
    sendJsonResponse("enroll", "Image Converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    sendJsonResponse("enroll", "Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    sendJsonResponse("enroll", "Communication Error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    sendJsonResponse("enroll", "Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    sendJsonResponse("enroll", "Could not find fingerprint features");
    return p;
  default:
    sendJsonResponse("enroll", "Unknown Error");
    return p;
  }

  sendJsonResponse("enroll", ("Creating model for ID #" + String(id)).c_str());
  p = finger.createModel();

  if (p == FINGERPRINT_OK)
  {
    sendJsonResponse("enroll", "Prints Matched!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    sendJsonResponse("enroll", "Communication Error");
    return p;
  }
  else if (p == FINGERPRINT_ENROLLMISMATCH)
  {
    sendJsonResponse("enroll", "Fingerprints did not match");
    return p;
  }
  else
  {
    sendJsonResponse("enroll", "Unknown Error");
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
    sendJsonResponse("enroll", "Communication Error");
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
    sendJsonResponse("enroll", "Unknown Error");
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
      sendJsonResponse("search", "Image Taken");
      break;
    case FINGERPRINT_NOFINGER:
      sendJsonResponse("search", "No finger detected");
      continue;
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      sendJsonResponse("search", "Communication Error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      sendJsonResponse("search", "Imaging Error");
      return p;
    default:
      sendJsonResponse("search", "Unknown Error");
      return p;
    }
  }

  // OK success!

  p = finger.image2Tz();
  switch (p)
  {
  case FINGERPRINT_OK:
    sendJsonResponse("search", "Image Converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    sendJsonResponse("search", "Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    sendJsonResponse("search", "Communication Error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    sendJsonResponse("search", "Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    sendJsonResponse("search", "Could not find fingerprint features");
    return p;
  default:
    sendJsonResponse("search", "Unknown Error");
    return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK)
  {
    if (finger.fingerID > 0)
    {
      sendJsonResponse("search", "Found a print match!");
    }
    else
    {
      sendJsonResponse("search", "Did not find a match");
      return FINGERPRINT_NOTFOUND;
    }
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    sendJsonResponse("search", "Communication Error");
    return p;
  }
  else if (p == FINGERPRINT_NOTFOUND)
  {
    sendJsonResponse("search", "Did not find a match");
    return p;
  }
  else
  {
    sendJsonResponse("search", "Unknown Error");
    return p;
  }

  // found a match!
  sendJsonResponse("search", "Fingerprint Matched", finger.fingerID);

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
      sendJsonResponse("verify", "Image Taken");
      break;
    case FINGERPRINT_NOFINGER:
      sendJsonResponse("verify", "No finger detected");
      continue;
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      sendJsonResponse("verify", "Communication Error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      sendJsonResponse("verify", "Imaging Error");
      return p;
    default:
      sendJsonResponse("verify", "Unknown Error");
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
    sendJsonResponse("verify", "Unknown Error");
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
    sendJsonResponse("verify", "Communication Error");
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
  sendJsonResponse("verify", "Fingerprint Matched", finger.fingerID);
  return 200;
}

String getUID(byte *uid)
{
  String uidStr = "";
  for (byte i = 0; i < rfid.uid.size; i++)
  {
    uidStr += (uid[i] < 0x10 ? "0" : ""); // Add leading zero if needed
    uidStr += String(uid[i], HEX);        // Convert byte to hexadecimal and add to the string
  }
  return uidStr;
}

void readRfidCard()
{
  while (1)
  {
    sendJsonResponseCard("rfid", "Scan Book");
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
    {
      cardUID = getUID(rfid.uid.uidByte); // Store the UID in the cardUID variable
      sendJsonResponseCard("rfid", "Card Read", cardUID);
      break;
    }
  }
}

void switchString(String command)
{
  if (command == "enroll")
  {
    // Handle the "enroll" command
    uint16_t response = enrollFinger();
    if (response == 200)
    {
      sendJsonResponse("enroll", "Enrollment Successful");
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
      sendJsonResponse("verify", "Fingerprint Matched", finger.fingerID);
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
      sendJsonResponse("search", "Fingerprint Matched", finger.fingerID);
    }
    else
    {
      sendJsonResponse("search", "Verification Failed");
    }
  }
  else if (command == "read")
  {
    // Handle the "read" command
    readRfidCard();
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

  SPI.begin();     // Initialize SPI bus
  rfid.PCD_Init(); // Initialize MFRC522

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