/*THIS WORK WAS MY (OUR) OWN WORK. IT WAS WRITTEN WITHOUT CONSULTING
WORK WRITTEN BY OTHER STUDENTS OR COPIED FROM ONLINE RESOURCES.
Mandy Sun*/

#define F_CPU 16000000L;  // tell code we are 16MHz
#include <avr/delay.h>;

/*-------------------------------Initialize Values-------------------------------*/
void _delay_ms(double msec);
void _delay_us(double usec);

// row pins
const int r1 = 10;
const int r2 = A2;
const int r3 = 11;
const int r4 = A3;
const int r5 = 12;
const int r6 = A1;
const int r7 = A0;
const int r8 = A4;
int rowArr[] = { r1, r2, r3, r4, r5, r6, r7, r8 };

// button input pins
const int rightButton = A7; // must do analogRead due to limitations with arduino (so all input functions accomodate for this analog pin)
const int midButton = A5; 
const int leftButton = 8;
int lInput = HIGH;
int mInput = HIGH;
int rInput = 300;

// initialize speaker + music freq
int song[] = {0,0,392,440, 392, 349, 329, 349, 392,293, 329, 349, 329, 349, 392, 392,440,392, 349, 329, 349, 392, 293, 392, 329, 261};
const int speaker = 9;

// display messages after the game
byte loseMessage[9][8] = {
  {B00000000,B11111111,B11111111,B11000011,B11001011,B11001111,B11001111,B00000000}, // G
  {B00000000,B01111111,B11111111,B11001100,B11001100,B11111111,B01111111,B00000000}, // A
  {B11111111,B11111111,B00110000,B00011000,B00011000,B00110000,B11111111,B11111111}, // M
  {B00000000,B11111111,B11111111,B11011011,B11011011,B11011011,B11000011,B00000000}, // E
  {B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000}, // 
  {B00000000,B01111110,B11111111,B11000011,B11000011,B11111111,B01111110,B00000000}, // O
  {B00000000,B11111100,B11111110,B00000111,B00000111,B11111110,B11111100,B00000000}, // V
  {B00000000,B11111111,B11111111,B11011011,B11011011,B11011011,B11000011,B00000000}, // E
  {B00000000,B11111111,B11111111,B11001100,B11001110,B11111011,B01110001,B00000000}  // R
};

byte winMessage[4][8]{
  {B11111100,B11111111,B00001111,B00111100,B00111100,B00001111,B11111111,B11111100}, // W
  {B00000000,B11000011,B11000011,B11111111,B11111111,B11000011,B11000011,B00000000}, // I
  {B11111111,B11111111,B01110000,B00111000,B00011100,B00001110,B11111111,B11111111}, // N
  {B00111100,B01000010,B10101001,B10000101,B10000101,B10101001,B01000010,B00111100}  // :)
};

/* -------------------------------------------------------------- */

void setup() { // setup all variables:
  //PORTD set as output
  DDRD = 0xFF;  

  // initialize input output of speaker + buttons
  pinMode(leftButton, INPUT_PULLUP); pinMode(rightButton, INPUT); pinMode(midButton, INPUT_PULLUP);
  pinMode(speaker, OUTPUT);

  // initialize all rows as output and on
  for (int x = 0; x < 8; x++) {
    pinMode(rowArr[x], OUTPUT); digitalWrite(rowArr[x], LOW);
  }

  // set random seed to unused pin to generate random tiles
  randomSeed(analogRead(A6));
}

void loop() {

  // -----Buffer scene before the game starts (user must hold any button to start the game)----- // 
  for (int x = 0; x < 8; x++) {
    digitalWrite(rowArr[x], LOW);
  }

  while(lInput == HIGH & mInput == HIGH & rInput > 20){
    lInput = digitalRead(leftButton); mInput = digitalRead(midButton);rInput = analogRead(rightButton);
    PORTD = B00000001;
    delay(400);
    PORTD = B00000000;
    delay(200);
  }

  // ------------------------------------------------------------------------------------------ //

  // Start the game
  int timems = 270;     // starting scroll speed
  bool lose = false;    // win state

  // Plays 3 rounds, increase scroll speed by 85ms each time until reach 100ms
  // The goal of the game is to click the buttons corresponding to falling tiles on the beat of the given song
  // There are no penalties for pressing incorrect buttons, you lose if you miss a tile
  // (Past 100ms the game becomes nearly impossible to play)

  while(lose == false and timems >= 100){
    lose = playGame(timems);
    noTone(speaker);
    delay(250);
    timems -= 85;
  }

  // Display appropriate messages for winning/losing

  if (lose == true){ // if you missed a tile
    for(int x = 0; x < 9; x++){
      display(loseMessage[x], 285);
    }
  } else{ // if you missed no tiles
    for(int x = 0; x < 3; x++){
      display(winMessage[x], 285);
    }
    display(winMessage[3], 3000);
  }

  delay(2000);
}


