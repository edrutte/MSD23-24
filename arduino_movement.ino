// defines pins numbers
const byte stepPinRmotor = 5;
const byte dirPin_Rmotor = 4;
const byte stepPinLmotor = 3;
const byte dirPin_Lmotor = 2;
const byte sleepPin = 8;
const byte resetPin = 0;

//newly added from my code
//defines pins numbers for each limit switch
const byte limitSwitchLeft = 6;
const byte limitSwitchDown = 7;
const byte electromagFET = 10;

const byte ledPin = 13;

#define led_pin (12)


//old onesqr
//const float numSteps = 262.5;    // 1 second worth of motor rotation for one square
//full x is 2250
//full y is 1842
// one y is 263.142
// one y for each grave moving up/down is 131.57143
//37 from bottom to center
const float gravesteps = 131.57143;
const float numSteps = 252;  //252? but center to center is 2250 so 250 idk if is good
const int timeSpeed = 550;  // how slow the motor spins; time in ms

// declaring/initializing coordinate array for each motor

typedef struct {
  signed char x;
  signed char y;
} Square;

typedef struct {
  signed char dirmotorLx;
  signed char dirmotorRx;
  signed char dirmotorLy;
  signed char dirmotorRy;
  signed char xcoord;
  signed char ycoord;
  signed char x1coord;
  signed char y1coord;
  signed char x1cordhold;
  signed char y1cordhold;
} Initialize;

typedef struct {
  unsigned char occupadoQ;
  unsigned char occupadoB;
  unsigned char occupadoN;
  unsigned char occupadoR;
  unsigned char occupadoP;
  signed char xcoord;
  signed char ycoord;
} Grave;

Grave grave = { 0, 0, 0, 0, 0, 0, 0 };

void setup() {

  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin , LOW);
  Serial.begin(9600);

  initState();

  Square sq = { 0, 0 };    //pickup coords
  Square sq2 = { 0, 0 };    //drop off coords
  Square sqG = { -1, -1 };  //capture coords
  char type = 'j';  //change to the piece name
  bool castling = false; 

  delay(500);

  /*make_move(sq, sq2, sqG, type, castling);*/

  //make_move({0,1}, {0, 3}, {-1, -1}, 'j', false);   //test

  //digitalWrite(electromagFET, LOW);

  /*make_move({4,4}, {6, 3}, {-1, -1}, 'j', false);   //test

  digitalWrite(electromagFET, LOW);

  make_move({4,4}, {6, 3}, {-1, -1}, 'j', false);   //test

  digitalWrite(electromagFET, LOW);

  make_move({4,4}, {6, 3}, {-1, -1}, 'j', false);   //test

  digitalWrite(electromagFET, LOW);

  make_move({4,4}, {6, 3}, {-1, -1}, 'j', false);   //test

  digitalWrite(electromagFET, LOW);

  make_move({4,4}, {6, 3}, {-1, -1}, 'j', false);   //test

  digitalWrite(electromagFET, LOW);

  make_move({4,4}, {6, 3}, {-1, -1}, 'j', false);   //test

  digitalWrite(electromagFET, LOW);*/

}



void loop() {
  if (Serial.available() > 0) {
    String receivedString = Serial.readStringUntil('\n');
    digitalWrite(led_pin, HIGH);
    
    Square from = {receivedString.charAt(0) - '0', receivedString.charAt(1) - '0'};
    Square to = {receivedString.charAt(2) - '0', receivedString.charAt(3) - '0'};
    Square capt = {receivedString.charAt(4) - '0', receivedString.charAt(5) - '0'};
    capt.x = (capt.x == 9) ? -1 : capt.x;
    capt.y = (capt.y == 9) ? -1 : capt.y;
    char captType = receivedString.charAt(6);
    bool castling = (receivedString.charAt(7) - '0') ? true : false;
    make_move(from, to, capt, captType, castling);
  }
}

