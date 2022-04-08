class Led
{
  private:
  int ledBrightness;

  public:
  int getBrightness() {
    return ledBrightness;
  }

  int ledPin;
  int ledChannel;

  int ledFadeIncrement;
  int ledMaxBrightness;
  int ledDelayIncrement;

  int ledDelaySpeed = 15; // Milliseconds to wait between each fade tick

  bool ledTriggered = false;
  unsigned long ledTriggerTime;
  unsigned int ledFadeTick = 0;

  public:
  Led(int pin, int channel, int brightness, int fadeIncrement, int maxBrightness, int delayIncrement)
  {
    ledPin = pin;
    ledChannel = channel;
    ledBrightness = brightness;
    ledFadeIncrement = fadeIncrement;
    ledMaxBrightness = maxBrightness;
    ledDelayIncrement = delayIncrement;

    // Setup
    ledcAttachPin(ledPin, ledChannel);
    ledcSetup(ledChannel, 4000, 8); // 12 kHz PWM, 8-bit resolution
  }


  void fadeInEffect() {
    // When millis >= triggerTime (millis of first start) + (delaySpeed * fadeTick), then it is ready for next fade tick
    if (millis() >= ledTriggerTime + (ledDelaySpeed * ledFadeTick)) {
      ledBrightness += ledFadeIncrement;
      ledcWrite(ledChannel, ledBrightness);
      ledFadeTick += 1;
    }
    
    // When brightness + fadeIncrement >= maxBrightness, stop, otherwise re-run.
    if (ledBrightness + ledFadeIncrement < ledMaxBrightness) {
      fadeInEffect();
    } else {
      // TODO: Last tick may not be exactly max brightness defined,
      //  as the last increment may be larger than the brightness desired and met the condition to not increment
      Serial.println("Done fading up.");
      Serial.println("");
      Serial.print("Final brightness = ");
      Serial.print(ledBrightness);
      Serial.println("");
      
      // Recycle and end
      ledFadeTick = 0;
      ledTriggered = false;
      return;
    }
  }


  // Wait for set delay between strips to turn fade in.
  void delayFader() {
    if (millis() < (ledTriggerTime + ledDelayIncrement * (ledChannel + 1))) {
      delayFader();
    } else {
      fadeInEffect();
    }
  }


  void fadeIn(long triggerTime) {
    if (ledTriggered) {
      return;
    }

    ledTriggered = true;
    ledTriggerTime = triggerTime;

    Serial.println("");
    Serial.print("Fading in channel: ");
    Serial.print(ledChannel);
    Serial.println("");
    delayFader();
  }
  
  
  void fadeOut() {  
    Serial.println("");
    Serial.print("Fading out channel: ");
    Serial.print(ledChannel);
    Serial.println("");
      
    do {
      ledBrightness -= ledFadeIncrement;
      ledcWrite(ledChannel, ledBrightness);
      delay(15);
    }
    while (ledBrightness > 0);
  }
};
