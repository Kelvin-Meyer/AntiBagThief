/*Report is a state because reporting may fail (wifi)*/
enum AlarmState {DISABLED, ACTIVE, REPORT, COOLDOWN};
AlarmState state;

// DISABLED
volatile bool enabled = true;
int enabledLed = 13; // led pin
int buttonPin = 2; // The on/off button pin. Must be on a pin that supports interrupts, see https://www.arduino.cc/en/Reference/attachInterrupt
unsigned long lastPressedTime;


// ACTIVE / DETECTION STATE
int tiltSensorPin = 8;
int movementSensitivity = 3; // periods before we call it movement
int periodLength = 1000; // Milliseconds per period
int triggerSensitivity = 50; // defines how many triggers need to be in a period for it to count as a movement

// REPORT STATE
int outputSpeaker = 6; // speaker pin

//COOLDOWN STATE
int outputCooldownLed = 12; // led pin

void setup() {
  // put your setup code here, to run once:
  state = DISABLED;

  Serial.begin(9600);
  pinMode(tiltSensorPin, INPUT);
  pinMode(outputSpeaker, OUTPUT);
  pinMode(enabledLed, OUTPUT);
  pinMode(buttonPin, INPUT);

  attachInterrupt(digitalPinToInterrupt(buttonPin), pushActivationButton, FALLING);
  lastPressedTime = millis();
  digitalWrite(enabledLed,HIGH);

}

void loop() {
  // put your main code here, to run repeatedly:
  switch (state) {
    case DISABLED : state = disabled(); break;
    case ACTIVE : state = active(); break;
    case REPORT : state = report(); break;
    case COOLDOWN : state = cooldown(); break;
  }
  
  if (!enabled) {
    state = DISABLED;
  }
}

/**
 * Disables the alarm
 * @return ACTIVE when activated again
 */
AlarmState disabled() {
  Serial.println("DISABLED state");
  
  while (!enabled) {
    delay(1000);
  }
  return ACTIVE;
}

/**
 * Detects movement
 * @return ACTIVE when not alarmed or REPORT when alarmed
 */
AlarmState active() {
    Serial.println("ACTIVE state");
    int successiveTriggers = 0; // defines movement seconds before alarming
    boolean alarmed = false;
    do {
        int triggersInThePeriod = measureMovement();
        
        if (triggersInThePeriod > triggerSensitivity) {
          successiveTriggers++;
          
          Serial.print("Movement detected count - ");
          Serial.print(successiveTriggers);
          Serial.print("/");
          Serial.println(movementSensitivity);
          
          if(successiveTriggers == movementSensitivity) {
            alarmed = true;
          }
        } else {
          successiveTriggers = 0; // start again with counting
        }
    } while (!alarmed && enabled);
  return REPORT;
}

/**
 * Reports the theft to the user
 * @return COOLDOWN when succesful, REPORT otherwise
 */
AlarmState report() {
  Serial.println("REPORT state");
  if (enabled) {
    digitalWrite(outputSpeaker, HIGH);
    delay(2500);
    digitalWrite(outputSpeaker, LOW);
  }
  return COOLDOWN;
}

/**
 * Waits 60 seconds
 * Currently unresponsive during waiting.
 * @return ACTIVE when done with waiting
 */
AlarmState cooldown() {
  Serial.println("COOLDOWN state");
  digitalWrite(outputCooldownLed, HIGH);

  for (int i = 6; i != 0; i--) {
    Serial.print(i);
    Serial.println(" seconds left until the cooldown ends");
    delay(1000);
  }
   digitalWrite(outputCooldownLed, LOW);

  return ACTIVE;  
}

/**
 * Measures the amount of movement within one period
 * @return the amount of times the tilt sensor switched state within one period
 */
int measureMovement() {
  int oldTiltState = digitalRead(tiltSensorPin);
  int newTiltState = digitalRead(tiltSensorPin);
  int timesTriggered = 0;
  unsigned long startTime = millis();
  unsigned long currentTime;

  // for periodLength milliseconds:
  do {
    oldTiltState = digitalRead(tiltSensorPin);
    if (newTiltState != oldTiltState) {
      timesTriggered++;
      newTiltState = oldTiltState;
    }
    currentTime = millis();
  } while ((currentTime - startTime) < periodLength);

  return timesTriggered; // return the results
  
}

/**
 * Switches the enabled boolean, is triggered by an interrupt when the pushActivationButton is pushed
 */
void pushActivationButton() {
  Serial.println("Detected buttonpress.");
  if (enabled) {
    digitalWrite(enabledLed,LOW);
    enabled = false;
  } else {
    digitalWrite(enabledLed,HIGH);
    enabled = true;
  }
}

