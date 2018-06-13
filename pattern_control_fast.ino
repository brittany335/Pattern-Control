
#include <SPI.h>
#include <SFE_MicroOLED.h>
#include <Wire.h> // I2C

SPISettings settingsA(2000000, MSBFIRST, SPI_MODE1);// setting our spi mode for the driver chips

volatile unsigned long prevTime = 0;// used in the interrupt
volatile int buttonState = 0;   // variable for reading the pushbutton status

//defining pins for the microoled
#define PIN_RESET A6  // Connect RST to pin 9
#define DC_JUMPER 0 // this refers to one of the solder pads that were physically shorted to use I2C on the micro led

MicroOLED oled(PIN_RESET, DC_JUMPER);// I2C declaration

//the pins for the buttons
const int buttonpin = A8;
const int buttonpin2 = A7;

enum State {flip, intro, option1, option2, option3, pattern1, pattern2, pattern3, intro_flipped, option1_flipped, option2_flipped, option3_flipped, pattern1_flipped, pattern2_flipped, pattern3_flipped };
//enum is the data type that consists of user defined varaibles ie like the words pattern, intro, ext
State state = flip; //telling the intial state to start at intro

//array for pattern 1 which includes turning on coumn 2, 4, 7, and 1 (in this order, note the columns arn't put in order)
unsigned int column_pattern1[] = {0b0000000100000100, 0b0000010000010000, 0b0100000010000010, 0b0000000010000010};
//array for pattern 2 which includes turning on coumn 3, 6,and 5
unsigned int column_pattern2[] = {0b0000001000001000, 0b0001000001000000, 0b0000100000100000};
//array for pattern 3 which includes turning on coumn 2,3,4,6,7,5, and 1
unsigned int column_pattern3[] = {0b0000000100000100, 0b0000001000001000, 0b0000010000010000, 0b0001000001000000, 0b0100000010000010, 0b0000100000100000, 0b0000000010000010};





//chip select pins for the halfdriver chips
const int css1 = 10;
const int css2 = A0;


void setup() {
  //Set up OLED
  oled.begin();    // Initialize the OLED
  oled.clear(ALL); // Clear the display's internal memory
  oled.clear(PAGE); // Clear the buffer.

  //Set up halfdrivers
  pinMode(css1, OUTPUT);
  pinMode(css2, OUTPUT);

  Serial.begin(9600);

  //Set up SPI
  SPI.begin();//beginging SPI
  SPI.beginTransaction(settingsA);//setting the SPI mode

  //Set up buttons
  //having the buttons be internal pulldowns
  pinMode(buttonpin, INPUT_PULLDOWN);
  pinMode(buttonpin2, INPUT_PULLDOWN);

  //Set up Interrupt
  //because the buttonpin is an input_pulldown it will be one when not pressed and zero when pressed
  //So if I want to detect when the button is pressed I chose falling because that is going from high to low
  attachInterrupt(buttonpin, button_pressed, FALLING);
}

