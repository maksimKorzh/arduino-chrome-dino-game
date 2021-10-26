  /****************************************************************\
   ================================================================

                      Arduino LCD shield Chrome Dino
                        
                                   by
                       
                            Code Monkey King

  ================================================================
  \****************************************************************/

  // LCD driver
  #include <LiquidCrystal.h>
  #include <Keypad.h>

  /****************************************************************\
   ================================================================
                      CIRCUIT (Tinkercad version)
   ================================================================

                LCD RS pin to arduino digital pin 12
                LCD Enable pin to arduino digital pin 11
                LCD D4 pin to arduino digital pin 5
                LCD D5 pin to arduino digital pin 4
                LCD D6 pin to arduino digital pin 3
                LCD D7 pin to arduino digital pin 2
                LCD R/W pin to arduino ground
                LCD VSS pin to arduino ground
                LCD VCC pin to arduino 5V
                PUSH BUTTON terminal 1a to arduino pin 7
                PUSH BUTTON terminal 2a to ground
    
   ================================================================
            CIRCUIT (LCD shield 1602A D1 ROBOT (DF ROBOT))
   ================================================================

                LCD RS pin to arduino digital pin 8
                LCD Enable pin to arduino digital pin 9
                LCD Backlight LED to arduino pin 10
                LCD D4 pin to arduino digital pin 4
                LCD D5 pin to arduino digital pin 5
                LCD D6 pin to arduino digital pin 6
                LCD D7 pin to arduino digital pin 7
                LCD A0  to arduino A0 pin (LCD shield keypad) 
                LCD VIN to arduino VIN pin
                LCD GND to arduino GND pin
                LCD 5V to arduino 5V pin
                LCD RST to arduino RST pin

   ================================================================
  \****************************************************************/

  // Comment out if running with a single button
  #define CMK_HARDWARE
  
  #ifdef CMK_HARDWARE
    // LCD pins
    #define RS 8    // LCD reset pin
    #define En 9    // LCD enable pin
    #define D4 4    // LCD data pin 4
    #define D5 5    // LCD data pin 5
    #define D6 6    // LCD data pin 6
    #define D7 7    // LCD data pin 7    
  #else
    // LCD pins
    #define RS 12   // LCD reset pin
    #define En 11   // LCD enable pin
    #define D4 5    // LCD data pin 4
    #define D5 4    // LCD data pin 5
    #define D6 3    // LCD data pin 6
    #define D7 2    // LCD data pin 7
  #endif

  // init LCD
  LiquidCrystal lcd(RS, En, D4, D5, D6, D7);
  
  // game score
  int score = 0;
  
  // limit jump frequency
  bool allow_jump = false;

  // dino left leg char
  byte dino_l[8] = {
    B00000111,
    B00000101,
    B00000111,
    B00010110,
    B00011111,
    B00011110,
    B00001110,
    B00000100
  };

  // dino right leg char
  byte dino_r[8] = {
    B00000111,
    B00000101,
    B00000111,
    B00010110,
    B00011111,
    B00011110,
    B00001110,
    B00000010
  };

  // small cactus
  byte cactus_small[8] = {
    B00000000,
    B00000000,
    B00000100,
    B00000101,
    B00010101,
    B00010110,
    B00001100,
    B00000100
  };

  // big cactus
  byte cactus_big[8] = {
    B00000000,
    B00000100,
    B00000101,
    B00010101,
    B00010110,
    B00001100,
    B00000100,
    B00000100
  };

  // game world
  char world[] = {
    32, 32, 32, 32, 32, 32, 32, 83, 99, 111, 114, 101, 58, 32, 32, 32,
    32, 0, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
  };

  // infinite run routine
  uint8_t scroll_world() {
    // on the real hardware we need a delay
    #ifdef CMK_HARDWARE
      delay(250);
    #endif
    
    // create next random object on the map (small/big cactus or empty square)
    char random_object = random(2, 35);  // '35' is a probability of encountering a cactus
    
    // place cactus on map
    if (random_object < 4) world[31] = random_object;
    
    // place empty square on map
    else world[31] = 32;
    
    // loop over second row of game world map
    for (int i = 16; i < 32; i++) {
      // scroll only if encounter a cactus
      if (world[i] == 2 or world[i] == 3) {
        // previous cell (cactus or empty)
        char prev = (i < 31) ? world[i + 1] : 32;
        
        // dino and cactus collision detection (game over)
        if (world[i - 1] < 2) return 1;
        
        // scroll world
        world[i - 1] = world[i];
        world[i] = prev;
      }
    }
    
    // erase top right cell on map (otherwise it gets a cactus as a side effect)
    world[15] = 32;
    
    // erase cactus behind dino after jump
    if (world[16] < 2) world[16] = 32;
    
    // no collisions, hence keep running
    return 0;
  }

  // update world
  void update_world() {  
    // scroll map, detect collisions
    int game_over = scroll_world();
    
    // winning condition
    if (score == 999) {
      lcd.setCursor(0, 0);
      lcd.print("    YOU WIN!    ");
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
      lcd.write(byte(32));
      lcd.write(byte(2));
      lcd.write(byte(2));
      lcd.write(byte(2));
      lcd.write(byte(3));
      lcd.write(byte(3));
      lcd.write(byte(3));
      lcd.write(byte(3));
      lcd.write(byte(3));
      lcd.write(byte(3));
      lcd.write(byte(2));
      lcd.write(byte(2));
      lcd.write(byte(2));
      lcd.write(byte(32));
      lcd.write(byte(1));
      while(1);
    }
    
    // hit a cactus
    if (game_over) {
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
      lcd.write(byte(3));
      lcd.print(" GAME OVER! ");
      lcd.write(byte(3));
      lcd.write(byte(0));
      while(1);
    }        
    
    // increase the score
    score++;
    
    // update score
    lcd.setCursor(13, 0);
    lcd.print(score);
    
    // set cursor to top left corner of LCD
    lcd.setCursor(0, 0);
    
    // loop over game world map array
    for (int i = 0; i < 32; i++) {
      // mimic dino stepping left and right legs
      if (world[i] < 2) world[i] ^= 1;
      
      // update cursor for rendering lower row of th LCD
      if (i == 16) lcd.setCursor(0, 1);
      
      // do not overwrite score to avoid flickering
      if (i < 13 || i > 15)
        lcd.write(byte(world[i]));
    }
  }

  // get user input
  bool get_button() {
    // use LCD Shield UP button
    #ifdef CMK_HARDWARE
      int shield_input;
      // 4 of LCD shield buttons are connected to analog pin 0
      shield_input = analogRead (0);
      if (shield_input < 200) return LOW;    // button up
      else return HIGH;
    #else
      return digitalRead(7);
    #endif
  }

  // arduino setup
  void setup() {
    #ifndef CMK_HARDWARE
      // connect button if no LCD shield
      pinMode(7, INPUT_PULLUP);
    #endif
    
    // create sprites, init LCD
    lcd.createChar(0, dino_l);
    lcd.createChar(1, dino_r);
    lcd.createChar(2, cactus_small);
    lcd.createChar(3, cactus_big);
    lcd.begin(16, 2);
  }

  // arduino loop
  void loop() {
    // set cursor to top left corner of LCD
    lcd.setCursor(0, 0);
    
    // wait for user to press a button to start
    lcd.print(" ARE YOU READY? ");
    while(get_button() == HIGH);
    
    // game loop
    while(true) {
      // dissallow dino hanging on the upper row all of the time
      allow_jump ^= 1;
      
      // on button press
      if (get_button() == LOW && allow_jump == true) {
        // update dino position on LCD
        lcd.setCursor(1, 1);
        lcd.write(byte(32));
        lcd.setCursor(1, 0);
        lcd.write(byte(0));
        
        // update dino position on world map
        world[1] = byte(0);
        world[17] = byte(32);
        
        // scroll map while dino is jumping
        for (int i = 0; i < 4; i++) update_world();

        // update dino position on world map
        world[1] = byte(32);
        world[17] = byte(0);
        
        // update dino position om LCD
        lcd.setCursor(1, 0);
        lcd.write(byte(32));
        lcd.setCursor(1, 1);
        lcd.write(byte(0));
      }
      
      // otherwise just scroll the world and update display
      update_world();
    }
  }
   