void initState(){
    //change these to set coordinates for pickup and dropoff

  // set values of all pins
  pinMode(stepPinRmotor, OUTPUT);
  pinMode(dirPin_Rmotor, OUTPUT);
  pinMode(stepPinLmotor, OUTPUT);
  pinMode(dirPin_Lmotor, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(sleepPin, OUTPUT);

  pinMode(limitSwitchLeft, INPUT);
  pinMode(limitSwitchDown, INPUT);
  pinMode(electromagFET, OUTPUT);

  digitalWrite(dirPin_Rmotor, HIGH);
  digitalWrite(dirPin_Lmotor, HIGH);
  digitalWrite(sleepPin, HIGH);
  digitalWrite(resetPin, HIGH);
  digitalWrite(electromagFET, LOW);


  //move to origin shown below 0,0
  //first it's set to move to the bottom left corner and
  //then adjusting to the middle of the bottom left square
  //do we even need delays? we can change delays to make it have a better feel



  //this is for testing purposes remove commments to test
  //while (1)
  //{
  //  digitalWrite(dirPin_Rmotor, HIGH);  // DIR1 = 4 DIR2 = 2  //up
  //  digitalWrite(dirPin_Lmotor, LOW);
  //  rotateMotor(0);  //set this number after measuring
  //  delay(10);

  //  digitalWrite(dirPin_Rmotor, HIGH);   //move right - half a square
  //  digitalWrite(dirPin_Lmotor, HIGH);
  //  rotateMotor(0);  //set this number after measuring
  //  delay(10);
  //}





    while (digitalRead(limitSwitchDown) == HIGH) {
    digitalWrite(dirPin_Rmotor, LOW);  // DIR1 = 4(right motor) DIR2 = 2 (left motor)  down
    digitalWrite(dirPin_Lmotor, HIGH);
    rotateMotor(1);
  }

  digitalWrite(sleepPin, LOW);
  delay(100);
  digitalWrite(sleepPin, HIGH);


  while (digitalRead(limitSwitchLeft) == HIGH) {
    digitalWrite(dirPin_Rmotor, HIGH);
    digitalWrite(dirPin_Lmotor, HIGH);  // right
    rotateMotor(1);
  }

  digitalWrite(dirPin_Rmotor, HIGH);  // DIR1 = 4(right motor) DIR2 = 2 (left motor)  up
  digitalWrite(dirPin_Lmotor, LOW);
  rotateMotor(37);

  digitalWrite(dirPin_Rmotor, LOW);
  digitalWrite(dirPin_Lmotor, LOW);
  rotateMotorL((2) * numSteps * 4);        //diagonal left
  rotateMotorR((2/3) * numSteps * 4);
  delay(10);


  digitalWrite(sleepPin, LOW);

}

void make_move(Square sq, Square sq2, Square sqG, char type, bool castling){
  digitalWrite(sleepPin, HIGH);
  Square sqn = { 0, 0 };   //needs to keep holding its changed value at end loop
  Square hold = { 0, 0 };  //needs to keep holding its changed value at end loop

  bool resetboard = false;

  Initialize m = { HIGH, HIGH, HIGH, HIGH, 0, 0, 0, 0, 0, 0 };

  m.xcoord = (sq.x - 4);  // so it knows if it needs to go left or right (negative = left) to reach pickup its current spot
  m.ycoord = (sq.y - 4);  // so it knows if it needs to go up or down (negative = down) to reach pickup from its current spot

  Square holdold = { hold.x, hold.y };

  if ((sq.x < 8 && sq.y < 8) && (sq.x != sq2.x || sq.y != sq2.y))  //so it does not continue loop until a new value is given
  {
    sqn = { sq.x, sq.y };
    hold = { sq2.x, sq2.y };  // so it knows its location after the end of the loop to prepare for the next loop

    if (sqG.x > -1) {
      grave = restinPieces(sq, sq2, type, m, holdold, sqn, sqG, grave);
      m.xcoord = grave.xcoord;
      m.ycoord = grave.ycoord;
    }


    m = initializepickup(m, sq, sq2);
    pickup(m, sq, sq2);  //movement to go to designated pickup spot from whatever previous spot it was at
    delay(100);        //remove this delay  just for testing

    if (castling == true){
      castle (castling, m, sq);
    }
    else
    {
      m = initializedrop(m);
      newdrop(m, sq);  //movement to go to designated drop off spot from pickup spot
      delay(100);       //remove this delay  just for testing
    }

    digitalWrite(electromagFET, LOW);


    movecenter (sq, sq2, hold, m, grave);
  }

  if (resetboard == true){
    reset ();
  }
  digitalWrite(sleepPin, LOW);
}

void rotateMotor(int steps) {
  for (int x = 0; x < steps; x++) {
    digitalWrite(stepPinRmotor, HIGH);
    digitalWrite(stepPinLmotor, HIGH);
    digitalWrite(ledPin, HIGH);
    delayMicroseconds(timeSpeed);
    digitalWrite(stepPinRmotor, LOW);
    digitalWrite(stepPinLmotor, LOW);
    digitalWrite(ledPin, LOW);
    delayMicroseconds(timeSpeed);
  }
}

void MoveDiag(Initialize m)  //moves diagonal in 4 directions
{
  if (m.x1cordhold == m.y1cordhold)  //topright, bottom left
  {
    digitalWrite(dirPin_Rmotor, m.dirmotorRx);
    digitalWrite(dirPin_Lmotor, m.dirmotorRx);
    rotateMotorL((2/3) * numSteps * m.x1coord);
    rotateMotorR((2) * numSteps * m.y1coord);
    delay(10);
  } else if (m.x1cordhold == (m.y1cordhold * -1))  //topleft, bottom right
  {
    digitalWrite(dirPin_Rmotor, m.dirmotorRx);
    digitalWrite(dirPin_Lmotor, m.dirmotorRx);
    rotateMotorL((2) * numSteps * m.x1coord);
    rotateMotorR((2/3) * numSteps * m.y1coord);
    delay(10);
  }
}

void rotateMotorL(int steps)  //prepare left motor for diagonal
{
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPinLmotor, HIGH);
    digitalWrite(ledPin, HIGH);
    delayMicroseconds(timeSpeed);
    digitalWrite(stepPinLmotor, LOW);
    digitalWrite(ledPin, LOW);
    delayMicroseconds(timeSpeed);
  }
}

