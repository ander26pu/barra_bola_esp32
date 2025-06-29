#include <Arduino.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include <Adafruit_VL53L0X.h>
#include <math.h>

// --- Configuración ---
const int servoPin = 18;
Servo myServo;
const int minPulse = 500, maxPulse = 2500;
const float servoMin = 130, servoMax = 175, midAngle = (servoMin+servoMax)/2;
Adafruit_VL53L0X lox;

// PID y autotune
float Kp=0.132, Ki=0.188, Kd=0.023;
float setpoint=200;
bool tuning=false;

// Deadband de error
float errMin=-5, errMax=5;

// Relay autotune vars
int relayCycles=10, switchCnt=0;
unsigned long lastSwitch, lastPid;
float relayAmp=15, maxE=-1e6, minE=1e6;
const unsigned long pidInterval=100;

void setup(){
  Serial.begin(115200);
  Wire.begin(21,22);
  lox.begin();
  myServo.setPeriodHertz(50);
  myServo.attach(servoPin,minPulse,maxPulse);
  lastSwitch=millis(); lastPid=millis();
  Serial.println("System listo. Comandos: KP,KI,KD,SET,TUNE ON/OFF,ERROR min max");
}

void loop(){
  if(millis()-lastPid<pidInterval) return;
  lastPid=millis();
  handleSerial();

  float dist=readDist(), error=setpoint-dist;
  maxE=max(maxE,error); minE=min(minE,error);

  if(tuning){
    // Relay autotune
    float angle=(switchCnt%2?midAngle-relayAmp:midAngle+relayAmp);
    writeServo(angle);
    if((error>=0&&switchCnt%2)||(error<0&&!(switchCnt%2))){
      unsigned long dt=millis()-lastSwitch; lastSwitch=millis(); switchCnt++;
      if(switchCnt>=relayCycles*2){ computePID(dt); tuning=false; }
    }
  } else {
    // PID control con deadband
    static float integral=0, prevE=0;
    float output=0;
    if(error<errMin||error>errMax){
      integral+=error*(pidInterval/1000.0);
      float deriv=(error-prevE)/(pidInterval/1000.0);
      output=Kp*error+Ki*integral+Kd*deriv;
      prevE=error;
    }
    writeServo(constrain(midAngle+output,servoMin,servoMax));
  }

  Serial.printf("Dist:%.1f Err:%.1f Kp:%.3f Ki:%.3f Kd:%.3f Tun:%d\n",
                dist,error,Kp,Ki,Kd,tuning);
}

void handleSerial(){
  while(Serial.available()){
    String c=Serial.readStringUntil('\n'); c.trim();
    if(c.startsWith("KP ")) Kp=c.substring(3).toFloat();
    else if(c.startsWith("KI ")) Ki=c.substring(3).toFloat();
    else if(c.startsWith("KD ")) Kd=c.substring(3).toFloat();
    else if(c.startsWith("SET ")) setpoint=c.substring(4).toFloat();
    else if(c.equalsIgnoreCase("TUNE ON")) tuning=true;
    else if(c.equalsIgnoreCase("TUNE OFF")) tuning=false;
    else if(c.startsWith("ERROR ")){
      float a, b; sscanf(c.c_str(),"ERROR %f %f",&a,&b);
      errMin=a; errMax=b;
    }
    Serial.printf("KP=%.3f KI=%.3f KD=%.3f SET=%.1f TUNE=%d ERR=[%.1f,%.1f]\n",
                  Kp,Ki,Kd,setpoint,tuning,errMin,errMax);
  }
}

float readDist(){ VL53L0X_RangingMeasurementData_t m; lox.rangingTest(&m,false);
  return m.RangeStatus!=4?m.RangeMilliMeter:setpoint; }

void writeServo(float ang){
  int pw=minPulse+(ang/180)*(maxPulse-minPulse);
  myServo.writeMicroseconds(pw);
}

void computePID(unsigned long dt){
  float Pu=(dt*2)/1000.0;
  float a=(maxE-minE)/2;
  float Ku=4*relayAmp/(PI*a);
  Kp=0.6*Ku; Ki=1.2*Ku/Pu; Kd=0.075*Ku*Pu;
}
