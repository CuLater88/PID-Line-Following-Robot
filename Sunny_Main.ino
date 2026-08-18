/*
Main.ino For Sunny the robot
Sunny can run autonomously, as well as be controlled remotely via bluetooth functionality
*/

#include <math.h>
#include <Servo.h>
#include <Wire.h>

int servo_P = 11;
int trig = 13;
int echo = 12;
byte incomingByte;

class Motor_Control{

  private:
  /*
  P and N = Positive polarity and Negitive polarity
  The motors work by reversing the flow of current (P == HIGH && N == LOW || P == LOW && N == HIGH)
  NOTE: Speed is from scale 0 to 255
  */
    const int STBY = 3; 

    const int PWM_R = 5;
    const int right_P = 7;  
    const int right_N = 9;

    const int PWM_L = 6;
    const int left_P = 8;
    const int left_N = 10;
    
    // NOTE: INV reverses the polarity of the motors, so forward becomes backward and vis versa
  public:
    /*
    REFER TO DRV8835 DATA SHEET
    PAGES 3 AND 7
    */
    void motor_Setup(){
      pinMode(STBY, OUTPUT);

      pinMode(PWM_R, OUTPUT);
      pinMode(right_P, OUTPUT);
      pinMode(right_N, OUTPUT);

      pinMode(PWM_L, OUTPUT);
      pinMode(left_P, OUTPUT);
      pinMode(left_N, OUTPUT);

      digitalWrite(STBY, HIGH);   // IMPORTANT -> sets standby mode to off 

    }

    void stop(){
      analogWrite(PWM_R, 0);
      analogWrite(PWM_L, 0);
    }

    void circles(int speed = 100){
      // makes the right side go forward (P --> N)
      digitalWrite(right_P, HIGH);
      digitalWrite(right_N, LOW);
      analogWrite(PWM_R, speed); // Speed scale 0-255

      // makes the left side go backward (N --> P)
      digitalWrite(left_P, LOW);
      digitalWrite(left_N, HIGH);
      analogWrite(PWM_L, speed); // Speed scale 0-255

      delay(5000);
      stop();

      // car go in opposite circles now
      digitalWrite(right_P, LOW);
      digitalWrite(right_N, HIGH);
      analogWrite(PWM_R, speed);

      digitalWrite(left_P, HIGH);
      digitalWrite(left_N, LOW);
      analogWrite(PWM_L, speed);

      delay(5000);
      stop();
    }
    
    void car_forward(int speedR=200, int speedL=200){
      // NOTE: Speed is from scale 0 to 255

      // makes the right side go forward (P --> N)
      digitalWrite(right_P, HIGH);
      digitalWrite(right_N, LOW);
      analogWrite(PWM_R, speedR); // Speed scale 0-255

      // makes the left side go forward (P --> N)
      digitalWrite(left_P, HIGH);
      digitalWrite(left_N, LOW);
      analogWrite(PWM_L, speedL); // Speed scale 0-255
    }

    void car_backward(int speed=200){
      // NOTE: Speed is from scale 0 to 255

      digitalWrite(right_P, LOW);
      digitalWrite(right_N, HIGH);
      analogWrite(PWM_R, speed); // Speed scale 0-255

      digitalWrite(left_P, LOW);
      digitalWrite(left_N, HIGH);
      analogWrite(PWM_L, speed); // Speed scale 0-255
    }

    void car_left(int speed=200){
      // NOTE: Speed is from scale 0 to 255

      digitalWrite(right_P, HIGH);
      digitalWrite(right_N, LOW);
      analogWrite(PWM_R, speed); // Speed scale 0-255

      digitalWrite(left_P, LOW);
      digitalWrite(left_N, HIGH);
      analogWrite(PWM_L, speed); // Speed scale 0-255
    }

    void car_right(int speed=200){
      // NOTE: Speed is from scale 0 to 255

      digitalWrite(right_P, LOW);
      digitalWrite(right_N, HIGH);
      analogWrite(PWM_R, speed); // Speed scale 0-255

      digitalWrite(left_P, HIGH);
      digitalWrite(left_N, LOW);
      analogWrite(PWM_L, speed); // Speed scale 0-255
    }

