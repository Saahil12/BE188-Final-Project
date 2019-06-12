#include "FinalProjectLib.h" 
#include <FastLED.h>
#include <DS3231.h>
#include <LiquidCrystal.h>

#define NUM_LEDS 60 //The amount of LEDs in the setup
#define LED_PIN 2 //The pin that controls the LEDs
#define ANALOG_READ 1 //The pin that we read sensor values form
#define MIC_LOW 0.0 //Confirmed microphone low value, and max value
#define MIC_HIGH 755.0
#define AVGLEN 5 //How many previous sensor values effects the operating average
#define LONG_SECTOR 20 //How many previous sensor values decides if we are on a peak/HIGH 
#define HIGH 3 //Mneumonics
#define NORMAL 2
#define MSECS 30 * 1000 //How long do we keep the "current average" sound, before restarting the measuring
#define CYCLES MSECS / DELAY
#define DELAY 1
#define DEV_THRESH 0.8 //how much a reading is allowed to deviate from thee average before being discarded
#define POTENT_READ 0 //The pin that we read the potentiometer from 
#define SWITCH_PIN 7//The pin that we read the switch from 

int curshow = NUM_LEDS; //Number of LEDs to display
int mode = 0; //switch between sound reactive mode and solid white 
int songmode = NORMAL; //Showing different colors based on the mode.
unsigned long song_avg; //Average sound measurement the last CYCLES
int iter = 0; //The amount of iterations since the song_avg was reset
float fade_scale = 1.2; //The speed the LEDs fade to black if not relit
const int rs = 12, en = 11, d4 = 8, d5 = 9, d6 = 10, d7 = 13;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
DS3231  rtc(SDA, SCL);

//Arrays
CRGB leds[NUM_LEDS]; //LED array
int avgs[AVGLEN] = {-1}; //Short sound avg used to "normalize" the input values.
int long_avg[LONG_SECTOR] = {-1}; //Longer sound avg

//Keeping track how often, and how long times we hit a certain mode
struct time_keeping {
  unsigned long times_start;
  short times;
};

//How much to increment or decrement each color every cycle
struct color {
  int r;
  int g;
  int b;
};

//create objects of classes 
struct time_keeping high;
struct color Color; 

void setup() {
  Serial.begin(9600);
  pinMode(7, INPUT); //slide button 
  rtc.begin();
  lcd.begin(16,2); //16 columns and 2 rows
  //rtc.setDOW(MONDAY);     // comment once set
  //rtc.setTime(23, 24, 30);     // comment once set
  //rtc.setDate(27, 05, 2019);   // comment once set
  
  //Set all lights to make sure all are working as expected
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  for (int i = 0; i < NUM_LEDS; i++) 
    leds[i] = CRGB(0, 0, 255); //set them to blue
  FastLED.show(); 
  delay(1000);  

  //bootstrap average with some low values
  for (int i = 0; i < AVGLEN; i++) {  
    insert(250, avgs, AVGLEN);
  }

  //Initial values
  high.times = 0;
  high.times_start = millis();
  Color.r = 0;  
  Color.g = 0;
  Color.b = 1;
}

/*With this we can change the mode if we want to implement a general 
lamp feature, with for instance general pulsing. Maybe if the
sound is low for a while? */ 
void loop() {
  if(rtc.getDOWStr() == "Wednesday") {
    lcd.setCursor(3, 0);
    lcd.print(rtc.getDOWStr());
  }
  else if (rtc.getDOWStr() == "Thursday" || rtc.getDOWStr() == "Saturday" || rtc.getDOWStr() == "Tuesday") {
    lcd.setCursor(4, 0);
    lcd.print(rtc.getDOWStr());
  }
  else if (rtc.getDOWStr() == "Monday" || rtc.getDOWStr() == "Sunday" || rtc.getDOWStr() == "Friday") {
    lcd.setCursor(5, 0);
    lcd.print(rtc.getDOWStr());
  }
  lcd.setCursor(4, 1);
  lcd.print(rtc.getTimeStr());
  mode = digitalRead(SWITCH_PIN);
  switch(mode) {
    case 0:
      visualize_music();
      break;
    case 1:
      solid_light();
      break;
    default:
      break;
  }
    delay(DELAY);       // delay in between reads for stability
}
