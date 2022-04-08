#define fadeAmount 2
#define timeSeconds 10

// Set GPIOs for LED and PIR Motion Sensor
const int led_gpio = 32;
const int motionSensor = 27;

// Timer: Auxiliary variables
unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;

// LED Fading
//int brightness = 0;
//boolean fadingIn = false;
//boolean fadingOut = false;

// Checks if motion was detected, sets LED HIGH and starts a timer
void IRAM_ATTR detectsMovement() {
  Serial.println("MOTION DETECTED!!!");

//  if (!fadingIn) {
//    fadeIn();
//  }
  
  startTimer = true;
  lastTrigger = millis();
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  
  // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(motionSensor, INPUT_PULLUP);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // Set LED to LOW
//  ledcAttachPin(led_gpio, 0); // assign a led pins to a channel
//  ledcSetup(0, 4000, 8); // 12 kHz PWM, 8-bit resolution
}

void loop() {
  // Current time
  now = millis();
  // Turn off the LED after the number of seconds defined in the timeSeconds variable
  if(startTimer && (now - lastTrigger > (timeSeconds*1000))) {
    Serial.println("Motion stopped...");
//    fadeOut();
    startTimer = false;
  }
}

void fadeIn() {
  Serial.println("Fading in...");
//    fadingIn = true;
//    brightness = 0;
//
//    do {
//      ledcWrite(0, brightness);
//
//      brightness += fadeAmount;
//      delay(30);
//    }
//    while (brightness <= 255);
//
//    // while is blocking, so this will trigger after the full fade in
//    fadingIn = false;
}

void fadeOut() {    
    Serial.println("Fading out...");
//    fadingOut = true;
//
//    do {
//      ledcWrite(0, brightness);
//
//      brightness -= fadeAmount;
//      delay(30);
//    }
//    while (brightness >= 0);
//
//    // while is blocking, so this will trigger after the full fade in
//    fadingOut = false;
}
