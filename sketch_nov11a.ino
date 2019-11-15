#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <GyverButton.h>
#include <EEPROM.h>
#include <Vector.h>

#define SS_PIN A4
#define RST_PIN A5
#define SERVO_PIN 3
#define SERVO_power 4
#define servo_open 1
#define servo_close 179
#define in_button A1
#define tail 12
#define speaker 9 
#define lock_state_diod_open 11
#define lock_state_diod_close 10
#define root_btn 2

//unsigned long keys[7]={20947,31003,26372,22020,4294941849,18180,4294964818};
Vector<long> keys;
bool door =1;
bool fail_key=0;
long uid;

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance.
Servo myservo;

void EEPROM_write(int addr,long val){ // запись в ЕЕПРОМ
    byte *x = (byte *)&val;
    for(byte i = 0; i < 4; i++) {EEPROM.write(addr, x[i]);addr++;}
}

float EEPROM_read(int addr){ // чтение из ЕЕПРОМ
    byte x[4];
    for(byte i = 0; i < 4; i++) {x[i] = EEPROM.read(addr);addr++;}
    long *y = (long *)&x;
    return y[0];
}

long getID(){
  if (! mfrc522.PICC_ReadCardSerial()) { //Since a PICC placed get Serial and continue
    return -1;
  }
  long hex_num;
  hex_num =  mfrc522.uid.uidByte[0] << 24;
  hex_num += mfrc522.uid.uidByte[1] << 16;
  hex_num += mfrc522.uid.uidByte[2] <<  8;
  hex_num += mfrc522.uid.uidByte[3];
  mfrc522.PICC_HaltA(); // Stop reading
  return hex_num;
}

void Error(){
    Serial.println("Error");
    tone(speaker,2000);
    digitalWrite(lock_state_diod_close,LOW);
    delay(500);
    noTone(speaker);
    digitalWrite(lock_state_diod_close,HIGH);
}

void Remove_last_user(){
    int len=(int)EEPROM.read(1)-1;
    Serial.print("Keys left: ");
    if(len<0)len=0;
    Serial.println(len);
    EEPROM.write(1,len);
    tone(speaker,2000,1000);
    delay(100);
    }

void Add_user(){
  bool copy=0;
    tone(speaker,2000,2000);
    uid=getID();
    Serial.print("NEW User: ");
    Serial.println(uid);
    int len=(int)EEPROM.read(1)+1;
    for(int i = 2;i<(len*4)+2;i+=4){
      if(uid == EEPROM_read(i)){
        copy=1;
        Error();
        break;
      }
    }
  if(!copy){
    EEPROM.write(1,len);
    EEPROM_write((len*4)-2,uid);
  }
  delay(300);
}

void Root_mode(){
  tone(speaker,2000,5000);
  Serial.println("Root mode");
  digitalWrite(lock_state_diod_open,LOW);
  digitalWrite(lock_state_diod_close,LOW);
  for(int i = 2;i<(EEPROM.read(1)*4-1);i+=4)
    {
    keys.push_back(EEPROM_read(i));
    }
  Serial.print("Keys are loaded: ");
  Serial.println(EEPROM.read(1));
  for(int i=0;i<keys.size();i++)Serial.println(keys[i]);
  while(1){
    if (mfrc522.PICC_IsNewCardPresent())Add_user();
    if(!digitalRead(in_button)){
      tone(speaker,2000,2000);
      Serial.println("Remove user!!!");
      Remove_last_user();
    }
  }
}

void Change_door(){
  if(door){
    Serial.println("open");
    digitalWrite(lock_state_diod_open,LOW);
    digitalWrite(lock_state_diod_close,HIGH);
    digitalWrite(SERVO_power,HIGH);
    myservo.write(servo_open);
    delay(1000);
    digitalWrite(SERVO_power,LOW);
  }
    else{
      Serial.println("closed");
      digitalWrite(lock_state_diod_open,HIGH);
      digitalWrite(lock_state_diod_close,LOW);
      digitalWrite(SERVO_power,HIGH);
      myservo.write(servo_close);
      delay(1000);
      digitalWrite(SERVO_power,LOW);
      }
    door=!door;
}

void setup() {
  pinMode(root_btn, INPUT_PULLUP);
  pinMode(tail,INPUT_PULLUP);
  pinMode(in_button,INPUT_PULLUP);
  pinMode (lock_state_diod_open, OUTPUT);
  pinMode(lock_state_diod_close,OUTPUT);
  pinMode(SERVO_power,OUTPUT);
  pinMode(SERVO_PIN,OUTPUT);
    pinMode (A7,OUTPUT);
    digitalWrite(A7,LOW);
    pinMode(A3,OUTPUT);
    digitalWrite(A3,LOW);
    pinMode(13,OUTPUT);
    digitalWrite(13,LOW);
    pinMode(A0,OUTPUT);
    digitalWrite(A0,HIGH);
    pinMode(4,OUTPUT);
    digitalWrite(4,LOW);
  Serial.begin(115200);
  SPI.begin();      
  mfrc522.PCD_Init();
  myservo.attach(SERVO_PIN);
  Serial.println("Start");
  if(myservo.read()==servo_open){
    door=1;
  }else{
    door=0;
  }
  if(!digitalRead(root_btn)) Root_mode();
  for(int i = 2;i<(EEPROM.read(1)*4-1);i+=4)
    {
    keys.push_back(EEPROM_read(i));
    }
  Serial.print("Keys are loaded: ");
  Serial.println(EEPROM.read(1));
  for(int i=0;i<keys.size();i++)Serial.println(keys[i]);
  Change_door();
}

void loop(){
  if (mfrc522.PICC_IsNewCardPresent()){
    uid=getID();
    Serial.println(uid);
    for (int i=0;i<keys.size();i++){
      if (uid==keys[i]){
        fail_key=0;
        if(digitalRead(tail)){tone(speaker,2000,200);Change_door();}
        else{tone(speaker,2000,1000);}
        break;    
      }else fail_key=1;  
  }
if(fail_key){
  //tone(speaker,2000,50);
  //delay(100);
  //tone(speaker,2000,50);
  //delay(100);
  //tone(speaker,2000,50);
  Error();
  }
}
if(!digitalRead(in_button)&&digitalRead(tail))Change_door();
}
