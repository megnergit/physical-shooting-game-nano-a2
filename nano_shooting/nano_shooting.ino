// ====== LDR + Melody (Arduino Nano, Passive Buzzer on D8) ======
#define C4  262
#define D4  294
#define E4  330
#define G4  392
#define A4  440
#define H4  494

#define C5  523
#define D5  587
#define E5  659
#define F5  698
#define FS5 740
#define G5  784
#define A5  880
#define H5  988
#define C6  1047
#define D6  1175
#define E6  1319
#define NN  0     // no sound

const int BUZZER_PIN = 9;   // active busser
const int LED_PIN = 8;
const int SENSOR_PIN = A0;  // read LDR partial voltage

//int dr1 = 150;              // length of one tone
int dr1 = 150;              // length of one tone
unsigned long coolDownMs = 500; // avoid multiple hits at a time

// ---- thresholds. need to adjust 
int onThr  = 96;  // if smaller than this, counted as a hit.
int offThr = 128;  // if larger than this, target is reset (= ready to get shot). 

bool beamOn = true; // at start, target is assumed to be in 'hit'-state.
bool waitingForReset = false;
bool last = true; 

// average signals to reduce noise.
int readA0Avg(uint8_t n=8){
  long s = 0;
  long a = 0;
  for(uint8_t i=0;i<n;i++){ 
    a = analogRead(A0);
//    s += analogRead(A0);
    s += a;
//    Serial.println(a);
    delay(1); 
    }
  return (int)(s / n);
}

// Popcorn (https://www.youtube.com/watch?v=NjxNnqTcHhg)
int mel_pop[] = {
  A5, G5, 
  A5, E5, C5, E5, A4, NN, A5, G5, 
  A5, E5, C5, E5, A4, NN, A5, H5, 
  C6, H5, C6, A5, H5, A5, H5, G5, 
  A5, G5, A5, F5, A5, NN, A5, G5, 
  A5, E5, C5, E5, A4, NN, A5, G5, 
  A5, E5, C5, E5, A4, NN, A5, H5, 
  C6, H5, C6, A5, H5, A5, H5, G5, 
  A5, G5, A5, H5, C6, NN, E5, D5,
  E5, C5, G4, C5, E4, NN, E5, D5, 
  E5, C5, G4, C5, E4, NN, E5, FS5, 
  G5, FS5, G5, E5, FS5, E5, FS5, D5,
  E5, D5, E5, C5, E5, NN, E5, D5, 
  E5, C5, G4, C5, E4, NN, E5, D5, 
  E5, C5, G4, C5, E4, NN, E5, FS5, 
  G5, FS5, G4, E5, FS5, E5, FS5, D5,
  E5, D5, C5, D5, E5, NN, NN, NN,
  A4, C5, A4, C5, A4, C5, A4, C5, 
  A4, NN, E5, NN, A5, NN, NN, NN,
  NN, NN, NN, NN, NN, NN, NN, NN
};

// interstellar
int mel_ins[] = {
  E6, H5, C6, D6,
  E6, H5, C6, D6,
  E6, H5, C6, D6,
  E6, NN, NN, NN,
  E6, NN, NN, NN,
  E6, NN, NN, NN,
  E6, NN, NN, NN,
  E6, NN, NN, NN,
  E6, NN, NN, NN
};
  
// Popcorn but only for first two tacts.
int mel[] = {
  A5, G5, 
  A5, E5, C5, E5, A4, NN 
};
void playOne(int freq, int dur_ms){
  if(freq > 0){
    tone(BUZZER_PIN, freq);
  }else{
    noTone(BUZZER_PIN);
  }
  delay(dur_ms);
  noTone(BUZZER_PIN);
}

void playMelody(){
  const int n = sizeof(mel)/sizeof(mel[0]);
  for(int i = 0; i < n; i++){
    playOne(mel[i], dr1);
  }
}

void setup(){
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);
  Serial.begin(115200);

  Serial.println("LDR + Melody test start");

  // initizlize the target
  int v0 = readA0Avg();
  beamOn = (v0 < onThr);  // smaller the value, the brighter the surroundings.
  waitingForReset = false; 
  last = beamOn;          // wait for the first hit

}


void loop(){
  // read the LDR and average the signal
  int v = readA0Avg();
  // uncomment the following line to measure the ambient light. 
//  Serial.println(v);  

  // current beam state.
  if(!beamOn && v < onThr)  beamOn = true;   // hit
  if(beamOn  && v > offThr) beamOn = false;  // deactivate LDR

 
  // while the target is hit, but LDR is innactive, do nothing.
  if(waitingForReset){
    if(beamOn){
      // when LDR is reset, you may hit again.
      waitingForReset = false;
      Serial.println("RESET");  // show the status to serial monitor
    }
    delay(2);
    return;
  }

  // wait here until the game is started, or LDR is active again.
  static bool last = true;
  if(last && beamOn){
    // 'hit' makes LDR deactive in the same time
    Serial.print("HIT! A0="); Serial.println(v);
    
    // LED
    digitalWrite(LED_PIN, HIGH);
    playMelody();
    digitalWrite(LED_PIN, LOW);

    waitingForReset = true;  // wait. 
  }
  last = beamOn;

  delay(2);
}