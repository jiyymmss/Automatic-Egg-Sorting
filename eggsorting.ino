#include <HX711.h>
#include <Servo.h>
#include <RTClib.h>
#include <SoftwareSerial.h>

SoftwareSerial espSerial(0, 1); // RX, TX

// ==================== PINS ====================
#define LOADCELL_DOUT_PIN 4
#define LOADCELL_SCK_PIN  A2

#define STOPPER_SERVO_PIN 8
#define SMALL_PUSHER_PIN  12
#define MEDIUM_PUSHER_PIN A1
#define LARGE_PUSHER_PIN  A0
#define FEEDER_SERVO_PIN  A3

#define IR_SENSOR_PIN     2
#define IR_LED_PIN        13

#define IR_COUNT1_PIN 9
#define IR_COUNT2_PIN 10
#define IR_COUNT3_PIN 11

#define RPWM 5
#define LPWM 6

// ==================== SETTINGS ====================
#define CALIBRATION_FACTOR 118.918914
#define SERVO_OPEN_ANGLE 8
#define SERVO_CLOSE_ANGLE 30
#define PUSH_ANGLE 40
#define HOME_ANGLE 90
#define PUSH_DURATION 500
#define STOPPER_DURATION 150
#define MAX_READINGS 2

// ============Delay of Servo Motor Push in Sorting=============
float IR_DELAY_SMALL  = 1.0;
float IR_DELAY_MEDIUM = 2.0;
float IR_DELAY_LARGE  = 3.0;

#define MAX_EGGS 10
#define MAX_FEED_SCHEDULES 10

// ==================== OBJECTS ====================
HX711 scale;
RTC_DS3231 rtc;

Servo stopperServo, pusherSmall, pusherMedium, pusherLarge;
Servo feederServo;

// ==================== FEED SCHEDULE ARRAYS ====================
int feedHour[MAX_FEED_SCHEDULES];
int feedMinute[MAX_FEED_SCHEDULES];
int feedPortion[MAX_FEED_SCHEDULES];
bool feedDone[MAX_FEED_SCHEDULES];
int feedCount = 0;

// ==================== SORTING VARIABLES ====================
float weightSum = 0;
int weightReadings = 0;

int eggQueue[MAX_EGGS];
int queueStart = 0;
int queueEnd = 0;

bool stopperClosing = false;
unsigned long stopperOpenTime = 0;
bool pusherActive = false;
unsigned long pusherStartTime = 0;
bool eggOnScale = false;
unsigned long eggDetectedTime = 0;

// ==================== IR COUNTERS ====================
bool lastIR1 = HIGH, lastIR2 = HIGH, lastIR3 = HIGH;

// ==================== BULK SORTING QUEUE FOR PLATFORM ====================
struct EggOnPlatform {
  int category;          // 1=small,2=medium,3=large
  unsigned long detectTime;
  bool pushed;
};

EggOnPlatform platformQueue[MAX_EGGS];
int platformQueueStart = 0;
int platformQueueEnd = 0;

// ==================== MOTOR ====================
void motorReverseSlow(int pwm) {
  analogWrite(RPWM, 0);
  analogWrite(LPWM, pwm);
}

void addEggToQueue(int cat) {
  eggQueue[queueEnd] = cat;
  queueEnd = (queueEnd + 1) % MAX_EGGS;
}

void activatePusher(int cat) {
  if (cat == 1) pusherSmall.write(PUSH_ANGLE);
  if (cat == 2) pusherMedium.write(PUSH_ANGLE);
  if (cat == 3) pusherLarge.write(PUSH_ANGLE);
  pusherActive = true;
  pusherStartTime = millis();
}

// ==================== PET FEED ====================
void dispenseFood(int portion) {
  for (int i = 0; i < portion; i++) {
    feederServo.write(120);
    delay(1000);
    feederServo.write(0);
    delay(1000);
  }
}

// ==================== PLATFORM QUEUE FUNCTIONS ====================
void addEggToPlatformQueue(int category) {
  int nextEnd = (platformQueueEnd + 1) % MAX_EGGS;
  if (nextEnd != platformQueueStart) { // queue not full
    platformQueue[platformQueueEnd].category = category;
    platformQueue[platformQueueEnd].detectTime = millis();
    platformQueue[platformQueueEnd].pushed = false;
    platformQueueEnd = nextEnd;
  }
}

void handlePlatformQueue() {
  if (platformQueueStart == platformQueueEnd) return; // empty

  EggOnPlatform &egg = platformQueue[platformQueueStart];

  if (!egg.pushed) {
    float delayTime = (egg.category == 1 ? IR_DELAY_SMALL :
                       egg.category == 2 ? IR_DELAY_MEDIUM :
                                            IR_DELAY_LARGE) * 1000;

    if (millis() - egg.detectTime >= delayTime) {
      // Activate corresponding pusher
      if (egg.category == 1) pusherSmall.write(PUSH_ANGLE);
      else if (egg.category == 2) pusherMedium.write(PUSH_ANGLE);
      else if (egg.category == 3) pusherLarge.write(PUSH_ANGLE);

      pusherActive = true;
      pusherStartTime = millis();
      egg.pushed = true;
    }
  }

  // Finish push
  if (pusherActive && millis() - pusherStartTime >= PUSH_DURATION) {
    pusherSmall.write(HOME_ANGLE);
    pusherMedium.write(HOME_ANGLE);
    pusherLarge.write(HOME_ANGLE);

    pusherActive = false;

    // Remove egg from platform queue after push
    platformQueueStart = (platformQueueStart + 1) % MAX_EGGS;
  }
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(9600);
   espSerial.begin(9600);
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC. Check connections!");
    while (1); // Stop here
  }
 //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(CALIBRATION_FACTOR);
  scale.tare();

  stopperServo.attach(STOPPER_SERVO_PIN);
  pusherSmall.attach(SMALL_PUSHER_PIN);
  pusherMedium.attach(MEDIUM_PUSHER_PIN);
  pusherLarge.attach(LARGE_PUSHER_PIN);
  feederServo.attach(FEEDER_SERVO_PIN);

  stopperServo.write(SERVO_CLOSE_ANGLE);
  pusherSmall.write(HOME_ANGLE);
  pusherMedium.write(HOME_ANGLE);
  pusherLarge.write(HOME_ANGLE);
  feederServo.write(0);

  pinMode(IR_SENSOR_PIN, INPUT_PULLUP);
  pinMode(IR_LED_PIN, OUTPUT);
  pinMode(IR_COUNT1_PIN, INPUT_PULLUP);
  pinMode(IR_COUNT2_PIN, INPUT_PULLUP);
  pinMode(IR_COUNT3_PIN, INPUT_PULLUP);

  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);

  motorReverseSlow(17); // ============Speed of Gear Motor============
}

