#include <Arduino.h>
#include <Wire.h>
#include "I2CFunctions.h"
// We got a Lidar object per laser. 

#ifndef LIDAR_OBJECT_H
#define LIDAR_OBJECT_H

enum LIDAR_STATE {
  SHUTING_DOWN = 240,       // Shutdown the laser to reset it
  NEED_RESET = 48,          // Too much outliers, need to reset
  RESET_PENDING = 80,       // Wait 15ms after you reset the Lidar, we are waiting in this state
  ACQUISITION_IN_PROGRESS = 64, // The acquisition in on progress
};

enum LIDAR_MODE {
  NONE = 0,
  DISTANCE = 1,
  VELOCITY = 2,
  DISTANCE_AND_VELOCITY = 3
};

class LidarObject {
  public:
/*******************************************************************************
  Constructor
*******************************************************************************/
    LidarObject() {};
/*******************************************************************************
  begin : Begin the I2C master device

  If fasti2c is true, use 400kHz I2C
*******************************************************************************/
    void begin(uint8_t _EnablePin = 2, uint8_t _ModePin = 1, uint8_t _Lidar = 0x62, uint8_t _configuration = 2,  LIDAR_MODE _mode = DISTANCE, char _name = 'A'){
      pinMode(_EnablePin, OUTPUT);
      off();
      last_distance = 0;
      distance = 0;
      velocity = 0;
      strength = 0;
      
      mode = _mode;
      configuration = _configuration;
      address = _Lidar;
      EnablePin = _EnablePin;
      ModePin = _ModePin;
      name = _name;
    };

/*******************************************************************************
  on : Power On the device
*******************************************************************************/
    void on(){
      digitalWrite(EnablePin, HIGH);
    };

/*******************************************************************************
  off : Power Off the device
*******************************************************************************/
    void off(){
      digitalWrite(EnablePin, LOW);
    };

/*******************************************************************************
  enable : ask for PWM reading and allow continuous readings
*******************************************************************************/
    void enable(){
      digitalWrite(ModePin, HIGH);
    };

/*******************************************************************************
  disable : stops PWM reading and allow continuous readings
*******************************************************************************/
    void disable(){
      digitalWrite(ModePin, LOW);
    };

/*******************************************************************************
  timerUpdate : Update the timer to the current time to start the timer.
*******************************************************************************/
    void timerUpdate(){
      timeReset = micros();
    };

/*******************************************************************************
  checkTimer : Check the reset timer to see if the laser is correctly resetted

  The laser takes 20ms to reset
*******************************************************************************/
    bool checkTimer(){
      if(lidar_state != RESET_PENDING)
        return true;

      return (micros() - timeReset > 20000);
    };

/*******************************************************************************
  checkLastMeasure : If the laser do not give new data, simply reset it
*******************************************************************************/
    bool checkLastMeasure(){
      return (micros() - lastMeasure > 200000);
    };

/*******************************************************************************
  resetNacksCount : The nack counter makes the Arduino able to know if a laser 
  needs to be resetted
*******************************************************************************/
    bool resetNacksCount(){
      nacksCount = 0;
    };

/*******************************************************************************
  setCallbackDistance : sets the distance callback 
*******************************************************************************/
    void setCallbackDistance(void (*_callback)(LidarObject * self)){
      notify_distance_cb = _callback;
    };

/*******************************************************************************
  setCallbackVelocity : sets the velocity callback
*******************************************************************************/
    void setCallbackVelocity(void (*_callback)(LidarObject * self, unsigned long dt)){
      notify_velocity_cb = _callback;
    };

/*******************************************************************************
  notify_distance : wraps the callback & callback existence verification 
*******************************************************************************/
    inline void notify_distance(){
      if(notify_distance_cb) notify_distance_cb(this);
    };

/*******************************************************************************
  notify_velocity : wraps the callback & callback existence verification 
*******************************************************************************/
    inline void notify_velocity(unsigned long dt){
      if(notify_velocity_cb) notify_velocity_cb(this, dt);
    };

    int last_distance = -1; // Last distance measured
    int distance = -1;      // Newest distance
    int velocity = 0;       // Newest velocity
    uint8_t strength = 0;   // Newest signal strength

    uint8_t nacksCount = 0;
    unsigned long timeReset = 0;
    uint8_t configuration;
    uint8_t address;
    uint8_t EnablePin;
    uint8_t ModePin;
    long lastMeasure = 0;
    char name;
    LIDAR_STATE lidar_state = NEED_RESET;
    LIDAR_MODE mode = DISTANCE;
    void (*notify_distance_cb)(LidarObject * self);
    void (*notify_velocity_cb)(LidarObject * self, unsigned long dt);
};

#endif
