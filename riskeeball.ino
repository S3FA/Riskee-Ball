#include <Button.h>
#include <SPI.h>
#include <Tlc5940.h>
#include "FastSPI_LED2.h"

// hacky fix for led chain glitching:
//   break it out into two chains!
// todo: add rs-485 functionality

#define MACHINEID  '1'  // nodes '1' through 'A' for type-ability

#define CTRLRE     5
#define CTRLDE     6
#define ARMEDPIN   7
#define LEDPIN     8
#define PIN20      14
#define PIN30      15
#define PIN40      16
#define PIN50      17
#define PIN100     18
#define PIN10      19
#define PIN0       2

#define FLAME20     0
#define FLAME30     1
#define FLAME40     2
#define FLAME50     3
#define FLAME100    4
#define BALLRELEASE 5
#define SPINLIGHT   6
#define KLAXON      7

#define BALLRELTIME 10000 // ball release time at start of game
#define FIRELEN     1000  // length of fire blast in usec
#define LEDSEGLEN   5     // how many LEDs per digit segment
#define LEDSEGLEN2  3     // same, for ball #
//#define NUM_LEDS    ((LEDSEGLEN * 21) + LEDSEGLEN2 * 7) 
#define NUM_LEDS 70
#define NUM_LEDS2 56


byte n;
byte c;
byte ballnum;
unsigned short score;
boolean crazy;
boolean idle;

Button holes[7] = { 
  Button(PIN20, BUTTON_PULLUP_INTERNAL, true, 1000), 
  Button(PIN30, BUTTON_PULLUP_INTERNAL, true, 1000), 
  Button(PIN40, BUTTON_PULLUP_INTERNAL, true, 1000),
  Button(PIN50, BUTTON_PULLUP_INTERNAL, true, 1000),
  Button(PIN100, BUTTON_PULLUP_INTERNAL, true, 1000),
  Button(PIN10, BUTTON_PULLUP_INTERNAL, true, 1000),
  Button(PIN0, BUTTON_PULLUP_INTERNAL, true, 1000),
};
byte points[7] = {20, 30, 40, 50, 100, 10, 0};
unsigned long firetime[5];
unsigned long ballreltime;

CRGB leds[NUM_LEDS];
CRGB leds2[NUM_LEDS2];
WS2811Controller800Mhz<LEDPIN> LED;
WS2811Controller800Mhz<19> LED2;


// setup
void setup() {

  Tlc.init();
  Tlc.clear();
  Tlc.update();
  
  for(c=0;c<5;c++) {
    firetime[c] = 0;
  }
  
  Serial.begin(115200);
  Serial.println("starting...");
  
  LEDS.addLeds<WS2811, LEDPIN, GRB>(leds, NUM_LEDS);
  LEDS.addLeds<WS2811, 19, GRB>(leds2, NUM_LEDS2);
  
  int x;
  memset(leds, 0, NUM_LEDS * sizeof(struct CRGB));
  memset(leds2, 0, NUM_LEDS2 * sizeof(struct CRGB));
  LEDS.show();
  
  
  delay(1000);
  
  /*
  for(x=0;x<=999;x++) {
    updatescore(x,x%10);
    delay(500);
  }
  */
  
  /*
  int y;
  for(y=0;y<=NUM_LEDS;y++) {
    for(x=0;x<y;x++) {
      leds[y].r = 255;
      leds[y].g = 0;
      leds[y].b = 0;
    }
    LEDS.show();
    delay(50);
  }
  */
  
  idle = true;

}

void startgame() {
  ballnum = 0;
  score = 0;
  idle = false;
  crazy = false;
  updatescore(score, ballnum);
  Tlc.set(BALLRELEASE, 4095);
  ballreltime = millis() + BALLRELTIME;
  Tlc.update();
}

void endgame() {
  Tlc.clear();
  Tlc.update();
  ballnum = 9;
  idle = true;
  crazy = false;
}

void startcrazy() {
  crazy = true;
  Tlc.set(SPINLIGHT, 4095);
  Tlc.set(KLAXON, 4095);
  Tlc.update();
}

void endcrazy() {
  crazy = false;
  Tlc.set(SPINLIGHT, 0);
  Tlc.set(KLAXON, 0);
  Tlc.update();
}

// this assumes four digits: three for score, one for balls thrown
//     x1x     x 8x     x15x
//    x   x   x    x   x    x
//    0   2   7    9   14   16
//    x   x   x    x   x    x
//     x3x     x10x     x17x
//    x   x   x    x   x    x
//    4   6   11   13  20   18
//    x   x   x    x   x    x
//     x5x     x12x     x19x
//
//             x22x         
//            x    x  
//            21   23  
//            x    x  
//             x24x   
//            x    x  
//            25   27  
//            x    x  
//             x26x   