    void turn_degs(float degree, char direction='R'){
      long time_Delay;
      if(degree <= 90){
        time_Delay = (275.5 * degree)/90.0;
      }else{
        time_Delay = (1000 * degree)/360.0;
      }
      
      switch(direction){
        case 'R':
          car_right(200);
          delay(time_Delay);
          stop();
          delay(500);
          break;
        case 'L':
          car_left(200);
          delay(time_Delay);
          stop();
          delay(500);
          break;
      }
    }

};

class Sensors{
  private:

  public:

  void sensors_Setup(){
    pinMode(trig, OUTPUT);
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    pinMode(echo, INPUT);
    delay(6000);
  }

  float ultrasonic_Sensors(int trig, int echo){
    digitalWrite(trig, LOW);
    delayMicroseconds(2);    // Used to trigger input pin to start ranging (refer to specs sheet)
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);
    long duration = pulseIn(echo, HIGH, 38000);   // duration in micro seconds
    delay(60);
    float inches = 0;
    if(duration>=38000){
      Serial.print("Out range");
      inches = 257;   // duration in maximum amount of inches
      return inches;
      }
    else{
      float distance = duration/58.0;   // refer to specs sheet for formula
      // distance in centimeters
      inches=distance/2.54;
      return inches;
      }
    }

  float average(float arr[]){
    int len = sizeof(arr);
    float avg = 0;
    for(int i = 0; i < len; i++){
      avg += arr[i];
    }
    float val = avg/len;
    return val;
  }

};

class Obstacle_Avoidance{
  private:
    Servo servo;
    Motor_Control motors;
    Sensors sensor;

    const int min_distance = 15;    // in inches

  public:

  void avoidance_Setup(int Servo_Pin){
    servo.attach(Servo_Pin);
    servo.write(0);
    delay(500);
    servo.write(90);
    delay(500);
    servo.write(180);
    delay(500);
    servo.write(90);

  }

  bool enviroment_Sweep(){
    bool scan = false;
    int us_Buffer = 1;
    while(scan != true){
      float ultra_Sonic = 0;
      for(int i = 0; i<=360;i+=10){

        int angle;
        if(i <= 180){
          angle = i;
        }else{
          angle = 360 - i;
        }

        ultra_Sonic = sensor.ultrasonic_Sensors(trig, echo);
        if(ultra_Sonic < min_distance && ultra_Sonic > us_Buffer){
          scan = true;
          return scan;
        }
        servo.write(angle);
        // delayMicroseconds(50);
      }
    }
  }

  void direction_Change(){
    // does a sweeping scan to see what direction has open space (0 to 90 - right) and (90 to 180 - left)
    // compares the distance free dected on both sides and turns which ever direction has the most free distance
    // then drives in that direction

    float max_Angle = 0;
    float max_Dist = 0;
    float ultra_Sonic = 0;

    for(int i = 0; i<=360;i+=10){
      int angle;
        if(i <= 180){
          angle = i;
        }else{
          angle = 360 - i;
        }
      ultra_Sonic = sensor.ultrasonic_Sensors(trig, echo);
      if(max_Dist < ultra_Sonic){ // keeps track of the highest value and its associated angle during the sweep
        max_Dist = ultra_Sonic;
        max_Angle = angle;
      }
      // Serial.println(i);
      servo.write(angle);
      //delay(1);
    }

    if(max_Angle <= 90){  // if it was detected to the RIGHT of the bot
      motors.turn_degs(max_Angle,'R');
    }else{
      motors.turn_degs(max_Angle,'L');
    }
  }

  void obstacle_Avoid(){
    bool avoid = enviroment_Sweep();
    if(avoid == true){
      motors.stop();
      direction_Change();
    }
  }

};

class MPU650_MotionTracking{
  // Make sure to include Wire.h
  private:
    const byte DEVICE_ADDR = 0x68;    // this is the device address of the MPU6050
  public:
    float x_Offset=0;
    float y_Offset=0;
    float z_Offset=0;

