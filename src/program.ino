#include <math.h>

// cross variables, not cables
int RS = 15;
int RW = 14;
int ENABLE = 16;
int D7 = 9;
int D6 = 8;
int D5 = 7;
int D4 = 6;
int D3 = 5;
int D2 = 4;
int D1 = 3;
int D0 = 2;
int DBUS[] = {D0,D1,D2,D3,D4,D5,D6,D7};

int Calibutton = 1;
int Offsetbutton = 0;

unsigned long int resistorvalue = 0;
unsigned long int offset = 10; // change this for your defaults
float scale = 1.2; // change this for your defaults

byte space = 0b10100000;

void clearBus() {
  for (int i=0; i<8; i++) {
    digitalWrite(DBUS[i], LOW);
  }
}

void clearAll() {
  digitalWrite(ENABLE, LOW);
  digitalWrite(RW, LOW);
  digitalWrite(RS, LOW);
  clearBus();
}

void cycleEnable() {
  digitalWrite(ENABLE, HIGH);
  delay(2);
  digitalWrite(ENABLE, LOW);
}

void setByte(byte b) {
  for (int i=0; i<8; i++) {
    byte v=b>>i;
    v&=0x01;
    digitalWrite(DBUS[i],v==0x01?HIGH:LOW);
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(Calibutton,INPUT);

  pinMode(ENABLE,OUTPUT);
  pinMode(RW,OUTPUT);
  pinMode(RS,OUTPUT);
  for (int i=0; i<8; i++) {
    pinMode(DBUS[i], OUTPUT);
  }

  clearAll();

  // Display Clear
  digitalWrite(D0,HIGH);
  cycleEnable();
}



void loop() {
  clearAll();

  // Cursor Home
  digitalWrite(D0,HIGH);
  digitalWrite(D1,HIGH);
  cycleEnable();

  // Mode Set
  digitalWrite(D0,HIGH);
  digitalWrite(D1,LOW);
  digitalWrite(D2,HIGH);
  cycleEnable();

  // Display Control
  digitalWrite(D0,LOW);
  digitalWrite(D1,LOW);
  digitalWrite(D2,HIGH);
  digitalWrite(D3,HIGH);
  cycleEnable();

  digitalWrite(RS,HIGH);

  resistorvalue=0; // resistorvalue+91;

  unsigned long int raw = 0;
  int Vin = 5;
  float Vout = 0;
  float R1 = 10000;
  float R2 = 0;
  float buf = 0;
  raw = analogRead(10);
  if(raw){
    buf = raw * Vin;
    Vout = (buf)/1024.0;
    buf = (Vin/Vout) - 1;
    R2 = R1 * buf;
    resistorvalue = ceil(R2);
  }

  if(digitalRead(Offsetbutton)) {
    offset = resistorvalue;
    Serial.print("Offset: ");
    Serial.println(offset);
  }
  resistorvalue=resistorvalue-offset;
  if(resistorvalue<0) {
    resistorvalue=0;
  }

  if(digitalRead(Calibutton)) {
    scale = 10000.0/(float)resistorvalue;
    Serial.print("Scale: ");
    Serial.println(scale);
  }
  resistorvalue=(unsigned long int)round(resistorvalue*scale);

  if(resistorvalue>100000000) {
    resistorvalue=0;
  }

  byte signA=space;
  byte signB=0b11110100; // Ohm
  unsigned long int tempvalue=resistorvalue;

  Serial.println(tempvalue);

  if(resistorvalue>=1000000) {
    tempvalue=floor(resistorvalue/100000);
    signA=0b01001101; // M
    signB=0b11110100; // Ohm
  } else if(resistorvalue>=1000) {
    tempvalue=floor(resistorvalue/100);
    signA=0b01101011; // k
    signB=0b11110100; // Ohm
  } else {
    tempvalue=floor(tempvalue*10);
  }

  setByte(signB);
  cycleEnable();
  setByte(signA);
  cycleEnable();

  tempvalue=floor(tempvalue);
  int temp=(int)tempvalue-(10*floor(tempvalue/10));
  temp=(byte)temp;
  temp=temp|0x30;
  setByte(temp);
  cycleEnable();
  tempvalue=floor(tempvalue/10);

  setByte(0b00101100);
  cycleEnable();

  for(int i=0;i<3;i++) {
    if(tempvalue>0) {
      tempvalue=floor(tempvalue);
      temp=(int)tempvalue-(10*floor(tempvalue/10));
      temp=(byte)temp;
      temp=temp|0x30;
      tempvalue=floor(tempvalue/10);
      setByte(temp);
      cycleEnable();
    } else {
      setByte(space);
      cycleEnable();
    }
  }

  delay(100);
}
