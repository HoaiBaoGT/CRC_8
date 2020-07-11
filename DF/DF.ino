#include <TimerOne.h>
#include <DHT.h>
#include "CRC8_D5.h"
CRC8_D5 crc;

#define calculate_current 4.88
/*
 * R = 200
 * range [0,1023] [0v,5v]
 * rate 1:1000
 * 1 ampere = 1000 miliamperes
 * 1000*1000
 * ---------
 * 1024*200
 */

int static pin_R1 = 8;
int static pin_R2 = 9;
int static pin_R3 = 10;
int static pin_R4 = 11;
int static pin_R5 = 12;

int static pin_DHT1 = 2;
int static pin_DHT2 = 3;
int static pin_DHT3 = 4;
DHT dht_1(pin_DHT1,DHT11);
DHT dht_2(pin_DHT2,DHT11);
DHT dht_3(pin_DHT3,DHT11);

int static pin_I1 = A0;
int static pin_I2 = A1;

bool is_SERVER_SEND = false;
volatile bool is_SEND = false;
//==================================================
#define Message_STR '['
#define Message_MID '|'
#define Message_END ']'
struct message {
  int i;
  int v;
};
message MS[32];
int MS_size = 0;
//--------------------------------------------------
void SendData(int i, int v) {
  unsigned long cache_i = crc.message_generate(i);
  unsigned long cache_v = crc.message_generate(v);
  Serial.print(Message_STR);
  for (int count = 5; count >= 0; count--) {
    byte pick = (cache_i >> (count * 4)) & 0xF;
    Serial.print(pick, HEX);
  }
  Serial.print(Message_MID);
  for (int count = 5; count >= 0; count--) {
    byte pick = (cache_v >> (count * 4)) & 0xF;
    Serial.print(pick, HEX);
  }
  Serial.print(Message_END);
}
//--------------------------------------------------
unsigned long uin_i_value;
unsigned long uin_v_value;
byte uin_i_index;
byte uin_v_index;
boolean uin_is_i = false;
boolean uin_is_v = false;
void ReceiveData() {
  while (Serial.available()) {
    char pick = Serial.read();
    //Serial.print(pick);
    if (pick == '\n') {
      continue;
    }
    if (pick == Message_STR) {
      uin_i_value = 0;
      uin_i_index = 5;
      uin_is_i = true;
      uin_is_v = false;
      continue;
    }
    if (pick == Message_MID) {
      uin_v_value = 0;
      uin_v_index = 5;
      uin_is_i = false;
      uin_is_v = true;
      continue;
    }
    if (pick == Message_END) {
      if (crc.message_check(uin_i_value) && crc.message_check(uin_v_value)) {
        MS[MS_size].i = crc.get_data(uin_i_value);
        MS[MS_size].v = crc.get_data(uin_v_value);
        //SendData(MS[MS_size].i,MS[MS_size].v);
        MS_size++;
      }
      uin_is_i = false;
      uin_is_v = false;
      continue;
    }
    if (uin_is_i && (uin_i_index >= 0)) {
      uin_i_value = uin_i_value | ((unsigned long)strtol(&pick, NULL, 16) << (uin_i_index * 4));
      uin_i_index--;
      continue;
    }
    if (uin_is_v && (uin_v_index >= 0)) {
      uin_v_value = uin_v_value | ((unsigned long)strtol(&pick, NULL, 16) << (uin_v_index * 4));
      uin_v_index--;
      continue;
    }
  }
}
//==================================================
void MS_Handle(){
  while (MS_size > 0) {
    MS_size--;
    message M = MS[MS_size];
    if (M.i == s_to_i("R1")){
      bool v = (M.v == s_to_i("T"));
      control_R(pin_R1,v);
      continue;
    }
    if (M.i == s_to_i("R2")){
      bool v = (M.v == s_to_i("T"));
      control_R(pin_R2,v);
      continue;
    }
    if (M.i == s_to_i("R3")){
      bool v = (M.v == s_to_i("T"));
      control_R(pin_R3,v);
      continue;
    }
    if (M.i == s_to_i("R4")){
      bool v = (M.v == s_to_i("T"));
      control_R(pin_R4,v);
      continue;
    }
    if (M.i == s_to_i("SS")) {
      is_SERVER_SEND = (M.v == s_to_i("T"));
      continue;
    }
  }
}
//==================================================
void setup() {
  Serial.begin(9600);
  
  //pinMode(pin_DHT1, INPUT);
  //pinMode(pin_DHT2, INPUT);
  //pinMode(pin_DHT3, INPUT);
  
  dht_1.begin();
  dht_2.begin();
  dht_3.begin();

  digitalWrite(pin_R1, HIGH);
  digitalWrite(pin_R2, HIGH);
  digitalWrite(pin_R3, HIGH);
  digitalWrite(pin_R4, HIGH);
  digitalWrite(pin_R5, HIGH);
  pinMode(pin_R1, OUTPUT);
  pinMode(pin_R2, OUTPUT);
  pinMode(pin_R3, OUTPUT);
  pinMode(pin_R4, OUTPUT);
  pinMode(pin_R5, OUTPUT);
  
  Timer1.initialize(5000000);
  Timer1.attachInterrupt(On_Timer);
}

void loop() {
  ReceiveData();
  MS_Handle();
  if (is_SEND) {
    //*
    int H1 = measure_H(1);
    int T1 = measure_T(1);
    int H2 = measure_H(2);
    int T2 = measure_T(2);
    int H3 = measure_H(3);
    int T3 = measure_T(3);

    int I1 = measure_I(pin_I1);
    int I2 = measure_I(pin_I2);
    
    SendData(s_to_i("H1"),H1);
    SendData(s_to_i("T1"),T1);
    SendData(s_to_i("H2"),H2);
    SendData(s_to_i("T2"),T2);
    SendData(s_to_i("H3"),H3);
    SendData(s_to_i("T3"),T3);
    
    SendData(s_to_i("I1"),I1);
    SendData(s_to_i("I2"),I2);
    //*/
    is_SEND = false;
  }
}
//==================================================
void On_Timer() {
  if (is_SERVER_SEND) {
    is_SEND = true;
  }
}
void control_R(int pin, bool mode){
  if (mode) {
    digitalWrite(pin, LOW);
    return;
  }
  digitalWrite(pin, HIGH);
}
int measure_I(int pin) {
  int v = 0;
  int v_max = 0;
  uint32_t st = millis();
  while ((millis() - st) < 20) {
    v = analogRead(pin);
    if (v > v_max) {
      v_max = v;
    }
  }
  if (v_max == 0) {
    v_max = 65536;
  }
  return v_max;
}

//Remember. DO NOT READ THIS SHIT DURING INTERRUPT.
int measure_H(int dv) {
  float v = 0;
  switch(dv){
    case 1:
      v = dht_1.readHumidity();
      break;
    case 2:
      v = dht_2.readHumidity();
      break;
    case 3:
      v = dht_3.readHumidity();
      break;
    default:
      break;
  }
  if (!isnan(v)) {
    return (int)v;
  }
  return 65536;
}

int measure_T(int dv) {
  float v = 0;
  switch(dv){
    case 1:
      v = dht_1.readTemperature();
      break;
    case 2:
      v = dht_2.readTemperature();
      break;
    case 3:
      v = dht_3.readTemperature();
      break;
    default:
      break;
  }
  if (!isnan(v)) {
    return (int)v;
  }
  return 65536;
}

int s_to_i(String s){
  int i = 0;
  int l = s.length();
  if (l > 0){
    i = i | s[0];
  }
  if (l > 1){
    i = i | (s[1] << 8);
  }
  return i;
}