void loop() {

  //Begin Finite State Machine
  switch (state) {
    //stating off in the state that lets the user choose if their want the screen to be flipped
    //this is so that if they are wearing the shirt they can easily see what they are pressing
    //verse if this is in an exhibition people can view what to press on a mannequin
    case flip:
      oled.setFontType(1);//setting font size
      oled.setCursor(0, 0);// Set the text cursor to the upper-left of the screen.
      oled.print(" Flip  Screen?");
      oled.setFontType(0);
      oled.setCursor(0, 30);
      oled.print("yes");
      oled.setCursor(40, 30);
      oled.print("no");
      oled.circle(10, 43, 3.5);
      oled.circle(50, 43, 3.5);
      oled.display();//displaying what we just sent to the OLED

      //based on which button pressed will decide if the finite statme machine will go to the flipped print or not
      if (digitalRead(buttonpin2) == HIGH) {
        oled.circleFill(50, 43, 3.5);
        oled.display();
        oled.clear(PAGE); // Clear the buffer.
        delay(500);
        state = intro;
      }
      if (digitalRead(buttonpin) != buttonState) {
        oled.circleFill(10, 43, 3.5);
        oled.display();
        oled.clear(PAGE); // Clear the buffer.
        delay(500);
        state = intro_flipped;
      }
      break;

    ////////////////////////////////////////////////////////
    ///////////UNFLIPPED FINITE STATE MACHINE/////////////////
    ////////////////////////////////////////////////////////

    case intro:
      oled.clear(PAGE);
      oled.setFontType(1);
      oled.setCursor(0, 0);  // Set the text cursor to the upper-left of the screen.
      oled.print("choose pattern");
      oled.setFontType(0);
      oled.setCursor(0, 30);
      oled.print("yes");
      oled.circle(10, 43, 3.5);
      oled.display();

      buttonState = 0;//making sure the buttonstate is set to zero again because if there is a bounce it will think
      //the button state changed

      if (digitalRead(buttonpin) != buttonState) {

        buttonState = 0; //reset button state so it won't automatically go into the next state

        //visually showing that the button was pressed by filling in the circle on the screen
        oled.circleFill(10, 43, 3.5);
        oled.display();

        delay(500);
        state = option1;

      }
      break;

    //note: option1, option2 and option3 are to dispaly waht we see on the screen and direct if we go into a pattern or not

    //pattern1 , pattern2, and pattern3 clear the screen from the option choices so that the screen is just blank and then goes into that pattern

    case option1:
      //choose between going to pattern 2 or choosing pattern 1
      oled.clear(PAGE); // Clear the buffer.
      oled.setFontType(0);
      oled.setCursor(0, 0);  // Set the text cursor to the upper-left of the screen.
      oled.print("Pattern 1");
      oled.setCursor(0, 20);
      oled.print("next");
      oled.setCursor(40, 20);
      oled.print("yes");
      oled.circle(10, 40, 7);
      oled.circle(50, 40, 7);
      oled.display();

      if (digitalRead(buttonpin2) == HIGH) {
        Serial.println("right button pressed");
        oled.circleFill(50, 40, 7);
        oled.display();
        delay(500);
        state = pattern1;
      }
      else if (digitalRead(buttonpin) == HIGH) {
        Serial.println("left button pressed");
        oled.circleFill(10, 40, 7);
        oled.display();
        delay(500);
        state = option2;
      }
      break;



    case option2:
      //choose between going to pattern 2 or choosing pattern 3?
      oled.clear(PAGE); // Clear the buffer.
      oled.setFontType(0);
      oled.setCursor(0, 0);  // Set the text cursor to the upper-left of the screen.
      oled.print("Pattern 2");
      oled.setCursor(0, 20);
      oled.print("next");
      oled.setCursor(40, 20);
      oled.print("yes");
      oled.circle(10, 40, 7);
      oled.circle(50, 40, 7);
      oled.display();

      if (digitalRead(buttonpin2) == HIGH) {
        Serial.println("right button pressed");
        oled.circleFill(50, 40, 7);
        oled.display();
        delay(500);
        state = pattern2;
      }
      else if (digitalRead(buttonpin) == HIGH) {
        Serial.println("left button pressed");
        oled.circleFill(10, 40, 7);
        oled.display();
        delay(500);
        state = option3;
      }
      break;

    case option3:
      //choose between going to pattern 2 or choosing pattern 3
      oled.clear(PAGE); // Clear the buffer.
      oled.setFontType(0);
      oled.setCursor(0, 0);  // Set the text cursor to the upper-left of the screen.
      oled.print("Pattern 3");
      oled.setCursor(0, 20);
      oled.print("next");
      oled.setCursor(40, 20);
      oled.print("yes");
      oled.circle(10, 40, 7);
      oled.circle(50, 40, 7);
      oled.display();

      if (digitalRead(buttonpin2) == HIGH) {
        Serial.println("right button pressed");
        oled.circleFill(50, 40, 7);
        oled.display();
        delay(500);
        state = pattern3;
      }
      else if (digitalRead(buttonpin) == HIGH) {
        Serial.println("left button pressed");
        oled.circleFill(10, 40, 7);
        oled.display();
        delay(500);
        state = intro;
      }
      break;


    //implimenting pattern1
    case pattern1:
      oled.clear(PAGE);
      oled.display();
      Serial.println("in pattern1 right before patternOne()");
      patternONE();
      Serial.println("in pattern1 right after patternOne()");
      if (digitalRead(buttonpin) != buttonState) {
        Serial.println("left button pressed");
        delay(500);
        state = option1;
      }
      break;

    //implimenting pattern2
    case pattern2:
      oled.clear(PAGE);
      oled.display();
      Serial.println("in pattern2 right before patternOne()");
      patternTWO();
      Serial.println("in pattern2 right after patternOne()");
      if (digitalRead(buttonpin) != buttonState) {
        Serial.println("left button pressed");
        delay(500);
        state = option2;
      }
      break;

    //implimenting pattern3
    case pattern3:
      oled.clear(PAGE);
      oled.display();
      Serial.println("in pattern3 right before patternOne()");
      patternTHREE();
      Serial.println("in pattern3 right after patternOne()");
      if (digitalRead(buttonpin) != buttonState) {
        Serial.println("left button pressed");
        delay(500);
        state = intro;
      }
      break;




    ////////////////////////////////////////////////////////
    ///////////FLIPPED FINITE STATE MACHINE/////////////////
    ////////////////////////////////////////////////////////


    case intro_flipped:
    oled.clear(PAGE);
      oled.flipVertical(true);
      oled.flipHorizontal(true);
      oled.setFontType(1);
      oled.setCursor(0, 0);  // Set the text cursor to the upper-left of the screen.
      oled.print("choose pattern");
      oled.setFontType(0);
      oled.setCursor(40, 30);
      oled.print("yes");
      oled.circle(50, 43, 3.5);
      oled.display();
      buttonState = 0;

      if (digitalRead(buttonpin) != buttonState) {

        buttonState = 0; //reset button state
        oled.circleFill(50, 43, 3.5);
        oled.display();
        delay(500);
        state = option1_flipped;

      }
      break;

    //note: option1_flipped, option2_flipped and option3_flipped are to dispaly what we see on the screen and direct if we go into a pattern or not

    //pattern1_flipped , pattern2_flipped, and pattern3_flipped clear the screen from the option choices

    case option1_flipped:
      //choose between going to pattern2_flipped or choosing pattern1_flipped

      oled.flipVertical(true);
      oled.flipHorizontal(true);
      oled.clear(PAGE); // Clear the buffer.

      //delay(300);
      oled.setFontType(0);
      oled.setCursor(0, 0);  // Set the text cursor to the upper-left of the screen.
      //oled.setFontType(2);
      //  oled.setCursor(0, 0);
      oled.print("Pattern 1");
      oled.setCursor(0, 20);
      oled.print("yes");
      oled.setCursor(40, 20);
      oled.print("next");
      oled.circle(10, 40, 7);
      oled.circle(50, 40, 7);
      oled.display();

      if (digitalRead(buttonpin2) == HIGH) {
        Serial.println("right button pressed");


        oled.circleFill(10, 40, 7);
        oled.display();
        delay(500);
        state = pattern1_flipped;
      }
      
      else if (digitalRead(buttonpin) == HIGH) {
        Serial.println("left button pressed");
        oled.circleFill(50, 40, 7);
        oled.display();
        delay(500);
        state = option2_flipped;
      }
      break;

      
    case option2_flipped:
     oled.flipVertical(true);
      oled.flipHorizontal(true);

      //choose between going to pattern 2 or choosing pattern 3?
      oled.clear(PAGE); // Clear the buffer.
      oled.setFontType(0);
      oled.setCursor(0, 0);  // Set the text cursor to the upper-left of the screen.
      oled.print("Pattern 2");
      oled.setCursor(0, 20);
      oled.print("yes");
      oled.setCursor(40, 20);
      oled.print("next");
      oled.circle(10, 40, 7);
      oled.circle(50, 40, 7);
      oled.display();

      if (digitalRead(buttonpin2) == HIGH) {
        Serial.println("right button pressed");
        
        oled.circleFill(10, 40, 7);
        oled.display();
        delay(500);
        state = pattern2_flipped;
      }
      else if (digitalRead(buttonpin) == HIGH) {
       Serial.println("left button pressed");
        oled.circleFill(50, 40, 7);
        oled.display();
        delay(500);
        state = option3_flipped;
      }
      break;



      
    case option3_flipped:
     oled.flipVertical(true);
      oled.flipHorizontal(true);
      //choose between going to pattern 2 or choosing pattern 3
      oled.clear(PAGE); // Clear the buffer.
      oled.setFontType(0);
      oled.setCursor(0, 0);  // Set the text cursor to the upper-left of the screen.
       oled.print("Pattern 3");
      oled.setCursor(0, 20);
      oled.print("yes");
      oled.setCursor(40, 20);
      oled.print("next");
      oled.circle(10, 40, 7);
      oled.circle(50, 40, 7);
      oled.display();

      if (digitalRead(buttonpin2) == HIGH) {
        Serial.println("right button pressed");
       
        oled.circleFill(10, 40, 7);
        oled.display();
        delay(500);
        state = pattern3_flipped;
      }
      else if (digitalRead(buttonpin) == HIGH) {
        Serial.println("left button pressed");
        oled.circleFill(50, 40, 7);
        oled.display();
        delay(500);
        state = intro_flipped;
      }
      break;


    case pattern1_flipped:
    
      oled.clear(PAGE);
      oled.display();
Serial.println("in pattern1 right before patternOne()");
      patternONE();
      Serial.println("in pattern1 right after patternOne()");
      if (digitalRead(buttonpin) != buttonState) {
        Serial.println("left button pressed");
        //         //delay to register that the button was pressed
        delay(500);
        //
        state = option1_flipped;
      }
      break;

    //implimenting pattern2
    case pattern2_flipped:
      oled.clear(PAGE);
      oled.display();
      Serial.println("in pattern2 right before patternOne()");
      patternTWO();
      Serial.println("in pattern2 right after patternOne()");
      if (digitalRead(buttonpin) != buttonState) {
        Serial.println("left button pressed");
        delay(500);
        state = option2_flipped;
      }
      break;
      
    //implimenting pattern3
    case pattern3_flipped:
      oled.clear(PAGE);
      oled.display();
      Serial.println("in pattern3 right before patternOne()");
      patternTHREE();
      Serial.println("in pattern3 right after patternOne()");
      if (digitalRead(buttonpin) != buttonState) {
        Serial.println("left button pressed");
        delay(500);
        state = intro_flipped;
      }
      break;

  }
}
////////////////////////////////////////////////////////
///////////PatternOne/////////////////
////////////////////////////////////////////////////////
void patternONE() {
  buttonState = 0;
  unsigned int chip1 = 0;
  unsigned int chip2 = 0;

  for (int i = 0; i < 4; ++i) {

    ///////////////Turning nitinol on///////////////

    digitalWrite(css2, LOW);//turns on the communication line to send the spi command to be sent
    chip2 = SPI.transfer16(column_pattern1[i]);
    Serial.print("chip2 column on high driver= ");//PLEASE NOTE: all these print statements are frame behinds so the print statment doesn't match what it's actually doing
    Serial.println(chip2, BIN);//printing binary value
    digitalWrite(css2, HIGH);//turns off the communication line to send the spi command to be sent

    //fixing some bugs on the driver chip that have happended before so just incase covering our basis 
    //////////////////////////////////////////////////////////////////
    //for some reason when it turns on for column 2 it will turn on column 7; turns on column 1 will turn on column 7; or turn on column 7 it will column1
    //to get rid of this issue I added these if statments to turn off the column that I don't want to be on

    //if column 2 is on turn off column 7
    if (column_pattern1[i] == 0b0000000100000100) {
      delay(1);
      digitalWrite(css2, LOW);
      chip2 = SPI.transfer16(0b0100000000000000);//sending a command to the driver to have column 7 off
      digitalWrite(css2, HIGH);

    }
    // if column 7 is on turn off column 1
    else if (column_pattern1[i] == 0b0100000010000010) {
      delay(1);
      digitalWrite(css2, LOW);
      chip2 = SPI.transfer16(0b000000000000000);//sending a command to the driver to have column 1 off
      digitalWrite(css2, HIGH);
    }

    //if column 1 is on turn off column 7
    else if (column_pattern1[i] == 0b0000000010000010) {
      delay(1);
      digitalWrite(css2, LOW);
      chip2 = SPI.transfer16(0b0100000000000000);
      digitalWrite(css2, HIGH);
    }
    //////////////////////////////////////////////////////////////////

    // turning row 4 and 6 to be a low driver
    digitalWrite(css1, LOW);
    chip1 = SPI.transfer16(0b0001010000000000);
    Serial.print("chip1 row 8 on high driver= ");
    Serial.println(chip1, BIN);
    digitalWrite(css1, HIGH);

    delay(1);//delay to so that the chip can process that we are about to turn on a different channel of drivers

    //turning row 8 to be a low driver
    digitalWrite(css1, LOW);
    chip1 = SPI.transfer16(0b0100000100000000);
    Serial.print("chip1 row 4 and 6 on low driver= ");
    Serial.println(chip1, BIN);
    digitalWrite(css1, HIGH);

    delay(4000);//having the nitinol stay on for x amount of seconds

    ///////////////Turning nitinol off///////////////

    //turning row 4 and 6 on high driver
    digitalWrite(css1, LOW);
    chip1 = SPI.transfer16(0b0001010001010000);
    Serial.print("chip1 row 8 on low driver= ");
    Serial.println(chip1, BIN);
    digitalWrite(css1, HIGH);

    delay(1);//delay to so that the chip can process that we are about to turn on a different channel of drivers

    //turning row 8 on high driver
    digitalWrite(css1, LOW);
    chip1 = SPI.transfer16(0b0100000100000100);
    Serial.print("chip1 row 4 and 6 on high driver= ");
    Serial.println(chip1, BIN);
    digitalWrite(css1, HIGH);

    digitalWrite(css2, LOW);
    chip2 = SPI.transfer16(column_pattern1[i]);
    Serial.println(chip2, BIN);
    digitalWrite(css2, HIGH);

    if (digitalRead(buttonpin) != buttonState) {
      Serial.println("button state pressed in pattern one ");
    }
    delay(200);//havint nitinol off for x amount of seconds

  }

}
////////////////////////////////////////////////////////
///////////PatternTWO/////////////////
////////////////////////////////////////////////////////
void patternTWO() {
  buttonState = 0;
  unsigned int chip1 = 0;
  unsigned int chip2 = 0;

  for (int i = 0; i < 3; ++i) {

    ///////////////Turning nitinol on///////////////

    digitalWrite(css2, LOW);//turns on the communication line to send the spi command to be sent
    chip2 = SPI.transfer16(column_pattern2[i]);
    Serial.print("chip2 column on high driver= ");//PLEASE NOTE: all these print statements are frame behinds so the print statment doesn't match what it's actually doing
    Serial.println(chip2, BIN);//printing binary value
    digitalWrite(css2, HIGH);//turns off the communication line to send the spi command to be sent

    // turning row 4 and 6 to be a low driver
    digitalWrite(css1, LOW);
    chip1 = SPI.transfer16(0b0001010000000000);
    Serial.print("chip1 row 8 on high driver= ");
    Serial.println(chip1, BIN);
    digitalWrite(css1, HIGH);

    delay(1);//delay to so that the chip can process that we are about to turn on a different channel of drivers

    //turning row 8 to be a low driver
    digitalWrite(css1, LOW);
    chip1 = SPI.transfer16(0b0100000100000000);
    Serial.print("chip1 row 4 and 6 on low driver= ");
    Serial.println(chip1, BIN);
    digitalWrite(css1, HIGH);

    delay(3000);//having the nitinol stay on for x amount of seconds

    ///////////////Turning nitinol off///////////////

    //turning row 4 and 6 on high driver
    digitalWrite(css1, LOW);
    chip1 = SPI.transfer16(0b0001010001010000);
    Serial.print("chip1 row 8 on low driver= ");
    Serial.println(chip1, BIN);
    digitalWrite(css1, HIGH);

    delay(1);//delay to so that the chip can process that we are about to turn on a different channel of drivers

    //turning row 8 on high driver
    digitalWrite(css1, LOW);
    chip1 = SPI.transfer16(0b0100000100000100);
    Serial.print("chip1 row 4 and 6 on high driver= ");
    Serial.println(chip1, BIN);
    digitalWrite(css1, HIGH);

    digitalWrite(css2, LOW);
    chip2 = SPI.transfer16(column_pattern2[i]);
    Serial.println(chip2, BIN);
    digitalWrite(css2, HIGH);

    if (digitalRead(buttonpin) != buttonState) {
      Serial.println("button state pressed in pattern 2 ");
    }
    delay(200);//havint nitinol off for x amount of seconds

  }

}

