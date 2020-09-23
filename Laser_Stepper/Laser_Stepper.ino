#define PIN_DIR 2
#define PIN_STEP 3

#define PIN_LASER 4

long pos = 0;
long home = 0;
long steps = 0;
unsigned long stepDelay = 100;
unsigned long stepSize = 100;
bool stepping = false;
bool continuous = false;
unsigned long lastStepMicros;
bool pulse = false;
bool dir = 0;

unsigned long lastPos = 0;
int posPeriod = 5000;
bool newPos = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_DIR, OUTPUT);
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_LASER, OUTPUT);

  digitalWrite(PIN_DIR, 0);
  digitalWrite(PIN_LASER, 0);

  Serial.begin(19200);
}

void loop() {
  if (Serial.available() > 0) {
    int c = Serial.read();

    switch (c) {
      case 'l': // lower-case L, turn off laser
        digitalWrite(PIN_LASER, 0);
        break;

      case 'L': // turn on laser
        digitalWrite(PIN_LASER, 1);
        break;

      case '<': // 1 step left
        steps = -1;
        continuous = false;
        break;

      case '>': // 1 step right
        steps = 1;
        continuous = false;
        break;

      case '!': //Read #steps
        steps = Serial.parseInt();
        continuous = false;
        break;

      case 'p': //set step period in uS
        stepDelay = Serial.parseInt();
        break;

      case '-': //continuous left
        continuous = true;
        steps = -1;
        break;

      case '+': //continous right
        continuous = true;
        steps = 1;
        break;

      case 's': //stop continuous
        continuous = false;
        steps = 0;
        break;

      case 'c': //request count
        Serial.println(pos);
        break;

      case 'r': //reset position
        pos = 0;
        break;

      case 'h': // set home
        home = pos;
        break;
        
      case 'H': // goto home
        steps = home - pos;
        continuous = false;
        break;

      case 'G': // goto position
        steps = Serial.parseInt() - pos;
        continuous = false;
        break;
    }
  }
  
  unsigned long curMicros = micros();

  if (stepping) {
    //check step timing
    if (curMicros - lastStepMicros >= stepDelay) {
      lastStepMicros = curMicros;
      pulse = !pulse;
      dir = steps > 0;
      
      digitalWrite(PIN_STEP, pulse);
      digitalWrite(PIN_DIR, dir);

      if (pulse) {
        pos += dir ? 1 : -1;
        newPos = true;
      }

      if (!pulse && !continuous) {
        if (steps > 0) {
          steps -= 1;
        } else if (steps < 0) {
          steps += 1;
        }

        if (steps == 0) {
          stepping = false;
        }
      }
    }
  } else {
    //check if need to start stepping
    if (steps != 0) {
      stepping = true;
      pulse = false;      
      lastStepMicros = micros();
    }
  }

  if (newPos && curMicros - lastPos >= posPeriod) {
    newPos = false;
    Serial.print("c");
    Serial.println(pos);
    lastPos = curMicros;
  }
}