  void motionTracking_setup(){
    Wire.begin();
    Wire.setClock(400000);
    Wire.beginTransmission(DEVICE_ADDR);    // this line sets up communcation with the device using the device address MPU
      Wire.write(0x6B);       // This line selects register name "PWR_MGMT_1"
      Wire.write(0);          // This line essentially "wakes" the device up by replacing its sleep bit from 1(sleep) to 0(awake)
    Wire.endTransmission();   // this line ensures the value is replaced

    gyro_Calibration();
  }

  void gyro_Calibration(){
    float gyro_x=0;
    float gyro_y=0;
    float gyro_z=0;

    int max_Test = 1000;
    for(int c = 0; c<max_Test; c++){
      Wire.beginTransmission(DEVICE_ADDR);
      Wire.write(0x43);              // tells the device to start at this register
      // when it reads the device it starts from the lowest register specified (43) and goes up
      Wire.endTransmission(false);
      Wire.requestFrom(DEVICE_ADDR,6);
      int16_t xH = Wire.read();    // reads 2 bytes
      int16_t xL = Wire.read();
      int16_t yH = Wire.read();
      int16_t yL = Wire.read();
      int16_t zH = Wire.read();
      int16_t zL = Wire.read();

      int16_t x = (xH << 8) | xL;  // combine high and low byte into one 16-bit value
                    // NOTE: << is a bit shift operator
      int16_t y = (yH << 8) | yL;  // combine high and low byte into one 16-bit value
      int16_t z = (zH << 8) | zL;  // combine high and low byte into one 16-bit value

      gyro_x += x / 131.0;    // convert to degrees per second
      gyro_y += y / 131.0;    // convert to degrees per second
      gyro_z += z / 131.0;    // convert to degrees per second
      delay(1);
    }

    x_Offset = gyro_x/max_Test;
    y_Offset = gyro_y/max_Test;
    z_Offset = gyro_z/max_Test;
  }

  void gyroScope(float gyro_Data[3]){
    Wire.beginTransmission(DEVICE_ADDR);
    Wire.write(0x43);              // tells the device to start at this register
    // when it reads the device it starts from the lowest register specified (43) and goes up
    Wire.endTransmission(false);
    Wire.requestFrom(DEVICE_ADDR,6);
    int16_t xH = Wire.read();    // reads 2 bytes
    int16_t xL = Wire.read();
    int16_t yH = Wire.read();
    int16_t yL = Wire.read();
    int16_t zH = Wire.read();
    int16_t zL = Wire.read();

    int16_t x = (xH << 8) | xL;  // combine high and low byte into one 16-bit value
                  // NOTE: << is a bit shift operator
    int16_t y = (yH << 8) | yL;  // combine high and low byte into one 16-bit value
    int16_t z = (zH << 8) | zL;  // combine high and low byte into one 16-bit value

    float gyro_x = (x / 131.0) + x_Offset;    // convert to degrees per second
    float gyro_y = (y / 131.0) + y_Offset;    // convert to degrees per second
    float gyro_z = (z / 131.0) + z_Offset;    // convert to degrees per second

    gyro_Data[0] = gyro_x;
    gyro_Data[1] = gyro_y;
    gyro_Data[2] = gyro_z;
    // Serial.println(gyro_Data[0]);
    // Serial.println(gyro_Data[1]);
    Serial.println(gyro_Data[2]);

  }

};

class PID{
  private:
    unsigned previous_Time = 0;
    float Kp = 0;
    float Ki = 0;
    float Kd = 0;
    float desired_Val;

    float integral = 0;
    float old_eT = 0;
  
  public:

  PID(float ideal_Val){
    desired_Val = ideal_Val;
  }

  float pid_Output(float kp, float ki, float kd, float actual_Val){
    Kp = kp;
    Ki = ki;
    Kd = kd;
    unsigned current_Time = millis();

    float dt = (current_Time - previous_Time)/1000.0;
    if (dt <= 0){
      return 0;
    }
    float eT = desired_Val - actual_Val;

    float P = Kp * eT;
    integral += eT * dt;
    float I = Ki * integral;
    float derivative = (eT - old_eT) / dt;
    float D = Kd * derivative;

    float pid = P + I + D;
    previous_Time = current_Time;
    old_eT = eT;
    return pid;
  }
};

