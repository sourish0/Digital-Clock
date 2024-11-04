#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int buttonSetPin = 2;          
const int buttonIncrementPin = 3;     
const int buttonChangeUnitPin = 4;    
const int buttonAlarmPin = 5;         
const int buttonStopwatchPin = 6;     
const int buttonResetStopwatchPin = 7; 
const int buzzerPin = 8; // Buzzer pin

int hours = 0, mins = 0, secs = 0;
int alarmHours = 0, alarmMinutes = 0; 
bool settingMode = false;             
bool alarmMode = false;               
bool alarmTriggered = false;          
bool stopwatchMode = false;           
bool stopwatchRunning = false;        
bool stopwatchPaused = false;         // New state for paused stopwatch
unsigned long stopwatchStartTime = 0; 
unsigned long elapsedMillis = 0;      
unsigned long elapsedMicros = 0;      
int settingUnit = 0;                  

// Debounce timers
unsigned long lastSetDebounceTime = 0;
unsigned long lastIncrementDebounceTime = 0;
unsigned long lastChangeUnitDebounceTime = 0;
unsigned long lastAlarmDebounceTime = 0;
unsigned long lastStopwatchDebounceTime = 0;
unsigned long lastResetStopwatchDebounceTime = 0; 
const unsigned long debounceDelay = 200;

void setup() {
  lcd.init();
  lcd.backlight();

  pinMode(buttonSetPin, INPUT_PULLUP);
  pinMode(buttonIncrementPin, INPUT_PULLUP);
  pinMode(buttonChangeUnitPin, INPUT_PULLUP);
  pinMode(buttonAlarmPin, INPUT_PULLUP);
  pinMode(buttonStopwatchPin, INPUT_PULLUP);
  pinMode(buttonResetStopwatchPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT); // Set buzzer pin as output

  Serial.begin(9600);
}

void loop() {
  static unsigned long lastClockUpdate = 0;
  if (millis() - lastClockUpdate >= 1000) {
    lastClockUpdate = millis();
    updateTime();
  }

  checkAlarm();
  handleButtons();

  if (stopwatchRunning && !stopwatchPaused) {
    elapsedMillis = millis() - stopwatchStartTime;
    elapsedMicros = micros() - stopwatchStartTime * 1000; 
  }

  if (settingMode) {
    displaySettingMode();
  } else if (stopwatchMode) {
    displayStopwatchMode();
  } else {
    displayNormalTime();
  }

  delay(100);  
}

void updateTime() {
  secs++;
  if (secs >= 60) {
    secs = 0;
    mins++;
    if (mins >= 60) {
      mins = 0;
      hours++;
      if (hours >= 24) {
        hours = 0;
      }
    }
  }
}

void checkAlarm() {
  if (alarmMode && hours == alarmHours && mins == alarmMinutes && !alarmTriggered) {
    alarmTriggered = true;
    Serial.println("Alarm! Time reached!");
    tone(buzzerPin, 1000); // Start buzzer sound at 1000 Hz
  } else if (hours != alarmHours || mins != alarmMinutes) {
    alarmTriggered = false; 
    noTone(buzzerPin); // Stop the buzzer
  }
}

