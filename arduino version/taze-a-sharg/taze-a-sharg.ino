#include <Servo.h>

// ===== Pins =====

// seven segment display for combo and timer
const byte timerDataPin = 31;
const byte timerLatchPin = 32;
const byte timerClockPin = 33;
const byte timerDigitPins[] = {37, 36, 35, 34};

// seven segment display for score
const byte scoreDataPin = 41;
const byte scoreLatchPin = 42;
const byte scoreClockPin = 43;
const byte scoreDigitPins[] = {47, 46, 45, 44};

// targets
const byte photoresistorPins[] = {A1, A2, A3};
const byte rgTargetPins[] = {22, 23, 24, 25, 26, 27};
const byte buzzerPin = 10;

// laser
const byte laserPin = 9;
const byte triggerPin = 3;
const byte servoPin = 5;
Servo servo;

// backlight
const byte rgbBacklightPins[] = {51, 52, 53};

// button
const byte buttonPin = 2;


// ===== Game constants =====

// seven segment displays
const int displayBrightness = 90;       // seven segment display brightness (0-100)
const bool displayLeadingZeros = true;  // show leading zeros in seven segment display
const int maxCombo = 99;                // max combo (2 digits on seven segment display)
const int maxTimer = 99;                // max timer (2 digits on seven segment display)
const int maxScore = 9999;              // max score (4 digits on seven segment display)

// score
const float basePoints = 83;            // base points for hit (5000/60 since half points for base)
const float scoreMultiplier = 1.64;     // score multiplier for combo (3000/1830 since 30% of points for combo)
const int maxSpeedBonus = 33;           // max speed bonus points (2018/60 since 20% of points for speed bonus)
const int maxHitSpeedMs = 1000;         // max hit speed for bonus points

// timer
const int countdownIntervalMs = 500;    // interval for countdown
const int countdownStart = 3;           // countdown timer start (countdown duration)
const int timerIntervalMs = 1000;       // interval for timer
const int timerStart = 60;              // game timer start (game duration)

// targets
const int numTargets = 3;               // number of targets
const int targetThreshold = 800;        // hit threshold for photoresistor (0-1023)
const int minTargetDurationMs = 2100;   // min target lifetime duration before switching to new target
const int maxTargetDurationMs = 4200;   // max target lifetime duration before switching to new target
const int buzzerDurationMs = 200;       // duration of buzzer for target hit/miss
const int newTargetDurationMs = 50;     // duration between old target off and new target on
const int hitToneHz = 700;              // tone frequency for target hit
const int missToneHz = 350;             // tone frequency for target miss

// laser
const int maxEnergy = 100;              // maximum energy level
const int dischargeRate = 1;            // amount of energy decreased per cycle when triggered
const int rechargeRateNormal = 1;       // amount of energy increased per cycle on normal
const int rechargeRateOverheat = 3;     // amount of energy increased per cycle on overheat
const int laserBrightness = 50;        // laser brightness for PWM (0-255)

// button
const int buttonDebounceMs = 200;       // threshold for button debounce


// ===== Color constants =====

// RGB color constants for backlight
const byte backlightOff[3] = {0, 0, 0}; 
const byte backlightCountdown[3] = {255, 255, 255};
const byte backlightGame[3] = {0, 0, 255};
const byte backlightOverheat[3] = {255, 0, 0};

// RG color constants for targets
const byte targetOff[2] = {0, 0};
const byte targetRed[2] = {255, 0};
const byte targetGreen[2] = {0, 255};


// ===== Seven segment display mapping =====
const byte segmentBits[] = {
  0x5F,  // = 0
  0x44,  // = 1
  0x9D,  // = 2
  0xD5,  // = 3
  0xC6,  // = 4
  0xD3,  // = 5
  0xDB,  // = 6
  0x45,  // = 7
  0xDF,  // = 8
  0xC7,  // = 9
  0x00,  // blank
};


// ===== Seven segment display digits =====
byte timerDigits[4];
byte scoreDigits[4];


// ===== Millis variables =====
unsigned long timerMillis;
unsigned long targetMillis;
unsigned long buttonMillis;


// ===== Game variables =====

// enums
enum GameState {
  PRE_GAME,
  COUNTDOWN,
  IN_GAME,
  GAME_OVER,
};

enum TargetState {
  SELECT_TARGET,
  UPDATE_TARGET,
  READ_TARGET,
};

// states
byte gameState;
byte targetState;

// seven segment displays
int timer;
int combo;
int score;
float speedBonus;

// targets
int currentTarget;
int targetDurationMs;

// laser
int energy;
int rechargeRate;
bool overheat;