// ==================== LOOP ====================
void loop() {

  // ---------- RECEIVE SCHEDULES ----------
if (espSerial.available()) {
    String line = espSerial.readStringUntil('\n');
    line.trim();
    Serial.print("Received schedule: "); Serial.println(line);

    if (line == "END") {
      // All schedules sent, do nothing here
      Serial.println("All schedules received");
    }

    if (line.startsWith("F,")) {
      int hour = line.substring(2,4).toInt();
      int minute = line.substring(5,7).toInt();
      int portion = line.substring(8).toInt();

      // Check if schedule already exists
      bool exists = false;
      for (int i=0; i<feedCount; i++) {
        if (feedHour[i]==hour && feedMinute[i]==minute) {
          exists = true;
          break;
        }
      }

      // Only add if it doesn't exist
      if (!exists && feedCount < MAX_FEED_SCHEDULES) {
        feedHour[feedCount]   = hour;
        feedMinute[feedCount] = minute;
        feedPortion[feedCount]= portion;
        feedDone[feedCount]   = false; // mark as not fed yet
        feedCount++;
        Serial.print("Added schedule: ");
        Serial.print(hour); Serial.print(":"); Serial.println(minute);
      }
    }
}

  // ---------- CHECK FEED TIME ----------
DateTime now = rtc.now();
for (int i = 0; i < feedCount; i++) {
  // If current time matches schedule and not yet fed
  if (!feedDone[i] &&
      now.hour() == feedHour[i] &&
      now.minute() == feedMinute[i]) {

    Serial.print("Feeding at: ");
    Serial.print(now.hour());
    Serial.print(":");
    Serial.println(now.minute());

    dispenseFood(feedPortion[i]);
    feedDone[i] = true;  // mark as done, will not feed again this minute
  }

  // Reset feedDone only when the current minute is NOT equal to schedule
  // This ensures it can feed again at the next scheduled time
  if (now.minute() != feedMinute[i] || now.hour() != feedHour[i]) {
    feedDone[i] = false;
  }
}
  

  // ---------- EGG SORTING (WEIGHT DETECTION) ----------
  if (scale.is_ready()) {
    float r = scale.get_units(1);

    if (r >= 35) {
      if (!eggOnScale) {
        eggOnScale = true;
        eggDetectedTime = millis();
      }

      weightSum += r;
      weightReadings++;

      if (weightReadings >= MAX_READINGS &&
          millis() - eggDetectedTime >= 2000) {

        float avg = weightSum / weightReadings;
        int cat = (avg <= 45) ? 1 : (avg <= 55) ? 2 : 3;

        addEggToQueue(cat);
        stopperServo.write(SERVO_OPEN_ANGLE);
        stopperOpenTime = millis();
        stopperClosing = true;

        weightSum = 0;
        weightReadings = 0;
        eggOnScale = false;
      }
    }
  }

  // ---------- STOPPER CONTROL ----------
  if (stopperClosing && millis() - stopperOpenTime >= STOPPER_DURATION) {
    stopperServo.write(SERVO_CLOSE_ANGLE);
    stopperClosing = false;
  }

  // ---------- PLATFORM IR DETECTION & PUSHER (BULK SORTING FIX) ----------
  int irState = digitalRead(IR_SENSOR_PIN);
  digitalWrite(IR_LED_PIN, irState == LOW ? HIGH : LOW);

  // When IR detects an egg, move from weight queue to platform queue
  if (irState == LOW && queueStart != queueEnd) {
    int nextEggCat = eggQueue[queueStart];
    addEggToPlatformQueue(nextEggCat);
    queueStart = (queueStart + 1) % MAX_EGGS; // advance classification queue
  }

  // Handle all eggs on platform (bulk sorting)
  handlePlatformQueue();

    // ---------- INDEPENDENT IR SENSOR CHECK ----------
  bool ir1 = digitalRead(IR_COUNT1_PIN);
  bool ir2 = digitalRead(IR_COUNT2_PIN);
  bool ir3 = digitalRead(IR_COUNT3_PIN);

  // Check each sensor for object (assuming LOW = object detected)
  if (ir1 == LOW && lastIR1 == HIGH) {
    Serial.write('S'); // send Small detected to ESP
  }
  if (ir2 == LOW && lastIR2 == HIGH) {
    Serial.write('M'); // send Medium detected to ESP
  }
  if (ir3 == LOW && lastIR3 == HIGH) {
    Serial.write('L'); // send Large detected to ESP
  }

  // Update last states for edge detection
  lastIR1 = ir1;
  lastIR2 = ir2;
  lastIR3 = ir3;

}
