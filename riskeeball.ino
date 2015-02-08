#include <Button.h>
#include <SPI.h>
#include <Tlc5940.h>

// Make sure that tlc_config.h file changed to say
// #define NUM_TLCS    2 
//See 'Using two or more TLC5940s': http://tronixstuff.com/2013/10/21/tutorial-arduino-tlc5940-led-driver-ic/

// Duemilanove ATmega328
// ttyusbserial
// todo: add rs-485 functionality

// DEBUG 1 = debug serial output w/USB cable; DEBUG 0 = serial output over RS-485
#define DEBUG 1

//http://alex.kathack.com/codes/tlc5940arduino/html_r011/group__CoreFunctions.html
//4095 is the max value for a channel on a tlc5940
#define TLC_ON 4095
#define TLC_OFF 0

//Arduino
#define CTRLRE     5
#define CTRLDE     6
#define ARMEDPIN   7 // System armed - Deadman pin - automatically tripped by applying power to Solenoid DC line?
#define LEDPIN     8 // *** Necessary?
#define PIN20      14 // Ball switch 20 points
#define PIN30      15 // Ball switch 30 points
#define PIN40      16 // Ball switch 40 points
#define PIN50      17 // Ball switch 50 points
#define PIN100     18 // Ball switch 100 points
// #define PIN10      19  // *** Not used (never existed)
//#define PIN0       2   // *** Not used (no score hole switch was always unreliable, omitted)

//TLC5940
  // Solenoids
#define FLAME20     0  // Solenoid 20 points
#define FLAME30     1  // Solenoid 30 points
#define FLAME40     2  // Solenoid 40 points
#define FLAME50     3  // Solenoid 50 points
#define FLAME100    4  // Solenoid 100 points
//#define BALLRELEASE 5  // *** Not used (ball release switch was unreliable, omitted)

// Scoreboard!
// Zeros digit simply turns on when the 
#define LEDONE      6

// Tens digit 7-segment
#define LEDTENA     7
#define LEDTENB     8
#define LEDTENC     9
#define LEDTEND     10
#define LEDTENE     11
#define LEDTENF     12
#define LEDTENG     13

// Hundreds digit 7-segment
#define LEDHUNDREDA 14
#define LEDHUNDREDB 15
#define LEDHUNDREDC 16
#define LEDHUNDREDD 17
#define LEDHUNDREDE 18
#define LEDHUNDREDF 19
#define LEDHUNDREDG 20

#define FIRELEN     1000  // length of fire blast in usec

//Pin to Digit Conversion (Following 7 seg cathode configuration)
int ONE[] = {LEDONE};
int TEN[] = {LEDTENA, LEDTENB, LEDTENC, LEDTEND, LEDTENE, LEDTENF, LEDTENG };
int HUNDRED[] = {LEDHUNDREDA, LEDHUNDREDB, LEDHUNDREDC, LEDHUNDREDD, LEDHUNDREDE, LEDHUNDREDF, LEDHUNDREDG};
int ALLSEGS[] = {LEDONE, LEDTENA, LEDTENB, LEDTENC, LEDTEND, LEDTENE, LEDTENF, LEDTENG, LEDHUNDREDA, 
      LEDHUNDREDB, LEDHUNDREDC, LEDHUNDREDD, LEDHUNDREDE, LEDHUNDREDF, LEDHUNDREDG};

unsigned short score;

//points must be same length as holes and firetime
unsigned long firetime[5];
Button holes[5] = { 
  Button(PIN20, BUTTON_PULLUP_INTERNAL, true, 1000),  
  Button(PIN30, BUTTON_PULLUP_INTERNAL, true, 1000), 
  Button(PIN40, BUTTON_PULLUP_INTERNAL, true, 1000),
  Button(PIN50, BUTTON_PULLUP_INTERNAL, true, 1000),
  Button(PIN100, BUTTON_PULLUP_INTERNAL, true, 1000)
};

byte points[5] = {20, 30, 40, 50, 100};

byte seven_seg_digits[10][7] = { { 1,1,1,1,1,1,0 },  // ZERO
  { 0,1,1,0,0,0,0 }, // ONE
  { 1,1,0,1,1,0,1 }, // TWO
  { 1,1,1,1,0,0,1 }, // THREE
  { 0,1,1,0,0,1,1 }, // FOUR
  { 1,0,1,1,0,1,1 }, // FIVE
  { 1,0,1,1,1,1,1 }, // SIX
  { 1,1,1,0,0,0,0 }, // SEVEN
  { 1,1,1,1,1,1,1 }, // EIGHT
  { 1,1,1,0,0,1,1 }  // NINE
};

// setup
void setup() {

  Tlc.init();
  Tlc.clear();
  Tlc.update();
  
  score = 0;
  
  updatescore(score);
  
  for(c=0;c < sizeof(firetime);c++) {
    firetime[c] = 0;
  }
  
  // *** WHAT DID THIS DO? *** //
  /*Serial.begin(57600); // *** WHAT DID THIS DO? *** //
  
  pinMode(CTRLRE, OUTPUT); // RE receive active low
  if( DEBUG ) {
    digitalWrite(CTRLRE, HIGH);
    Serial.println("starting...");
  }
  else {
    digitalWrite(CTRLRE, LOW);
  }
  pinMode(CTRLDE, OUTPUT); // DE transmit active high
  digitalWrite(CTRLDE, LOW);

  delay(1000);
  
  idle = true;*/
  // *** WHAT DID THIS DO? *** //
 
}


// three digits for score TLC 4950 Pin Numbers (7 Segment Common Cathode Configuration)
//     x 14 x     x  7 x     x 6 x
//    x      x   x      x   x     x
//    19    15  12      8   6     6
//    x      x   x      x   x     x
//     x 20 x     x 13 x     x 6 x
//    x      x   x      x   x      x
//    18     16 11     9    6      6
//    x      x   x      x   x      x
//     x 17 x     x 10 x     x 6 x

  
//Should explicitly update all segments of the display
void updatescore(unsigned short score) {
  unsigned short i, j, k;

 //TODO: Zero Out Digits
 for(c=0;c<sizeof(ALLSEGS);c++) {
    Tlc.set(ALLSEGS[c],TLC_OFF);
  }
      
  //Ten Digit
  i = (score / 10) % 10;
  for(j=0;j<7;j++) {
    if(seven_seg_digits[i][j]==1){
      Tlc.set(TEN[j],TLC_ON);
    }
  }
  
  //Hundred Digit
  i = (score / 100) % 10;
  for(j=0;j<7;j++) {
    if(seven_seg_digits[i][j]==1){
      Tlc.set(HUNDRED[j],TLC_ON);
    }
  }
  
  //Zero Digit (on or off)
  Tlc.set(ONE[0],TLC_ON);
  
  Tlc.update();
  
}

// main loop
void loop() {
  
  // turn off active flames when their time is up
  for(c=0;c < sizeof(firetime);c++) {
    if( firetime[c] < millis() ) {
      Tlc.set(c,TLC_OFF);
    }
  }
  
  Tlc.update();

  // check button presses
  for(c=0;c < sizeof(holes);c++) {
    if( holes[c].uniquePress() ) {
      score += points[c];
      updatescore(score);
      
      if( DEBUG ) {
        Serial.print("points scored: ");
        Serial.print(points[c]);
        Serial.print(" total score: ");
        Serial.print(score);
      }

      firetime[c] = millis() + FIRELEN; 
      Tlc.set(c,TLC_ON); //Flame on!

    }
    
    Tlc.update();
  }
}