void rotateMotorR(int steps)  //initialize right motor for diagonal
{

  for (int i = 0; i < steps; i++) {     // positive y 2 * steps, negative y 2/3
    digitalWrite(stepPinRmotor, HIGH);  //moves it diagonal
    digitalWrite(ledPin, HIGH);
    delayMicroseconds(timeSpeed);
    digitalWrite(stepPinRmotor, LOW);
    digitalWrite(ledPin, LOW);
    delayMicroseconds(timeSpeed);
  }
}



Grave restinPieces(Square sq, Square sq2, char type, Initialize m, Square holdold, Square sqn, Square sqG, Grave grave)  //initializes graveyard for each piece
{

  Initialize n = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  if (type == 'q') {
    grave.occupadoQ++;  //lets us know if the space is occupied
  } else if (type == 'b') {
    grave.occupadoB++;  //lets us know if the space is occupied
  } else if (type == 'n') {
    grave.occupadoN++;  //lets us know if the space is occupied
  } else if (type == 'r') {
    grave.occupadoR++;  //lets us know if the space is occupied
  } else if (type == 'p') {
    grave.occupadoP++;  //lets us know if the space is occupied
  }


  sq.x = sq2.x;  //previously x1 value
  sq.y = sq2.y;
  sq2.x = 8;  //new grave value
  sq2.y = 7;

  m.xcoord = sqG.x - 4;  // so it knows if it needs to go left or right (negative = left) to reach pickup its current spot
  m.ycoord = sqG.y - 4;  // new x minus the old xhold from before
  n = m;
  
  n = initializepickup(n, sqG, sq2);
  pickup(n, sqG, sq2);  //not sure

  

  dropoff(type, n, sqG, sq2, grave);


  /*x1 = holdx;  //to set it back to original values
  y1 = holdy;
  sq.x = sqn.x;  //prob dont need
  sq.y = sqn.y;*/

  grave.xcoord = sqn.x - 8;  // so it knows if it needs to go left or right (negative = left) to reach pickup its current spot
  grave.ycoord = sqn.y - 7;  // old x and old y minus new x and new y (graveyard x and y) so it knows how to get to pickup from grave
  return grave;
}

Initialize initializepickup(Initialize m, Square sq, Square sq2) {  //initialize pickup pins for x and y movement

  m.x1coord = sq2.x - sq.x;                                         // so it knows if it needs to go left or right (negative = left) to reach dropoff from pickup
  m.y1coord = sq2.y - sq.y;                                         // so it knows if it needs to go up or down (negative = down) to reach dropoff from pickup
  m.x1cordhold = m.x1coord;
  m.y1cordhold = m.y1coord;
  // digitalWrite(sleepPin, HIGH);
  //where to pick piece up

  if (m.xcoord < 0)  //x val right/left
  {
    m.dirmotorRx = LOW;  //left
    m.dirmotorLx = LOW;

    m.xcoord = m.xcoord * -1;
  } else {
    m.dirmotorRx = HIGH;
    m.dirmotorLx = HIGH;  //right
  }

  if (m.ycoord < 0)  //y val up/down
  {
    m.dirmotorRy = LOW;
    m.dirmotorLy = HIGH;  //down

    m.ycoord = m.ycoord * -1;
  } else {
    m.dirmotorRy = HIGH;
    m.dirmotorLy = LOW;  //up
  }
  return m;
}
//added from my code
void pickup(Initialize m, Square sq, Square sq2)  //movement to pick up piece, keeping in mind the previous loop dropoff x1
{

  digitalWrite(dirPin_Rmotor, m.dirmotorRx);  //move in calculated x direction
  digitalWrite(dirPin_Lmotor, m.dirmotorLx);
  rotateMotor(numSteps * m.xcoord);
  delay(10);

  digitalWrite(dirPin_Rmotor, m.dirmotorRy);  //move in calculated y direction
  digitalWrite(dirPin_Lmotor, m.dirmotorLy);
  rotateMotor(numSteps * m.ycoord);
  delay(10);


  //turn on electromagnet
  digitalWrite(electromagFET, HIGH);

  delay(1000);
}

