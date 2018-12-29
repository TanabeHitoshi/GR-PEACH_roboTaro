#include "Drive.h"
#include "mbed.h"
#include "math.h"
#include "iodefine.h"
#include "DisplayBace.h"
#include "Camera.h"

DigitalOut  Left_motor_signal(P4_6);    /* Used by motor fanction   */
DigitalOut  Right_motor_signal(P4_7);   /* Used by motor fanction   */

Serial      pcc(USBTX, USBRX);

Drive::Drive()
{
    init_MTU2_PWM_Motor();
    init_MTU2_PWM_Servo();
    Max_Speed = 50;

}

//motor speed control(PWM)
//Arguments: motor:-100 to 100
//Here, 0 is stop, 100 is forward, -100 is reverse
//------------------------------------------------------------------//
void Drive::run( int accele ,int turn)
{
    if(turn > 0)
//        motor(accele,accele,1);
        motor(accele,diff(accele),1);
    else
//        motor(accele,accele,1);
        motor(diff(accele),accele,1);
}
//motor speed control(PWM)
//Arguments: motor:-100 to 100
//Here, 0 is stop, 100 is forward, -100 is reverse
// mode 0 -> non Max_Speed  1 -> used Max_Speed
//------------------------------------------------------------------//
void Drive::motor( int set_accele_l, int set_accele_r, int set_mode )
{
    if(set_mode){
        accele_l = ( set_accele_l * Max_Speed ) / 100;
        accele_r = ( set_accele_r * Max_Speed ) / 100;
    }else{
        accele_l = set_accele_l;
        accele_r = set_accele_r;
    }
}

//motor speed control(PWM)
//Arguments: motor:-100 to 100
//Here, 0 is stop, 100 is forward, -100 is reverse
// mode 0 -> non Max_Speed  1 -> used Max_Speed
//------------------------------------------------------------------//
void Drive::motor_drive( void )
{
     /* Left Motor Control */
    if( accele_l >= 0 ) {
        /* forward */
        Left_motor_signal = 0;
        MTU2TGRC_4 = (long)( MOTOR_PWM_CYCLE - 1 ) * accele_l / 100;
    } else {
        /* reverse */
        Left_motor_signal = 1;
        MTU2TGRC_4 = (long)( MOTOR_PWM_CYCLE - 1 ) * ( -accele_l ) / 100;
    }

    /* Right Motor Control */
    if( accele_r >= 0 ) {
        /* forward */
        Right_motor_signal = 0;
        MTU2TGRD_4 = (long)( MOTOR_PWM_CYCLE - 1 ) * accele_r / 100;
    } else {
        /* reverse */
        Right_motor_signal = 1;
        MTU2TGRD_4 = (long)( MOTOR_PWM_CYCLE - 1 ) * ( -accele_r ) / 100;
    }
    /* PWM値を積分して走行距離とする。ただし－は積算しない */
    if(accele_l > 0)tripmeter_l += (long)accele_l;
    if(accele_r > 0)tripmeter_r += (long)accele_r;
    tripmeter = (tripmeter_l + tripmeter_r) / 2;
}

//INITIALIZE MTU2 PWM FUNCTIONS
//------------------------------------------------------------------//
//MTU2_3, MTU2_4
//Reset-Synchronized PWM mode
//TIOC4A(P4_4) :Left-motor
//TIOC4B(P4_5) :Right-motor
//------------------------------------------------------------------//
void Drive::init_MTU2_PWM_Motor( void )
{
    /* Port setting for S/W I/O Contorol */
    /* alternative mode     */

    /* MTU2_4 (P4_4)(P4_5)  */
    GPIOPBDC4   = 0x0000;               /* Bidirection mode disabled*/
    GPIOPFCAE4 &= 0xffcf;               /* The alternative function of a pin */
    GPIOPFCE4  |= 0x0030;               /* The alternative function of a pin */
    GPIOPFC4   &= 0xffcf;               /* The alternative function of a pin */
    /* 2nd altemative function/output   */
    GPIOP4     &= 0xffcf;               /*                          */
    GPIOPM4    &= 0xffcf;               /* p4_4,P4_5:output         */
    GPIOPMC4   |= 0x0030;               /* P4_4,P4_5:double         */

    /* Mosule stop 33(MTU2) canceling */
    CPGSTBCR3  &= 0xf7;

    /* MTU2_3 and MTU2_4 (Motor PWM) */
    MTU2TCR_3   = 0x20;                 /* TCNT Clear(TGRA), P0φ/1  */
    MTU2TOCR1   = 0x04;                 /*                          */
    MTU2TOCR2   = 0x40;                 /* N L>H  P H>L             */
    MTU2TMDR_3  = 0x38;                 /* Buff:ON Reset-Synchronized PWM mode */
    MTU2TMDR_4  = 0x30;                 /* Buff:ON                  */
    MTU2TOER    = 0xc6;                 /* TIOC3B,4A,4B enabled output */
    MTU2TCNT_3  = MTU2TCNT_4 = 0;       /* TCNT3,TCNT4 Set 0        */
    MTU2TGRA_3  = MTU2TGRC_3 = MOTOR_PWM_CYCLE;
    /* PWM-Cycle(1ms)           */
    MTU2TGRA_4  = MTU2TGRC_4 = 0;       /* Left-motor(P4_4)         */
    MTU2TGRB_4  = MTU2TGRD_4 = 0;       /* Right-motor(P4_5)        */
    MTU2TSTR   |= 0x40;                 /* TCNT_4 Start             */
}