////////////////////////////////////////////////////////
///////////PatternThree/////////////////
////////////////////////////////////////////////////////
void patternTHREE() {
  buttonState = 0;
  unsigned int chip1 = 0;
  unsigned int chip2 = 0;

  for (int i = 0; i < 7; ++i) {

    ///////////////Turning nitinol on///////////////

    digitalWrite(css2, LOW);//turns on the communication line to send the spi command to be sent
    chip2 = SPI.transfer16(column_pattern3[i]);
    Serial.print("chip2 column on high driver= ");//PLEASE NOTE: all these print statements are frame behinds so the print statment doesn't match what it's actually doing
    Serial.println(chip2, BIN);//printing binary value
    digitalWrite(css2, HIGH);//turns off the communication line to send the spi command to be sent

    //fixing some bugs on the driver chip that have happended before so just incase covering our basis 
    //////////////////////////////////////////////////////////////////
    //for some reason when it turns on for column 2 it will turn on column 7; turns on column 1 will turn on column 7; or turn on column 7 it will column1
    //to get rid of this issue I added these if statments to turn off the column that I don't want to be on


    //if column 2 is on turn off column 7
    if (column_pattern3[i] == 0b0000000100000100) {
      delay(1);
      digitalWrite(css2, LOW);
      chip2 = SPI.transfer16(0b0100000000000000);//sending a command to the driver to have column 7 off
      digitalWrite(css2, HIGH);

    }
    // if column 7 is on turn off column 1
    else if (column_pattern3[i] == 0b0100000010000010) {
      delay(1);
      digitalWrite(css2, LOW);
      chip2 = SPI.transfer16(0b000000000000000);//sending a command to the driver to have column 1 off
      digitalWrite(css2, HIGH);
    }

    //if column 1 is on turn off column 7
    else if (column_pattern3[i] == 0b0000000010000010) {
      delay(1);
      digitalWrite(css2, LOW);
      chip2 = SPI.transfer16(0b0100000000000000);
      digitalWrite(css2, HIGH);
    }

    //if column 5 is on turn off column 7
    else if (column_pattern3[i] == 0b0000100000100000) {
      delay(1);
      digitalWrite(css2, LOW);
      chip2 = SPI.transfer16(0b0100000000000000);
      digitalWrite(css2, HIGH);
    }

    //////////////////////////////////////////////////////////////////

    // turning row 4 and 6 to be a low driver
    digitalWrite(css1, LOW);
    chip1 = SPI.transfer16(0b0001010000000000);
    Serial.print("chip1 row 8 on high driver= ");
    Serial.println(chip1, BIN);
    digitalWrite(css1, HIGH);

    delay(1);//delay to so that the chip can process that we are about to turn on a different channel of drivers

    //turning row 8 to be a low driver
    digitalWrite(css1, LOW);
    chip1 = SPI.transfer16(0b0100000100000000);
    Serial.print("chip1 row 4 and 6 on low driver= ");
    Serial.println(chip1, BIN);
    digitalWrite(css1, HIGH);

    delay(4000);//having the nitinol stay on for x amount of seconds

    ///////////////Turning nitinol off///////////////

    //turning row 4 and 6 on high driver
    digitalWrite(css1, LOW);
    chip1 = SPI.transfer16(0b0001010001010000);
    Serial.print("chip1 row 8 on low driver= ");
    Serial.println(chip1, BIN);
    digitalWrite(css1, HIGH);

    delay(1);//delay to so that the chip can process that we are about to turn on a different channel of drivers

    //turning row 8 on high driver
    digitalWrite(css1, LOW);
    chip1 = SPI.transfer16(0b0100000100000100);
    Serial.print("chip1 row 4 and 6 on high driver= ");
    Serial.println(chip1, BIN);
    digitalWrite(css1, HIGH);

    digitalWrite(css2, LOW);
    chip2 = SPI.transfer16(column_pattern3[i]);
    Serial.println(chip2, BIN);
    digitalWrite(css2, HIGH);

    if (digitalRead(buttonpin) != buttonState) {
      Serial.println("button state pressed in pattern 3 ");
    }
    delay(200);//havint nitinol off for x amount of seconds

  }

}

/////Interrupt////
//this interrupt is used more as a flag, as in when I press the button it won't change the state but it will alert an if statement that
//the buttonState has been changed to 1
void button_pressed()
{
  cli();
  //confirmitng that the button was pressed and that its not just like a bounce"
  if (millis() - prevTime > 50) {
    Serial.println("left button pressed");
    buttonState = 1;
    Serial.println("buttonState =");
    Serial.println(buttonState);
  }
  sei();
  prevTime = millis();

}