void newdrop(Initialize m, Square sq)  //mew dropps for all drop off situations
{

  /*gravex = 0;
  gravex = 0;     //was gonna use this to hold values 
  gravex1 = 0;    //looks like might not need
  gravey1 = 0;*/

  if (m.x1coord == m.y1coord) {
    MoveDiag(m);
  } else if (m.x1coord == 0) {
    digitalWrite(dirPin_Rmotor, m.dirmotorRy);  //move in calculated y direction
    digitalWrite(dirPin_Lmotor, m.dirmotorLy);
    rotateMotor(numSteps * m.y1coord);

  } else if (m.y1coord == 0) {

    digitalWrite(dirPin_Rmotor, m.dirmotorRx);  //move in calculated x direction - half a square
    digitalWrite(dirPin_Lmotor, m.dirmotorLx);
    rotateMotor(numSteps * m.x1coord);
    

  } 
  else if (m.x1coord != m.y1coord && m.x1coord != 0 && m.y1coord != 0) {
    if (sq.y != 0) {
      digitalWrite(dirPin_Rmotor, LOW);  //   down
      digitalWrite(dirPin_Lmotor, HIGH);
      rotateMotor((0.5) * numSteps);
      delay(10);
    } 
    else {
      digitalWrite(dirPin_Rmotor, HIGH);  // up
      digitalWrite(dirPin_Lmotor, LOW);
      rotateMotor((0.5) * numSteps);
      delay(10);
    }

    digitalWrite(dirPin_Rmotor, m.dirmotorRx);  //move in calculated x direction - half a square
    digitalWrite(dirPin_Lmotor, m.dirmotorLx);
    rotateMotor((numSteps * m.x1coord) - (0.5) * numSteps);
    delay(10);

    digitalWrite(dirPin_Rmotor, m.dirmotorRy);  //move in calculated y direction
    digitalWrite(dirPin_Lmotor, m.dirmotorLy);
    rotateMotor(numSteps * m.y1coord);
    delay(10);

    digitalWrite(dirPin_Rmotor, m.dirmotorRx);
    digitalWrite(dirPin_Lmotor, m.dirmotorLx);  //move the rest of the x direction
    rotateMotor((0.5) * numSteps);
    delay(10);

    if (sq.y != 0) {
      digitalWrite(dirPin_Rmotor, HIGH);  // up
      digitalWrite(dirPin_Lmotor, LOW);
      rotateMotor((0.5) * numSteps);
      delay(10);
    } else {
      digitalWrite(dirPin_Rmotor, LOW);  //   down
      digitalWrite(dirPin_Lmotor, HIGH);
      rotateMotor((0.5) * numSteps);
    }
  }
  delay(1000);
  digitalWrite(electromagFET, LOW);
}