// Clear screen function to reset all rows to off/HIGH
void clearScreen() {
  for (int i = 0; i < 8; i++) {  // set all col and rows off
    digitalWrite(rowArr[i], HIGH);
  }
}

// Display a given letter pattern function
void display(byte letterPattern[], int timems) {
  clearScreen();

  float starttime = millis();
  float endtime = starttime;
  while ((endtime - starttime) <= timems) {
    for (int x = 0; x < 8; x++) {
      PORTD = letterPattern[x];
      digitalWrite(rowArr[x], LOW); _delay_us(10); digitalWrite(rowArr[x], HIGH); 
    };
    endtime = millis();
  };
}

// Game function, returns win/lose state
bool playGame(int timems) {
  clearScreen();

  // Set up score related variables
  bool lose = false;

  // Randomly generate tiles, for 24 notes (first "merge" array is the first set of tiles, second "merge2" array is for the additional set of tiles)
  // 0 = left tile, 1 = middle tile, 2 = right tile
  int merge[26];
  int mergeIndex = 0;
  for(int rand = 0; rand < 26; rand++){
    merge[rand] = random(3);
  }

  int merge2[26];
  for(int rand = 0; rand < 26; rand++){
    if(rand % 7 != 0){ // only generate duplicate tiles for every 7 tiles (since the switch mobility isn't robust enough for multiple double tiles)
      merge2[rand] = -1;
    } else {
      merge2[rand] = random(3);
    }
  }

  // Create 2 additional "empty" buffer notes so that the song syncs up with the blocks falling
  merge[24] = -1;   merge[25] = -1;

  // Initialize empty screen array
  byte screen[8] = {
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000
  };

  // ------------Game round begins:------------ //
  // Offset each row by bit manipulation and add corresponding tiles as they appear in the array
  // 78 loops to properly offset/scroll enough to show all tiles for the entire song

  for (int offset = 78; offset > 0; offset--) {
    if(offset % 3 == 0){
      // play new note every 3 loops
      tone(speaker, song[mergeIndex]);

      // check every merge array for each value to add into the screen array
      // (since there can be 2 simultaneous tiles)
      if(merge[mergeIndex] == 0 | merge2[mergeIndex] == 0){
        screen[0] |= B11000000;
        screen[1] |= B11000000;
      } 
      
      if (merge[mergeIndex] == 1 | merge2[mergeIndex] == 1){
        screen[3] |= B11000000;
        screen[4] |= B11000000;
      }
      
      if (merge[mergeIndex] == 2 | merge2[mergeIndex] == 2){
        screen[6] |= B11000000;
        screen[7] |= B11000000;
      }
      mergeIndex++;
    } 

    float starttime = millis();
    float endtime = starttime;

    // Modified display function for the tiles
    while ((endtime - starttime) <= timems) {

      // Get button input
      lInput = digitalRead(leftButton); mInput = digitalRead(midButton);rInput = analogRead(rightButton);

      // Quickly go through each row in the given frame to display properly
      for (int x = 0; x < 8; x++) {

        // Check all potential input values to remove a tile from the screen array 
        if(lInput == LOW){
          byte val = screen[0] & B00000011; 
          if(merge[mergeIndex-2] == 0 | merge2[mergeIndex-2] == 0){
            if((val == B00000011) or (val == B00000010) or (val == B00000001)){ // if the last two bits contain a tile and the button input corresponds with the merge array data, remove the tiles from the screen
              screen[0] &= B11111000;
              screen[1] &= B11111000;
            }
          }          
        } 
        
        if(mInput == LOW){
          byte val = screen[3] & B00000011;
          if(merge[mergeIndex-2] == 1 | merge2[mergeIndex-2] == 1){
            if((val == B00000011) or (val == B00000010) or (val == B00000001)){ // if the last two bits contain a tile and the button input corresponds with the merge array data, remove the tiles from the screen
              screen[3] &= B11111000;
              screen[4] &= B11111000;
            }
          }
        }
        
        if(rInput < 20){
          byte val = screen[6] & B00000011;
          if(merge[mergeIndex-2] == 2 | merge2[mergeIndex-2] == 2){
            if((val == B00000011) or (val == B00000010) or (val == B00000001)){ // if the last two bits contain a tile and the button input corresponds with the merge array data, remove the tiles from the screen
              screen[6] &= B11111000;
              screen[7] &= B11111000;
            }
          }
        } 

        // set portd to the given screen byte
        PORTD = screen[x];

        digitalWrite(rowArr[x], LOW); _delay_us(5); digitalWrite(rowArr[x], HIGH);         
      };
      endtime = millis();
    };
    _delay_us(5);

    // check if any tiles were missed (if not, bitwise shift and continue with the loop, if so, return lose = true and exit from game)
    for (int x = 0; x < 8; x++){
      if(screen[x] & B00000011 == B00000011){
        lose = true;
        return lose;
      }
      screen[x] = screen[x] >> 1;
    }
  }

  return lose;
}

