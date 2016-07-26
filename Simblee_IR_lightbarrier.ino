#include <Arduino.h>

#include <SimbleeForMobile.h>
#include "Timer.h"

const bool DEBUG = true;

const int DURATION_PAIRING_MODE_MS = SECONDS(10);
const int DURATION_CONNECTED_MODE_MS = SECONDS(30);
const int INTERVAL_CHECK = SECONDS(12);


const int button = 15; // The Simblee BOB (WRL-13632) has a button on pin 3.
const int sensor = 3;
const int led = 12;

const int THRESHOLD = 200;


uint8_t boxID;
uint8_t textID;

boolean isBirdInside;
boolean wasBirdInside;



Timer pairingModeTimer;
boolean pairingMode = false;
Timer connectedModeTimer;
boolean connectedMode = false;

int count = 0;

void setup()
{
  pinMode(sensor, INPUT);
  pinMode(button, INPUT);
  pinMode(led, OUTPUT);

  SimbleeForMobile.advertisementData = "Button";
  SimbleeForMobile.deviceName = "WRL-13632";
  SimbleeForMobile.advertisementInterval = 100;

  // txPowerLevel can be any multiple of 4 between -20 and +4, inclusive. The
  // default value is +4; at -20 range is only a few feet.
  SimbleeForMobile.txPowerLevel = -20;

  // This must be called *after* you've set up the variables above, as those
  // variables are only written during this function and changing them later
  // won't actually propagate the settings to the device.
  SimbleeForMobile.begin();
  Serial.begin(9600);

  wasBirdInside = checkBirdInside();
}

void loop()
{
  //CHECK BIRD
  isBirdInside = checkBirdInside();
  if ( wasBirdInside && !isBirdInside) {
    wasBirdInside = false;
    count++;
    Serial.print("bird is out! ");
    Serial.println(count);
    //digitalWrite(led, HIGH);
  } else if ( !wasBirdInside && isBirdInside) {
    //digitalWrite(led, LOW);
    wasBirdInside = true;
  }

  //CHECK PAIRINGMODE
  if ( digitalRead(button) == HIGH && !pairingMode) {
    enablePairingMode();
  } else if ( pairingMode ) {
    pairingModeTimer.update();
  }

  if ( connectedMode) {
    updateUI();
    connectedModeTimer.update();
  }

  if (!pairingMode && !connectedMode && digitalRead(button) != HIGH) {
    Serial.println("GOING TO SLEEP");
    delay(200);
    Simblee_ULPDelay(INTERVAL_CHECK);
  }

  SimbleeForMobile.process();
}

void SimbleeForMobile_onConnect() {
  Serial.println("CONNECTED");
  enableConnectedMode();
}

void SimbleeForMobile_onDisconnect() {
  Serial.println("DISCONNECTED");
}

void enablePairingMode() {
  Serial.println("pairing mode ON");
  digitalWrite(led, HIGH);
  pairingMode = true;
  pairingModeTimer.after(DURATION_PAIRING_MODE_MS, disablePairingMode);
}

//disables pairingmode
void disablePairingMode() {
  Serial.println("pairing mode OFF");
  pairingMode = false;
  digitalWrite(led, LOW);
}

void enableConnectedMode() {
  Serial.println("connected mode ON");
  disablePairingMode();
  connectedModeTimer.after(DURATION_CONNECTED_MODE_MS, disableConnectedMode);
  connectedMode = true;
}

//disables pairingmode
void disableConnectedMode() {
  Serial.println("connected mode OFF");
  connectedMode = false;
  digitalWrite(led, LOW);
}

void updateUI() {
  if (SimbleeForMobile.updatable)
  {
    Serial.println("updating ui value to ");
    Serial.print(count);
    SimbleeForMobile.updateValue(textID, count);
    disableConnectedMode();
  }
}

boolean checkBirdInside() {
  int sensorValue = analogRead(sensor);
  if ( sensorValue <= THRESHOLD) {
    return false;
  }
  return true;
}


// ui() is a SimbleeForMobile specific function which handles the specification
// of the GUI on the mobile device the Simblee connects to.
void ui()
{
  // color_t is a special type which contains red, green, blue, and alpha
  // (transparency) information packed into a 32-bit value. The functions rgb()
  // and rgba() can be used to create a packed value.
  color_t darkgray = rgb(85, 85, 85);

  // These variable names are long...let's shorten them. They allow us to make
  // an interface that scales and scoots appropriately regardless of the screen
  // orientation or resolution.
  uint16_t wid = SimbleeForMobile.screenWidth;
  uint16_t hgt = SimbleeForMobile.screenHeight;

  // The beginScreen() function both sets the background color and serves as a
  // notification that the host should try to cache the UI functions which come
  // between this call and the subsequent endScreen() call.
  //SimbleeForMobile.beginScreen(darkgray);
  SimbleeForMobile.beginScreen();

  // SimbleeForMobile doesn't really have an kind of indicator- but there IS a
  // drawRect() function, and we can freely change the color of the rectangle
  // after drawing it! The x,y coordinates are of the upper left hand corner.
  // If you pass a second color parameter, you'll get a fade from top to bottom
  // and you'll need to update *both* colors to get the whole box to change.
  /*  boxID = SimbleeForMobile.drawRect(
              (wid / 2) - 50,      // x position
              (hgt / 2) - 50,      // y positon
              100,                 // x dimension
              100,                 // y dimensionrectangle
              BLACK);              // color of rectangle.
  */
  textID = SimbleeForMobile.drawText(
             (wid / 2) - 50,      // x position
             (hgt / 2) - 50,      // y positon
             count,                 // value
             BLACK,            //color
             36);             //size

  SimbleeForMobile.endScreen();
}

// This function is called whenever a UI event occurs. Events are fairly easy
// to predict; for instance, touching a button produces a "PRESS_EVENT" event.
// UI elements have default event generation settings that match their expected
// behavior, so you'll only rarely have to change them.
void ui_event(event_t &event)
{
  // In this case, we're sending data back from the Simblee to the app, so we
  // don't really care about any events that might occur.
}

//prints message to serialport
void debug(String msg, boolean endl) {
  if ( DEBUG) {
    Serial.println(msg);
    if ( endl) {
      Serial.println();
    }
  }
}

void debug(int num, boolean endl) {
  debug("" + num, true);
}

void debug(int num) {
  debug(num, true);
}

void debug(String msg) {
  debug(msg, true);
}