void castle(bool castling, Initialize m, Square sq)  // do_it asks if we want to do the castle  type asks for the piece name
{
  m = initializedrop(m);
  digitalWrite(electromagFET, LOW);


  digitalWrite(dirPin_Rmotor, m.dirmotorRx);  
  digitalWrite(dirPin_Lmotor, m.dirmotorLx); //to pick up rook
  rotateMotor(numSteps * (m.x1coord + 1));

  digitalWrite(electromagFET, HIGH);
  delay(1000);

  digitalWrite(dirPin_Rmotor, !m.dirmotorRx);  //drop off rook
  digitalWrite(dirPin_Lmotor, !m.dirmotorLx);
  rotateMotor(numSteps * 2);

  delay(1000);
  digitalWrite(electromagFET, LOW);
  

  //king movement now
  digitalWrite(dirPin_Rmotor, !m.dirmotorRx);  
  digitalWrite(dirPin_Lmotor, !m.dirmotorLx); //to pick up king
  rotateMotor(numSteps * (m.x1coord - 1));

  digitalWrite(electromagFET, HIGH);
  delay(1000);

  digitalWrite(dirPin_Rmotor, LOW);  // down
  digitalWrite(dirPin_Lmotor, HIGH);
  rotateMotor((0.5) * numSteps);
  delay(10);

  digitalWrite(dirPin_Rmotor, m.dirmotorRx);  //move in calculated x direction - half a square
  digitalWrite(dirPin_Lmotor, m.dirmotorLx);
  rotateMotor(numSteps * m.x1coord);

  digitalWrite(dirPin_Rmotor, HIGH);  // up
  digitalWrite(dirPin_Lmotor, LOW);
  rotateMotor((0.5) * numSteps);
  delay(10);

  delay(1000);
  digitalWrite(electromagFET, LOW);

}

Initialize initializedrop(Initialize m) {  //initialize drop values
                                           /*int ydir = m.y1coord;
  int xdir = m.x1coord;*/


  //where to drop piece off

  if (m.x1coord < 0)  //x val right/left
  {
    m.dirmotorRx = LOW;  //left
    m.dirmotorLx = LOW;
    m.x1coord = m.x1coord * -1;
  } else {
    m.dirmotorRx = HIGH;
    m.dirmotorLx = HIGH;  //right
  }


  if (m.y1coord < 0)  //y val up/down
  {
    m.dirmotorRy = LOW;
    m.dirmotorLy = HIGH;  //down
    m.y1coord = m.y1coord * -1;
  } else {
    m.dirmotorRy = HIGH;
    m.dirmotorLy = LOW;  //up
  }
  return m;
}