//Handle fanction
//------------------------------------------------------------------//
void Drive::handle( int angle )
{
    handle_buff = angle;
    /* When the servo move from left to right in reverse, replace "-" with "+" */
 //   MTU2TGRD_0 = SERVO_CENTER - angle * HANDLE_STEP;
    MTU2TGRD_0 = SERVO_CENTER - angle;
}
//Initialize MTU2 PWM functions
//------------------------------------------------------------------//
//MTU2_0
//PWM mode 1
//TIOC0A(P4_0) :Servo-motor
//------------------------------------------------------------------//
void Drive::init_MTU2_PWM_Servo( void )
{
    /* Port setting for S/W I/O Contorol */
    /* alternative mode     */

    /* MTU2_0 (P4_0)        */
    GPIOPBDC4   = 0x0000;               /* Bidirection mode disabled*/
    GPIOPFCAE4 &= 0xfffe;               /* The alternative function of a pin */
    GPIOPFCE4  &= 0xfffe;               /* The alternative function of a pin */
    GPIOPFC4   |= 0x0001;               /* The alternative function of a pin */
    /* 2nd alternative function/output   */
    GPIOP4     &= 0xfffe;               /*                          */
    GPIOPM4    &= 0xfffe;               /* p4_0:output              */
    GPIOPMC4   |= 0x0001;               /* P4_0:double              */

    /* Mosule stop 33(MTU2) canceling */
    CPGSTBCR3  &= 0xf7;

    /* MTU2_0 (Motor PWM) */
    MTU2TCR_0   = 0x22;                 /* TCNT Clear(TGRA), P0φ/16 */
    MTU2TIORH_0 = 0x52;                 /* TGRA L>H, TGRB H>L       */
    MTU2TMDR_0  = 0x32;                 /* TGRC and TGRD = Buff-mode*/
    /* PWM-mode1                */
    MTU2TCNT_0  = 0;                    /* TCNT0 Set 0              */
    MTU2TGRA_0  = MTU2TGRC_0 = SERVO_PWM_CYCLE;
    /* PWM-Cycle(16ms)          */
    MTU2TGRB_0  = MTU2TGRD_0 = 0;       /* Servo-motor(P4_0)        */
    MTU2TSTR   |= 0x01;                 /* TCNT_0 Start             */
}
/************************************************************************/
/* トリップメータのリセット     */
/* 引数　 なし                                                      */
/* 戻り値 なし                                                       */
/************************************************************************/
void Drive::reset_tripmeter( void )
{
    tripmeter_l = tripmeter_r = 0;
}
/************************************************************************/
/* トリップメータの取得     */
/* 引数　 なし                                                      */
/* 戻り値    トリップメータの値                                                    */
/************************************************************************/
long Drive::get_tripmeter( void )
{
    return tripmeter;
}
//Deff fanction
//------------------------------------------------------------------//
int Drive::diff( int pwm )
{
    int i, ret;
    int revolution_difference[] = {
        100, 98, 97, 95, 93,
        92, 90, 88, 87, 85,
        84, 82, 81, 79, 78,
        76, 75, 73, 72, 71,
        69, 68, 66, 65, 64,
        62, 61, 60, 60, 60,
        60, 60, 60, 60, 60,
        60, 60, 60, 60, 60,
        60, 60, 60, 60, 60,
        60
    };
    i  = handle_buff / 3;
    if( i <  0 ) i = -i;
    if( i > 45 ) i = 45;
    ret = revolution_difference[i] * pwm / 100;

    return ret;
}