byte DIGITS[] = { 0x77, 0x44, 0x3E, 0x6E, 0x4D, 0x6B, 0x7B, 0x46, 0x7F, 0x4F };

void updatescore(unsigned short score, byte ball) {
  unsigned short i, j, k;
  byte lite[4];
  memset(leds, 0, NUM_LEDS * sizeof(struct CRGB));
  memset(leds2, 0, NUM_LEDS2 * sizeof(struct CRGB));
 
  Serial.print("ball = ");
  Serial.println(ball);
  
  i = score / 100;
  for(j=0;j<7;j++) {
    if( (1<<j) & DIGITS[i] ) {
      for(k=0;k<LEDSEGLEN;k++) {
        leds[j*LEDSEGLEN + k].r = 255;
      }
    }
  }
  
  i = (score / 10) % 10;
  for(j=0;j<7;j++) {
    if( (1<<j) & DIGITS[i] ) {
      for(k=0;k<LEDSEGLEN;k++) {
        leds[(j+7)*LEDSEGLEN + k].r = 255;
      }
    }
  }
  
  
  i = score % 10;
  for(j=0;j<7;j++) {
    if( (1<<j) & DIGITS[i] ) {
      for(k=0;k<LEDSEGLEN;k++) {
        //leds[(j+14)*LEDSEGLEN + k].r = 255;
        leds2[j*LEDSEGLEN + k].r = 255;
      }
    }
  }
  
  for(j=0;j<7;j++) {
    if( (1<<j) & DIGITS[ball] ) {
      for(k=0;k<LEDSEGLEN2;k++) {
        //leds[21*LEDSEGLEN + j*LEDSEGLEN2 + k].r = 255;
        leds2[7*LEDSEGLEN + j*LEDSEGLEN2 + k].r = 255;
      }
    }
  }
        
  LEDS.show();
}

// main loop
void loop() {
  
  // turn off active flames
  for(c=0;c<5;c++) {
    if( firetime[c] < millis() ) {
      Tlc.set(c,0);
    }
  }
  
  if( ballreltime < millis() ) {
    Tlc.set(BALLRELEASE, 0);
  }
  Tlc.update();

  if(!idle) {
    // check button presses
    for(c=0;c<7;c++) {
      if( holes[c].uniquePress() ) {
        score += points[c];
        Serial.print("points scored: ");
        Serial.print(points[c]);
        Serial.print(" total score: ");
        Serial.print(score);
        ballnum += 1;
        Serial.print(" ball number: ");
        Serial.println(ballnum);
        updatescore(score, ballnum);
        if(c < 5) {
          firetime[c] = millis() + FIRELEN; 
          Tlc.set(c,4095);
        }
        if(ballnum >= 9) {
          endgame();
        }
      }
    }
    Tlc.update();
  }
  
  // serial messages:
  // # is '0' - '9', 'A' or '*' for broadcast
  // '#S' - start new game
  // '#+' - enter crazy mode
  // '#-' - leave crazy mode
  // '#X' - quit current game
  // '#?' - query ball number
  // '#0' - '#4' - crazyfire solenoid 0 through 4
  if(Serial.available() >= 2) {
    n = Serial.read();
    c = Serial.read();
    if( (n == MACHINEID) || (n == '*')) {
      switch(c) {
        case 'S' :
          Serial.println("starting game");
          startgame();
          break;
        case 'X' :
          Serial.println("ending game");
          endgame();
          break;
        case '+' :
          Serial.println("CRAZY MODE");
          startcrazy();
          break;
        case '-' :
          Serial.println("returning to sanity");
          endcrazy();
          break;
        case '?' : 
          Serial.write(score);
          Serial.print(ballnum);
          break;
        case '0' :
          if(crazy) {
            firetime[0] = millis() + FIRELEN;
            Tlc.set(0, 4095);
          }
          break;
        case '1' :
          if(crazy) {
            firetime[1] = millis() + FIRELEN;
            Tlc.set(1, 4095);
          }
          break;
        case '2' :
          if(crazy) {
            firetime[2] = millis() + FIRELEN;
            Tlc.set(2, 4095);
          }
          break;
        case '3' :
          if(crazy) {
            firetime[3] = millis() + FIRELEN;
            Tlc.set(3, 4095);
          }
          break;
          case '4' :
          if(crazy) {
            firetime[4] = millis() + FIRELEN;
            Tlc.set(4, 4095);
          }
          break;
      }
      Tlc.update();
    }
  }

}


