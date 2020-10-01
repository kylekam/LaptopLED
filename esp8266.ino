#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#include <FastLED.h>

// VARIABLES -------------------------------------------------------------||
#define MAX_NUM_LEDS 86

int NUM_LEDS;
int FADE_SPEED;

bool initial = true;

CRGBArray<MAX_NUM_LEDS> leds = {};
CRGBArray<MAX_NUM_LEDS> temp_leds = {};

int i_led = 0;
int i_pixel = 0;
int init_index = 0;

String temp;

// FUNCTIONS -------------------------------------------------------------||

// nblendU8TowardU8 and fadeTowardColor taken from Kiernan Freitag
// Helper function that blends one uint8_t toward another by a given amount
void nblendU8TowardU8( uint8_t& cur, const uint8_t target, uint8_t amount)
{
  if( cur == target) return;
  
  if( cur < target ) {
    uint8_t delta = target - cur;
    delta = scale8_video( delta, amount);
    cur += delta;
  } else {
    uint8_t delta = cur - target;
    delta = scale8_video( delta, amount);
    cur -= delta;
  }
}

// Blend one CRGB color toward another CRGB color by a given amount.
// Blending is linear, and done in the RGB color space.
// This function modifies 'cur' in place.
CRGB fadeTowardColor( CRGB& cur, const CRGB& target, uint8_t amount)
{
  nblendU8TowardU8( cur.red,   target.red,   amount);
  nblendU8TowardU8( cur.green, target.green, amount);
  nblendU8TowardU8( cur.blue,  target.blue,  amount);
  return cur;
}

void createLED() {
  FastLED.addLeds<NEOPIXEL, 5>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(3.3, 300);
  FastLED.setBrightness(25);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  initial = false;
}

void initialLoop() {
  while(Serial.available() > 0) {
    switch(init_index) {
      case 0:
        NUM_LEDS = (int)Serial.read();
        break;
      case 1:
        FADE_SPEED = (int)Serial.read();
        Serial.print("Num of leds: ");
        Serial.println(NUM_LEDS);
        Serial.print("Fade speed: ");
        Serial.println(FADE_SPEED);
        createLED();
        break;
    }
    init_index++;
  }
}
// MAIN -------------------------------------------------------------||
void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.print("Arduino setup complete");
  
  // Waits for the first contact
  while(Serial.available() <= 0) {}
}

void loop() {
  
  //Initializes LED strip first, then fills the LED with color.
  if(initial) {
    initialLoop();
  } else {
      // Takes in the bytearray and translate it to each LED
      while(Serial.available() > 0) {
        temp_leds[i_led][i_pixel] = Serial.read();
        i_pixel++;
        if(i_pixel > 2) {
          i_pixel = 0;
          i_led++;
        }

        //Copies temp_leds into leds
        if(i_led == 86) {
          for (int i=0; i<NUM_LEDS; i++) {
            fadeTowardColor(leds[i], temp_leds[i], FADE_SPEED);
          }
          FastLED.show();
          i_pixel = 0;
          i_led = 0;
        }
      }     
   }
}  