void dropoff(char type, Initialize m, Square sq, Square sqG, Grave grave)  //pretty much just for grave movement now
{
  m = initializedrop(m);
  if (sqG.x > -1) {
    if (sq.y != 0) {
      digitalWrite(dirPin_Rmotor, LOW);  //   down
      digitalWrite(dirPin_Lmotor, HIGH);
      rotateMotor((0.5) * numSteps);
      delay(10);
    } else {
      digitalWrite(dirPin_Rmotor, HIGH);  // up
      digitalWrite(dirPin_Lmotor, LOW);
      rotateMotor((0.5) * numSteps);
      delay(10);
    }

    if (m.x1coord != 0) {

      digitalWrite(dirPin_Rmotor, m.dirmotorRx);  //move in calculated x direction - half a square
      digitalWrite(dirPin_Lmotor, m.dirmotorLx);
      rotateMotor((numSteps * m.x1coord) - (0.5) * numSteps);

      if (m.y1coord != 0) {
        delay(10);
      }

    } else {

      digitalWrite(dirPin_Rmotor, HIGH);  //move right - half a square
      digitalWrite(dirPin_Lmotor, HIGH);
      rotateMotor((0.5) * numSteps);
    }

    digitalWrite(dirPin_Rmotor, m.dirmotorRy);  //move in calculated y direction
    digitalWrite(dirPin_Lmotor, m.dirmotorLy);
    rotateMotor(numSteps * m.y1coord);

    if (m.y1coord != 0) {
      delay(10);
    }

    if (m.x1coord != 0) {

      if (sqG.x > -1) {
        delay(10);
      } else {
        digitalWrite(dirPin_Rmotor, m.dirmotorRx);
        digitalWrite(dirPin_Lmotor, m.dirmotorLx);  //move the rest of the x direction
        rotateMotor((0.5) * numSteps);
        delay(10);
      }

    } else {

      digitalWrite(dirPin_Rmotor, LOW);  //move left - half a square
      digitalWrite(dirPin_Lmotor, LOW);
      rotateMotor((0.5) * numSteps);
    }

    if (sq.y != 0) {
      digitalWrite(dirPin_Rmotor, HIGH);  // up
      digitalWrite(dirPin_Lmotor, LOW);
      rotateMotor((0.5) * numSteps);
      delay(10);
    } else {
      digitalWrite(dirPin_Rmotor, LOW);  //   down
      digitalWrite(dirPin_Lmotor, HIGH);
      rotateMotor((0.5) * numSteps);
      delay(100);
    }

    if (sqG.x > -1) {
      digitalWrite(dirPin_Rmotor, HIGH);  // DIR1 = 4 DIR2 = 2  //up
      digitalWrite(dirPin_Lmotor, LOW);
      rotateMotor(37);  //set this number after measuring

      //add stuff here once on da board
      if (type == 'q') {
        digitalWrite(dirPin_Rmotor, HIGH);  //move right - half a square
        digitalWrite(dirPin_Lmotor, HIGH);
        rotateMotor(0.5*numSteps);  //set this number after measuring
        delay(10);

        delay(1000);
        digitalWrite(electromagFET, LOW);


      } 
      else if (type == 'b') {
        if (grave.occupadoB == 1) {
          delay(1000);
          digitalWrite(dirPin_Rmotor, LOW);  // down
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor(gravesteps);  //set this number after measuring
          delay(10);

          digitalWrite(dirPin_Rmotor, HIGH);  //move right - half a square
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor(0.5*numSteps);  //set this number after measuring
          delay(10);

          delay(1000);
          digitalWrite(electromagFET, LOW);

          digitalWrite(dirPin_Rmotor, HIGH);  // down
          digitalWrite(dirPin_Lmotor, LOW);
          rotateMotor(1 * gravesteps);  //set this number after measuring
          delay(10);

        } else {
          delay(1000);
          digitalWrite(dirPin_Rmotor, LOW);  // down
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor(2 * gravesteps);  //set this number after measuring
          delay(10);

          digitalWrite(dirPin_Rmotor, HIGH);  //move right - half a square
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor(0.5*numSteps);  //set this number after measuring
          delay(10);

          delay(1000);
          digitalWrite(electromagFET, LOW);

          digitalWrite(dirPin_Rmotor, HIGH);  // down
          digitalWrite(dirPin_Lmotor, LOW);
          rotateMotor(2 * gravesteps);  //set this number after measuring
          delay(10);

        }
      } else if (type == 'n') {
        if (grave.occupadoN == 1) {
          delay(1000);
          digitalWrite(dirPin_Rmotor, LOW);  // down
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor(3 * gravesteps);  //set this number after measuring
          delay(10);

          digitalWrite(dirPin_Rmotor, HIGH);  //move right - half a square
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor(0.5*numSteps);  //set this number after measuring
          delay(10);

          delay(1000);
          digitalWrite(electromagFET, LOW);

          digitalWrite(dirPin_Rmotor, HIGH);  // down
          digitalWrite(dirPin_Lmotor, LOW);
          rotateMotor(3 * gravesteps);  //set this number after measuring
          delay(10);


        } else {
          delay(1000);
          digitalWrite(dirPin_Rmotor, LOW);  // down
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor(4 * gravesteps);  //set this number after measuring
          delay(10);

          digitalWrite(dirPin_Rmotor, HIGH);  //move right - half a square
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor(0.5*numSteps);  //set this number after measuring
          delay(10);

          delay(1000);
          digitalWrite(electromagFET, LOW);

          digitalWrite(dirPin_Rmotor, HIGH);  // down
          digitalWrite(dirPin_Lmotor, LOW);
          rotateMotor(4 * gravesteps);  //set this number after measuring
          delay(10);


        }
      } else if (type == 'r') {
        if (grave.occupadoR == 1) {
          delay(1000);
          digitalWrite(dirPin_Rmotor, LOW);  // down
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor(5 * gravesteps);  //set this number after measuring
          delay(10);

          digitalWrite(dirPin_Rmotor, HIGH);  //move right - half a square
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor(0.5*numSteps);  //set this number after measuring
          delay(10);

          delay(1000);
          digitalWrite(electromagFET, LOW);

          digitalWrite(dirPin_Rmotor, HIGH);  // down
          digitalWrite(dirPin_Lmotor, LOW);
          rotateMotor(5 * gravesteps);  //set this number after measuring
          delay(10);


        } else {
          delay(1000);
          digitalWrite(dirPin_Rmotor, LOW);  // down
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor(6 * gravesteps);  //set this number after measuring
          delay(10);

          digitalWrite(dirPin_Rmotor, HIGH);  //move right - half a square
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor(0.5*numSteps);  //set this number after measuring
          delay(10);

          delay(1000);
          digitalWrite(electromagFET, LOW);

          digitalWrite(dirPin_Rmotor, HIGH);  // down
          digitalWrite(dirPin_Lmotor, LOW);
          rotateMotor(6 * gravesteps);  //set this number after measuring
          delay(10);


        }
      } else if (type == 'p') {
        if (grave.occupadoP == 1) {
          delay(1000);
          digitalWrite(dirPin_Rmotor, LOW);  // down
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor((grave.occupadoP + 6) * gravesteps);  //set this number after measuring
          delay(10);

          digitalWrite(dirPin_Rmotor, HIGH);  //move right - half a square
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor(0.5*numSteps);  //set this number after measuring
          delay(10);

          delay(1000);
          digitalWrite(electromagFET, LOW);

          digitalWrite(dirPin_Rmotor, HIGH);  // down
          digitalWrite(dirPin_Lmotor, LOW);
          rotateMotor(7 * gravesteps);  //set this number after measuring
          delay(10);


        } else if (grave.occupadoP == 2) {
          delay(1000);
          digitalWrite(dirPin_Rmotor, LOW);  // down
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor((grave.occupadoP + 6) * gravesteps);  //set this number after measuring
          delay(10);

          digitalWrite(dirPin_Rmotor, HIGH);  //move right - half a square
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor(0.5*numSteps);  //set this number after measuring
          delay(10);

          delay(1000);
          digitalWrite(electromagFET, LOW);

          digitalWrite(dirPin_Rmotor, HIGH);  // down
          digitalWrite(dirPin_Lmotor, LOW);
          rotateMotor(8 * gravesteps);  //set this number after measuring
          delay(10);


        } else if (grave.occupadoP == 3) {
          delay(1000);
          digitalWrite(dirPin_Rmotor, LOW);  // down
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor((grave.occupadoP + 6) * gravesteps);  //set this number after measuring
          delay(10);

          digitalWrite(dirPin_Rmotor, HIGH);  //move right - half a square
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor(0.5*numSteps);  //set this number after measuring
          delay(10);

          delay(1000);
          digitalWrite(electromagFET, LOW);

          digitalWrite(dirPin_Rmotor, HIGH);  // down
          digitalWrite(dirPin_Lmotor, LOW);
          rotateMotor(9 * gravesteps);  //set this number after measuring
          delay(10);

        } else if (grave.occupadoP == 4) {
          delay(1000);
          digitalWrite(dirPin_Rmotor, LOW);  // down
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor((grave.occupadoP + 6) * gravesteps);  //set this number after measuring
          delay(10);

          digitalWrite(dirPin_Rmotor, HIGH);  //move right - half a square
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor(0.5*numSteps);  //set this number after measuring
          delay(10);

          delay(1000);
          digitalWrite(electromagFET, LOW);

          digitalWrite(dirPin_Rmotor, HIGH);  // down
          digitalWrite(dirPin_Lmotor, LOW);
          rotateMotor(10 * gravesteps);  //set this number after measuring
          delay(10);

        } else if (grave.occupadoP == 5) {
          delay(1000);
          digitalWrite(dirPin_Rmotor, LOW);  // down
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor((grave.occupadoP + 6) * gravesteps);  //set this number after measuring
          delay(10);

          digitalWrite(dirPin_Rmotor, HIGH);  //move right - half a square
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor(0.5*numSteps);  //set this number after measuring
          delay(10);

          delay(1000);
          digitalWrite(electromagFET, LOW);

          digitalWrite(dirPin_Rmotor, HIGH);  // down
          digitalWrite(dirPin_Lmotor, LOW);
          rotateMotor(11 * gravesteps);  //set this number after measuring
          delay(10);

        } else if (grave.occupadoP == 6) {
          delay(1000);
          digitalWrite(dirPin_Rmotor, LOW);  // down
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor((grave.occupadoP + 6) * gravesteps);  //set this number after measuring
          delay(10);

          digitalWrite(dirPin_Rmotor, HIGH);  //move right - half a square
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor(0.5*numSteps);  //set this number after measuring
          delay(10);

          delay(1000);
          digitalWrite(electromagFET, LOW);

          digitalWrite(dirPin_Rmotor, HIGH);  // down
          digitalWrite(dirPin_Lmotor, LOW);
          rotateMotor(12 * gravesteps);  //set this number after measuring
          delay(10);

        } else if (grave.occupadoP == 7) {
          delay(1000);
          digitalWrite(dirPin_Rmotor, LOW);  // down
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor((grave.occupadoP + 6) * gravesteps);  //set this number after measuring
          delay(10);

          digitalWrite(dirPin_Rmotor, HIGH);  //move right - half a square
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor(0.5*numSteps);  //set this number after measuring
          delay(10);

          delay(1000);
          digitalWrite(electromagFET, LOW);

          digitalWrite(dirPin_Rmotor, HIGH);  // down
          digitalWrite(dirPin_Lmotor, LOW);
          rotateMotor(13 * gravesteps);  //set this number after measuring
          delay(10);

        } else {
          delay(1000);
          digitalWrite(dirPin_Rmotor, LOW);  // down
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor(14 * gravesteps);  //set this number after measuring
          delay(10);

          digitalWrite(dirPin_Rmotor, HIGH);  //move right - half a square
          digitalWrite(dirPin_Lmotor, HIGH);
          rotateMotor(0.5*numSteps);  //set this number after measuring
          delay(10);

          delay(1000);
          digitalWrite(electromagFET, LOW);

          digitalWrite(dirPin_Rmotor, HIGH);  // down
          digitalWrite(dirPin_Lmotor, LOW);
          rotateMotor(14 * gravesteps);  //set this number after measuring
          delay(10);
  
        }
      }
    }
  }


  //turn off electromagnet
  digitalWrite(dirPin_Rmotor, LOW);  // down
  digitalWrite(dirPin_Lmotor, HIGH);
  rotateMotor(37);  //set this number after measuring

  delay(1000);
}