void setup() {
  Serial.begin(9600);

  // seven segment displays
  pinMode(timerDataPin, OUTPUT);
  pinMode(timerLatchPin, OUTPUT);
  pinMode(timerClockPin, OUTPUT);
  pinMode(scoreDataPin, OUTPUT);
  pinMode(scoreLatchPin, OUTPUT);
  pinMode(scoreClockPin, OUTPUT);
  for (int i = 0; i < 4; i++) {
    pinMode(timerDigitPins[i], OUTPUT);
    pinMode(scoreDigitPins[i], OUTPUT);
  }

  // setting all the seven segement displays off
  digitalWrite(timerLatchPin, HIGH);  
  digitalWrite(scoreLatchPin, HIGH);

  // targets
  for (int i = 0; i < numTargets; i++) {
    pinMode(photoresistorPins[i], INPUT);
  }
  for (int i = 0; i < numTargets * 2; i++) {
    pinMode(rgTargetPins[i], OUTPUT);
  }
  pinMode(buzzerPin, OUTPUT);

  // laser
  pinMode(laserPin, OUTPUT);
  pinMode(triggerPin, INPUT);
  servo.attach(servoPin);

  // backlight
  for (int i = 0; i < 3; i++) {
    pinMode(rgbBacklightPins[i], OUTPUT);
  }

  // button
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), changeGameState, FALLING);
  buttonMillis = millis();

  // initialize random seed
  randomSeed(analogRead(0));

  // initialize game state
  gameState = GameState::PRE_GAME;
}

void loop() {
  switch (gameState) {
    case GameState::PRE_GAME:
      gameReady();
      gameState = GameState::COUNTDOWN;
      break;

    case GameState::COUNTDOWN:
      updateTimer(countdownIntervalMs);
      if (timer == 0) {
        gameStart();
        gameState = GameState::IN_GAME;
      }
      break;

    case GameState::IN_GAME:
      updateTimer(timerIntervalMs);
      if (timer == 0) {
        gameOver();
        gameState = GameState::GAME_OVER;
      }
      else {
        gameLoop();
      }
      break;

    case GameState::GAME_OVER:
      break;
  }

  updateSegmentDigits();
  displayTimerSegments();
  displayScoreSegments();
  delayMicroseconds(1638*((100-displayBrightness)/10)); // largest value 16383
}

void changeGameState() {
  if (millis() - buttonMillis > buttonDebounceMs) {
    if (gameState == GameState::GAME_OVER) {
      gameState = GameState::PRE_GAME;
    }
    else {
      gameState = GameState::GAME_OVER;
    }
    buttonMillis = millis();
  }
}

void gameReady() {
  timer = countdownStart;
  combo = 0;
  score = 0;
  energy = 100;
  currentTarget = 0;
  targetState = TargetState::SELECT_TARGET;
  timerMillis = millis();
  targetMillis = millis();
  setBacklightColor(backlightCountdown);
}

void gameStart() {
  timer = timerStart;
}

void gameLoop() {
  switch (targetState) {
    case TargetState::SELECT_TARGET:
      if (millis() - targetMillis > buzzerDurationMs) {
        setTargetColor(currentTarget, targetOff);
        noTone(buzzerPin);
        currentTarget = random(numTargets);
        targetDurationMs = random(minTargetDurationMs, maxTargetDurationMs);
        targetMillis = millis();
        targetState = TargetState::UPDATE_TARGET;
      }
      break;
    
    case TargetState::UPDATE_TARGET:
      if (millis() - targetMillis > newTargetDurationMs) {
        setTargetColor(currentTarget, targetRed);
        targetMillis = millis();
        targetState = TargetState::READ_TARGET;
      }
      break;
    
    case TargetState::READ_TARGET:
      if (readTarget()) {
        targetMillis = millis();
        targetState = TargetState::SELECT_TARGET;
      }
      break;
  }
  
  checkEnergy();
  readTrigger();
  updateEnergyServo();
}

void gameOver() {
  analogWrite(laserPin, 0);
  noTone(buzzerPin);
  setBacklightColor(backlightOff);
  for (int i = 0; i < numTargets; i++) {
    setTargetColor(i, targetOff);
  }
}

void updateTimer(int intervalMs) {
  if (millis() - timerMillis > intervalMs) {
    timer = timer - 1;
    timerMillis = millis();
  }
}

bool readTarget() {
  int value = analogRead(photoresistorPins[currentTarget]);
  if (value > targetThreshold) {
    setTargetColor(currentTarget, targetGreen);
    tone(buzzerPin, hitToneHz);
    combo = combo + 1;
    score = score + calculatePoints();
    return true;
  }
  else if (millis() - targetMillis > targetDurationMs) {
    setTargetColor(currentTarget, targetOff);
    tone(buzzerPin, missToneHz);
    combo = 0;
    return true;
  }
  return false;
}

