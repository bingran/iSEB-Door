#include "U8glib.h"
#include "pitches.h"
#include <EEPROM.h>

int addr = 0;

/* Pinout Definition */
#define buzzerPin 11
#define btnPin A2
#define signalPin  A3

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);	// I2C / TWI

int buzzerTimeout = 0;
int ledTimeout = 0;
int doorCount  = 0;
int signalDebounce = 50;
char buffer[15];

/* button */
bool bButton = false;
bool bSignal = true; /* by default put close to prevent buzzer beep */

/* buzzer */
bool bfBuzzer = 1;
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

void buzzerPlay ()
{
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(buzzerPin, melody[thisNote], noteDuration);
    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(buzzerPin);
  }

}

void draw(void) {
  // graphic commands to redraw the complete screen should be placed here  
  u8g.setFont(u8g_font_unifont);
  //u8g.setFont(u8g_font_osb21);
  if(false == bSignal)
  {
    u8g.drawStr( 0, 22, "Door: Open" );
    buzzerTimeout++;
    if(20 < buzzerTimeout)
    {
      buzzerTimeout = 0;
    }
    else if(10 < buzzerTimeout)
    {
      Serial.println("No Buzzer");
      // stop the tone playing:
      noTone(buzzerPin);
    }
    else
    {
      Serial.println("Buzzer");
      tone(buzzerPin, melody[3], 2000);
    }
  }
  else
  {
    u8g.drawStr( 0, 22, "Door: Close" );
    buzzerTimeout = 0;
    noTone(buzzerPin);
  }
  sprintf(buffer, "Count :%d", doorCount);
  u8g.drawStr( 0, 44, buffer );
  
}

void setup(void) {
  Serial.begin(9600);
  // flip screen, if required
  // u8g.setRot180();
  
  // set SPI backup if required
  //u8g.setHardwareBackup(u8g_backup_avr_spi);

  // assign default color value
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }


  pinMode(btnPin, INPUT);  // sets the digital pin 13 as output
  pinMode(signalPin, INPUT_PULLUP);    // sets the digital pin 7 as input

  buzzerPlay();

  doorCount = EEPROM.read(addr);
}

void loop(void) {
 
  if (bButton != digitalRead(btnPin)) {
    bButton = digitalRead(btnPin);
    if (0 == bButton) {
      Serial.println("Button is pressed");
      doorCount = 0;
      EEPROM.write(addr, doorCount);
    } else {
      Serial.println("Button is released");
    }
  }

  if (bSignal != digitalRead(signalPin)) 
  {
    signalDebounce--;
    if(0 == signalDebounce)
    {
      bSignal = digitalRead(signalPin);
      if (0 == bSignal) {
        Serial.println("Door is open");
      }
      else {
        doorCount++;
        EEPROM.write(addr, doorCount);
        Serial.println("Door is close");
      }
    }
  }
  else
  {
    signalDebounce = 50;
  }

  if(0 != ledTimeout)
  {
    ledTimeout--;
  }

  if(0 == ledTimeout)
  {
    ledTimeout = 50;
    // picture loop
    u8g.firstPage();  
    do {
      draw();
    } while( u8g.nextPage() );
  }
  // rebuild the picture after some delay
  delay(1);
}