//Reset the board
void reset(){
  while (digitalRead(limitSwitchDown) == HIGH) {
    digitalWrite(dirPin_Rmotor, LOW);  // DIR1 = 4(right motor) DIR2 = 2 (left motor)  down
    digitalWrite(dirPin_Lmotor, HIGH);
    rotateMotor(1);
  }

  delay(1000);


  while (digitalRead(limitSwitchLeft) == HIGH) {
    digitalWrite(dirPin_Rmotor, HIGH);
    digitalWrite(dirPin_Lmotor, HIGH);  // right
    rotateMotor(1);
  }

  delay(100);

  //above makes use of limit switches to move it all the way to the bottom right that it can go
  //below moves it to center to initialize grid

  digitalWrite(dirPin_Rmotor, LOW);
  digitalWrite(dirPin_Lmotor, LOW);  // left
  rotateMotor(2250);                 // all the way  to  the other end of the board
  delay(100);


  digitalWrite(dirPin_Rmotor, HIGH);  // DIR1 = 4 DIR2 = 2  //up
  digitalWrite(dirPin_Lmotor, LOW);
  rotateMotor(300);  //we need to figure out after board is there
  delay(100);

  digitalWrite(dirPin_Rmotor, HIGH);
  digitalWrite(dirPin_Lmotor, HIGH);  // right
  rotateMotor(300);                   //we need to figure out after board is there
  delay(1000);

}



