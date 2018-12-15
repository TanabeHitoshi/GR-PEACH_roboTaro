#ifndef LOW_DEVICE_H
#define LOW_DEVICE_H
 
#include "mbed.h"
 
class Low_device {
public:
    Low_device(void);
    void led_RGB(int led);                      // RGB LED control
    void led_OUT(int led);                      //led_out(on Motor drive board)
    unsigned int peach_button_get( void );      //PEACH_button_get(on GR-PEACH board)
    unsigned int pushsw_get( void );            //pushsw_get(on Motor drive board)
    unsigned char dipsw_get( void );            //Shield board
private:  

};
 
#endif