class BlueTooth{
  private:
    Motor_Control motors;
  public:

  void pid_Blu(float &kp, float &ki, float &kd){
    char weight;
    // float kp = Kp;
    // float ki = Ki;
    // float kd = Kd;
    if(Serial.available() > 0){
      weight = Serial.read();
      Serial.println("Controller Variable: " + String(weight));
      switch(weight){
        case 'p':
          do{
            kp = Serial.parseFloat();
          }while(Serial.available() == 0);
          Serial.println("NEW PROPORTIONAL VALUE: " + String(kp));
          break;
        case 'i':
          do{
            ki = Serial.parseFloat();
          }while(Serial.available() == 0);
          Serial.println("NEW INTEGRAL VALUE: " + String(ki));
          break;
        case 'd':
          do{
            kd = Serial.parseFloat();
          }while(Serial.available() == 0);
          Serial.println("NEW DERIVATIVE VALUE: " + String(kd));
          break;
      }
    }
  }

  void remote_Control(int &speed){
    char command = Serial.read();
    switch(command){
      case 'V':
        speed = Serial.read();
        break;
      case 'F':
        motors.car_forward(speed, speed);
        break;
      case 'B':
        motors.car_backward(speed);
        break;
      case 'L':
        motors.car_left(speed);
        break;
      case 'R':
        motors.car_right(speed);
      default:
        Serial.println("Car STOP");
        motors.stop();
        break;
    }
  }

};

// Object creation
Servo servo;
Motor_Control motors;
Sensors sensor;
Obstacle_Avoidance avoid;
MPU650_MotionTracking motionT;
BlueTooth blu;
PID pid(0);


void setup() {
  /* 
  Sunny Startup Routine
  This test the Ultra-sonic servo directions and 
  initializes the motor and sensors.
  */
  motors.motor_Setup();
  sensor.sensors_Setup();
  avoid.avoidance_Setup(servo_P);
  motionT.motionTracking_setup();
  

  Serial.begin(9600);
  Serial.println("TERMINAL RESET HERE");
  
  float valx = motionT.x_Offset;
  float valy = motionT.y_Offset;
  float valz = motionT.z_Offset;
  Serial.println(valx);
  Serial.println(valy);
  Serial.println(valz);

  delay(2000);
  Serial.println("The PID Bluetooth control is:\np## = Proportional Value\ni## = Integral Value\nd## = Derivative Value");
}


int speed = 100;
float gyro_Data[3];
char weight;
float kp = 15.0;
float ki = 2.0;
float kd = 0.2;


void loop() {
  Serial.println("Change Mode?");
  if(Serial.available() > 0){
    String mode = Serial.readString();
    while(mode == 'remote'){       // bluetooth remote capability
      Serial.println("Remote Control Active");
      blu.remote_Control(speed);
      mode = Serial.readString();
    }
    if(mode == 'sunny'){        // autonomous capabality
      Serial.println("PID Tuning Active");
      blu.pid_Blu(kp,ki,kd);    // capability to manually change PID values via bluetooth functionality between phone and robot
    }
  }

  avoid.obstacle_Avoid();
  motors.car_forward(speed,speed);
  motionT.gyroScope(gyro_Data);

  // MANUAL PID TUNING
  float pid_Out = pid.pid_Output(kp,ki,kd,gyro_Data[2]);
  Serial.print("\nP: "+String(kp)+"\nI: "+String(ki)+"\nD: "+String(kd)+"\n");
  delay(1000);
  if(gyro_Data[2] < 0){   // if the car drifts right
    int L_Speed = abs(speed - pid_Out);
    motors.car_forward(speed, L_Speed);
  }else{    // if the car drifts left
    int R_Speed = abs(speed - pid_Out);
    motors.car_forward(R_Speed, speed);
  }
  
  // Serial.println(gyro_Data[0]);
  // Serial.println(gyro_Data[1]);

  Serial.println("GYRO SCOPE YAW(z) VALUE:");
  Serial.println(gyro_Data[2]);
  Serial.println("PID CORRECTION VALUE:");
  Serial.println(pid_Out);
  delay(1000);

}