void checkEnergy() {
  if (energy == 0) {
    overheat = true;
    rechargeRate = rechargeRateOverheat;
    setBacklightColor(backlightOverheat);
  }
  else if (energy == maxEnergy) {
    overheat = false;
    rechargeRate = rechargeRateNormal;
    setBacklightColor(backlightGame);
  }
}

void readTrigger() {
  int value = digitalRead(triggerPin); // on is 0, off is 1
  if (!value) {
    if (!overheat) {
      analogWrite(laserPin, laserBrightness);
      energy = energy - dischargeRate;
      if (energy < 0) {
        energy = 0;
      }
    }
    else {
      analogWrite(laserPin, 0);
    }
  }
  else {
    analogWrite(laserPin, 0);
    energy = energy + rechargeRate;
    if (energy > maxEnergy) {
      energy = maxEnergy;
    }
  }
}

void updateEnergyServo() {
  int position = map(energy, 0, maxEnergy, 180, 0);
  servo.write(position);
}

void setBacklightColor(byte* rgb) {
  analogWrite(rgbBacklightPins[0], rgb[0]);
  analogWrite(rgbBacklightPins[1], rgb[1]);
  analogWrite(rgbBacklightPins[2], rgb[2]);
}

void setTargetColor(int target, byte* rg) {
  if (target < numTargets) {
    analogWrite(rgTargetPins[target*2], rg[0]);
    analogWrite(rgTargetPins[target*2+1], rg[1]);
  }
}

// Seven segment digits index: 0 1 2 3
void updateSegmentDigits() {
  if (combo > maxCombo) {
    combo = maxCombo;
  }
  if (timer > maxTimer) {
    timer = maxTimer;
  }
  if (score > maxScore) {
    score = maxScore;
  }

  int comboTemp = combo;
  for (int i = 1; i >= 0; i--) {
    byte digit = comboTemp % 10;
    comboTemp = comboTemp / 10;
    timerDigits[i] = digit;
  }

  int timerTemp = timer;
  for (int i = 3; i >= 2; i--) {
    byte digit = timerTemp % 10;
    timerTemp = timerTemp / 10;
    timerDigits[i] = digit;
  }

  int scoreTemp = score;
  for (int i = 3; i >= 0; i--) {
    byte digit = scoreTemp % 10;
    scoreTemp = scoreTemp / 10;
    scoreDigits[i] = digit;
  }

  if (!displayLeadingZeros) {
    if (combo < 10) {
      timerDigits[0] = 10;  // blank
    }
    if (timer < 10) {
      timerDigits[2] = 10;  // blank
    }
    if (score < 1000) {
      scoreDigits[0] = 10;  // blank
      if (score < 100) {
        scoreDigits[1] = 10;  // blank
        if (score < 10) {
          scoreDigits[2] = 10;  // blank
        }
      }
    }
  }
}

void displayTimerSegments() {
  for (int i = 0; i < 4; i++){
    for (int j = 0; j < 4; j++){
        digitalWrite(timerDigitPins[j], LOW);   // turn off digits
    }
    digitalWrite(timerLatchPin, LOW);
    shiftOut(timerDataPin, timerClockPin, MSBFIRST, segmentBits[timerDigits[i]]);
    digitalWrite(timerLatchPin, HIGH);
    digitalWrite(timerDigitPins[i], HIGH);      // turn on one digit
    delay(1);
  }
  for (int j = 0; j < 4; j++){
      digitalWrite(timerDigitPins[j], LOW);     // turn off digits
  }
}

void displayScoreSegments() {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
        digitalWrite(scoreDigitPins[j], LOW);   // turn off digits
    }
    digitalWrite(scoreLatchPin, LOW);
    shiftOut(scoreDataPin, scoreClockPin, MSBFIRST, segmentBits[scoreDigits[i]]);
    digitalWrite(scoreLatchPin, HIGH);
    digitalWrite(scoreDigitPins[i], HIGH);      // turn on one digit
    delay(1);
  }
  for (int j = 0; j < 4; j++) {
      digitalWrite(scoreDigitPins[j], LOW);     // turn off digits
  }
}

float calculatePoints() {
  int hitSpeedMs = millis() - targetMillis;
  if (hitSpeedMs < maxHitSpeedMs) {
    speedBonus = map(hitSpeedMs, maxHitSpeedMs, 0, 0, maxSpeedBonus);
  }
  return (basePoints + (combo * scoreMultiplier) + speedBonus) * (timerStart / 60);
}