void handleButtons() {
  if (digitalRead(buttonSetPin) == LOW && millis() - lastSetDebounceTime > debounceDelay) {
    lastSetDebounceTime = millis();
    settingMode = !settingMode;
    settingUnit = 0;  
  }

  if (digitalRead(buttonAlarmPin) == LOW && millis() - lastAlarmDebounceTime > debounceDelay) {
    lastAlarmDebounceTime = millis();
    alarmMode = !alarmMode;
    alarmTriggered = false; 
    noTone(buzzerPin); // Stop the buzzer when alarm mode is toggled
  }

  if (digitalRead(buttonIncrementPin) == LOW && millis() - lastIncrementDebounceTime > debounceDelay) {
    lastIncrementDebounceTime = millis();
    incrementTimeSetting();
  }

  if (digitalRead(buttonChangeUnitPin) == LOW && millis() - lastChangeUnitDebounceTime > debounceDelay) {
    lastChangeUnitDebounceTime = millis();
    settingUnit = (settingUnit + 1) % 3; 
  }

  if (digitalRead(buttonStopwatchPin) == LOW && millis() - lastStopwatchDebounceTime > debounceDelay) {
    lastStopwatchDebounceTime = millis();
    if (!stopwatchRunning) {
      stopwatchRunning = true;
      stopwatchStartTime = millis() - elapsedMillis; 
      stopwatchMode = true; 
    } else {
      stopwatchRunning = false; 
    }
  }

  if (digitalRead(buttonResetStopwatchPin) == LOW && millis() - lastResetStopwatchDebounceTime > debounceDelay) {
    lastResetStopwatchDebounceTime = millis();
    resetStopwatch();
  }
}

void incrementTimeSetting() {
  if (settingMode) {
    if (settingUnit == 0) { 
      hours = (hours < 23) ? hours + 1 : 0;
      alarmHours = (alarmMode) ? (alarmHours < 23) ? alarmHours + 1 : 0 : alarmHours; 
    } else if (settingUnit == 1) { 
      mins = (mins < 59) ? mins + 1 : 0;
      alarmMinutes = (alarmMode) ? (alarmMinutes < 59) ? alarmMinutes + 1 : 0 : alarmMinutes; 
    } else { 
      secs = (secs < 59) ? secs + 1 : 0;
    }
  } else if (alarmMode) {
    if (settingUnit == 0) { 
      alarmHours = (alarmHours < 23) ? alarmHours + 1 : 0;
    } else if (settingUnit == 1) { 
      alarmMinutes = (alarmMinutes < 59) ? alarmMinutes + 1 : 0;
    }
  }
}

void resetStopwatch() {
  stopwatchRunning = false;
  stopwatchPaused = false; // Reset the paused state
  stopwatchMode = false; 
  elapsedMillis = 0;     
  elapsedMicros = 0;     
}

void displayNormalTime() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  lcd.print(hours < 10 ? "0" : ""); lcd.print(hours);
  lcd.print(":");
  lcd.print(mins < 10 ? "0" : ""); lcd.print(mins);
  lcd.print(":");
  lcd.print(secs < 10 ? "0" : ""); lcd.print(secs);

  lcd.setCursor(0, 1);
  if (alarmMode) {
    lcd.print("Alarm: ");
    lcd.print(alarmHours < 10 ? "0" : ""); lcd.print(alarmHours);
    lcd.print(":");
    lcd.print(alarmMinutes < 10 ? "0" : ""); lcd.print(alarmMinutes);
    if (alarmTriggered) {
      lcd.print(" ALARM!");
    }
  } else {
    lcd.print("                "); 
  }
}

void displaySettingMode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set ");
  if (settingUnit == 0) {
    lcd.print("Hour: ");
    lcd.print(hours);
  } else if (settingUnit == 1) {
    lcd.print("Minute: ");
    lcd.print(mins);
  } else {
    lcd.print("Second: ");
    lcd.print(secs);
  }
}

void displayStopwatchMode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Stopwatch");
  
  lcd.setCursor(0, 1);
  unsigned long totalElapsed = elapsedMillis / 1000; 
  int stopwatchHours = totalElapsed / 3600;
  int stopwatchMinutes = (totalElapsed % 3600) / 60;
  int stopwatchSeconds = totalElapsed % 60;
  int stopwatchMicros = elapsedMicros % 1000; 

  lcd.print(stopwatchHours < 10 ? "0" : ""); lcd.print(stopwatchHours);
  lcd.print(":");
  lcd.print(stopwatchMinutes < 10 ? "0" : ""); lcd.print(stopwatchMinutes);
  lcd.print(":");
  lcd.print(stopwatchSeconds < 10 ? "0" : ""); lcd.print(stopwatchSeconds);
}
