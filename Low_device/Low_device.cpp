#include "Low_device.h"
#include "mbed.h"
 
DigitalOut  LED_R(P6_13);               /* LED1 on the GR-PEACH board */
DigitalOut  LED_G(P6_14);               /* LED2 on the GR-PEACH board */
DigitalOut  LED_B(P6_15);               /* LED3 on the GR-PEACH board */
DigitalOut  LED_3(P2_14);               /* LED3 on the Motor Drive board */
DigitalOut  LED_2(P2_15);               /* LED2 on the Motor Drive board */

DigitalIn   user_botton(P6_0);          /* SW1 on the GR-PEACH board */
BusIn       dipsw( P7_15, P8_1, P2_9, P2_10 ); /* SW1 on Shield board */
DigitalIn   push_sw(P2_13);             /* SW1 on the Motor Drive board */
 
Low_device::Low_device() {
}
 
 // RGB LED control
void Low_device::led_RGB(int led) {
    LED_R = led & 0x1;
    LED_G = (led >> 1 ) & 0x1;
    LED_B = (led >> 2 ) & 0x1;
}

//led_out(on Motor drive board)
//------------------------------------------------------------------//
void Low_device::led_OUT(int led)
{
    led = ~led;
    LED_3 = led & 0x1;
    LED_2 = ( led >> 1 ) & 0x1;
}
//PEACH_button_get(on GR-PEACH board)
//------------------------------------------------------------------//
unsigned int Low_device::peach_button_get( void )
{
    return (~user_botton) & 0x1;        /* Read ports with switches */
}

//pushsw_get(on Motor drive board)
//------------------------------------------------------------------//
unsigned int Low_device::pushsw_get( void )
{
    return (~push_sw) & 0x1;            /* Read ports with switches */
}
//******************************************************************//
// functions ( on Shield board )
//*******************************************************************/
//------------------------------------------------------------------//
//Dipsw get Function
//------------------------------------------------------------------//
unsigned char Low_device::dipsw_get( void )
{
    return( dipsw.read() & 0x0f );
}
