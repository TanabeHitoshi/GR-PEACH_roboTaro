#ifndef DRIVE_H
#define DRIVE_H
 
#include "mbed.h"
//Motor PWM cycle
#define     MOTOR_PWM_CYCLE     33332   /* Motor PWM period         */

//Servo PWM cycle
#define     SERVO_PWM_CYCLE     33332   /* SERVO PWM period         */
/* 16ms   P0φ/16 = 0.48us   */
//#define     SERVO_CENTER        3224    /* 1.5ms / 0.48us - 1 = 3124*/
#define     SERVO_CENTER        3114    /* 1.5ms / 0.48us - 1 = 3124*/
#define     HANDLE_STEP         18      /* 1 degree value           */
//Handle
#define     ANLOG_STEP          60      /*                          */
class Drive {
public:
    Drive(void);
    void run(int accele,int turn);
    void motor( int accele_l, int accele_r, int mode );           //motor speed control(PWM)   
    void motor_drive(void);     
    void handle( int angle );                           //Handle fanction
    volatile int            handle_buff;
    void reset_tripmeter( void );                       //tripmeter reset
    long get_tripmeter(void);                           // tripmeter value get
    int diff( int pwm );                                //image_sensorAnalog_get
     int Max_Speed;                             //max speed
    int accele_l, accele_r,mode;
    long tripmeter,tripmeter_l,tripmeter_r; /* PWM値の積分値     */
private: 
    void init_MTU2_PWM_Motor( void );                   //Initialize MTU2 PWM functions
    void init_MTU2_PWM_Servo( void );                   //Initialize MTU2 PWM functions

};
 
#endif
