#include <ECE3.h>

  uint16_t sensorMin[8];
  uint16_t sensorMax[8];
  uint16_t sensorValues[8];
  int calibrated[8];

  // Constants for left wheel
  const int left_nslp_pin=31; // nslp ==> awake & ready for PWM
  const int left_dir_pin=29;
  const int left_pwm_pin=40;
  // Constants for right wheel
  const int right_nslp_pin=11; // nslp ==> awake & ready for PWM
  const int right_dir_pin=30;
  const int right_pwm_pin=39;
  // LED 
  const int LED_RF = 41;

  const int refSpeed = 70;
  int fusionPrev = 0;
  int fusionCur = 0;
  int meetEnd = 0;
  bool stopCar = false;

void setup() {
  ECE3_Init();
  // Set serial baud rate
  Serial.begin(9600); 
  // put your setup code here, to run once:
  // Setting up left wheel
  pinMode(left_nslp_pin,OUTPUT);
  pinMode(left_dir_pin,OUTPUT);
  pinMode(left_pwm_pin,OUTPUT);
  // Setting up right wheel
  pinMode(right_nslp_pin,OUTPUT);
  pinMode(right_dir_pin,OUTPUT);
  pinMode(right_pwm_pin,OUTPUT);
  
  digitalWrite(left_dir_pin,LOW);
  digitalWrite(left_nslp_pin,HIGH);
  digitalWrite(right_dir_pin,LOW);
  digitalWrite(right_nslp_pin,HIGH);

  pinMode(LED_RF, OUTPUT);

  // Calibration in the beginning
  // LED blink 5 times to indicate starting
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_RF,HIGH);
    delay(500);
    digitalWrite(LED_RF,LOW);
    delay(500);
  }
  ECE3_read_IR(sensorValues);
  for (int i = 0; i < 8; i++) {
    sensorMin[i] = sensorValues[i];
    sensorMax[i] = sensorValues[i];
  }
  // Sensors start reading in data
  for (int i = 0; i < 100; i++) {
    ECE3_read_IR(sensorValues);
    for (int j = 0; j < 8; j++) {
      // set Min and Max to the current Min and Max
      if (sensorMin[j] > sensorValues[j]) 
        sensorMin[j] = sensorValues[j];
      if (sensorMax[j] < sensorValues[j])
        sensorMax[j] = sensorValues[j];
    }
    delay(50);
  }

  // Use LED to notify done reading
  digitalWrite(LED_RF,HIGH);
  delay(2000);
  // Place the car in the right position
  // LED blink 5 times to indicate starting
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_RF,HIGH);
    delay(500);
    digitalWrite(LED_RF,LOW);
    delay(500);
  }
}

void loop() {
  if (!stopCar) {
    // put your main code here, to run repeatedly: 
    // read in values
    ECE3_read_IR(sensorValues);
  
    // initialize left and right pwm
    int left_pwm = 0;
    int right_pwm = 0;
    
    // calibrate the values
    for (int i = 0; i < 8; i++) {
      calibrated[i] = (sensorValues[i] - sensorMin[i]) * 1000 / sensorMax[i];
    }
  
    fusionCur = (-15*calibrated[0] - 14*calibrated[1] - 12*calibrated[2] - 8*calibrated[3]
                 + 8*calibrated[4] + 12*calibrated[5] + 14*calibrated[6] + 15*calibrated[7])/8;
    left_pwm = refSpeed - 0.0817*fusionCur; // - 0.005*(fusionPrev - fusionCur);
    right_pwm = refSpeed + 0.0817*fusionCur; // + 0.005*(fusionPrev - fusionCur);
    fusionPrev = fusionCur;
    
    if ((sensorValues[1] >= sensorMax[1] - 300) && (sensorValues[2] >= sensorMax[2] - 300) && 
        (sensorValues[3] >= sensorMax[3] - 300) && (sensorValues[4] >= sensorMax[4] - 300) && 
        (sensorValues[5] >= sensorMax[5] - 300) && (sensorValues[6] >= sensorMax[6] - 300)) {
          if (!meetEnd) {
            left_pwm = 150;
            right_pwm = 150;
            digitalWrite(left_dir_pin, HIGH);
            analogWrite(left_pwm_pin, left_pwm);
            analogWrite(right_pwm_pin, right_pwm);
            delay(500);
            digitalWrite(left_dir_pin, LOW);
            meetEnd++;
          } else {
            stopCar = true;
            left_pwm = 0;
            right_pwm = 0;
          }
    }
    analogWrite(left_pwm_pin, left_pwm);
    analogWrite(right_pwm_pin, right_pwm);
  }
  
}