void movecenter (Square sq, Square sq2, Square hold, Initialize m, Grave grave) {

  /*Initialize n = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  Square x = {-1, -1};
  char j = 'j';

  /*sq.x = sq2.x;  
  sq.y = sq2.y;
  sq2.x = 4;  
  sq2.y = 4;

  m.xcoord = sq2.x - hold.x;  // so it knows if it needs to go left or right (negative = left) to reach pickup its current spot
  m.ycoord = sq2.y - hold.y;  // new x minus the old xhold from before
  n = m;

  n = initializepickup(m, sq, sq2);
  pickup(n, sq, sq2);  //not sure

  grave.xcoord = 0;  // so it knows if it needs to go left or right (negative = left) to reach pickup its current spot
  grave.ycoord = 0;  // old x and old y minus new x and new y (graveyard x and y) so it knows how to get to pickup from grave
  return grave;

  n = initializedrop(n);
  newdrop(n, sq);*/

  while (digitalRead(limitSwitchDown) == HIGH) {
    digitalWrite(dirPin_Rmotor, LOW);  // DIR1 = 4(right motor) DIR2 = 2 (left motor)  down
    digitalWrite(dirPin_Lmotor, HIGH);
    rotateMotor(1);
  }

  digitalWrite(sleepPin, LOW);
  delay(100);
  digitalWrite(sleepPin, HIGH);


  while (digitalRead(limitSwitchLeft) == HIGH) {
    digitalWrite(dirPin_Rmotor, HIGH);
    digitalWrite(dirPin_Lmotor, HIGH);  // right
    rotateMotor(1);
  }

  digitalWrite(dirPin_Rmotor, HIGH);  // DIR1 = 4(right motor) DIR2 = 2 (left motor)  up
  digitalWrite(dirPin_Lmotor, LOW);
  rotateMotor(37);

  digitalWrite(dirPin_Rmotor, LOW);
  digitalWrite(dirPin_Lmotor, LOW);
  rotateMotorL((2) * numSteps * 4);        //diagonal left
  rotateMotorR((2/3) * numSteps * 4);
  delay(10);


  digitalWrite(sleepPin, LOW);


}


