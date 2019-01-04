//------------------------------------------------------------------//
//Supported MCU:   RZ/A1H
//File Contents:   Trace program 2 (Mark detection) by image processing
//                                 (GR-PEACH version on the Micon Car)
//Version number:  Ver.2.02
//Date:            2018.12.06
//Copyright:       JYOTO KOUKA Highschool
//------------------------------------------------------------------//

//------------------------------------------------------------------//
#include "mbed.h"
//#include "SDFileSystem.h"
#include "math.h"
#include "iodefine.h"
#include "DisplayBace.h"
#include "Low_device.h"
#include "Drive.h"
#include "Camera.h"
#include "isCamera.h"
#include <stdio.h>
#include <vector>

//Define
//------------------------------------------------------------------//
//LED Color on GR-PEACH
#define     LED_OFF             0x00
#define     LED_RED             0x01
#define     LED_GREEN           0x02
#define     LED_YELLOW          0x03
#define     LED_BLUE            0x04
#define     LED_PURPLE          0x05
#define     LED_SKYBLUE         0x06
#define     LED_WHITE           0x07

//Status
#define     RUN                 0x00
#define     SENSOR              0x01
#define     MARK                0x02
#define     STOP                0x03
#define     ERROR               0xff

//#define     SPEED               50
#define		MAX_MEMORY			10000
#define     mem_count           6
#define     MEMORY

//Constructor
//------------------------------------------------------------------//
Ticker      interrput;
Serial      pc(USBTX, USBRX);
Low_device  d;                          // Used LED and SW
Drive m;                                // Used Motor and Servo
isCamera c;                             // Used Camera

//Prototype
//------------------------------------------------------------------//

void intTimer( void );                  /* Interrupt fanction       */
void led_status_process( void );        /* Function for only interrupt */
void led_status_set( int set );

void ServoControl_process( void );           //ServoControl_process
volatile int            iServo;

//Prototype(NTSC-video)
//------------------------------------------------------------------//
static void IntCallbackFunc_Vfield(DisplayBase::int_type_t int_type);
static void WaitVfield(const int32_t wait_count);
static void IntCallbackFunc_Vsync(DisplayBase::int_type_t int_type);
static void WaitVsync(const int32_t wait_count);

//Prototype(Display Debug)
//------------------------------------------------------------------//
double Standard_Deviation( unsigned char *data, double *Devi, int items );

//Globle
//------------------------------------------------------------------//
volatile unsigned long  cnt0;           /* Used by timer function   */
volatile unsigned long  cnt1;           /* Used within main         */
volatile unsigned long  cntCrank;       /* Used Crank check         */
volatile int            pattern;        /* Pattern numbers          */
volatile int            status_set;     /* Status                   */
int						memory[MAX_MEMORY][5];
int						m_number;
int 					SPEED;
/* Trace by image processing */

volatile int            digital_sensor_threshold;

volatile int            white;
volatile int            black;
int                     speed;
int                     speedB;
volatile int            data[150];
volatile int            flag;
volatile int            cnt_d;

double          TempDevi_A[15];
unsigned char   TempBinary_A[15] = {0,1,1,1,0,
                                    0,0,1,0,0,
                                    0,0,0,0,0
                                   };
double          NowDevi[15];
unsigned char   NowImageBinary[15];
volatile int            sensor_x[5];
volatile int            pid_flag;
volatile double retDevi_A;
volatile double retDevi_B;
volatile double retCovari;
volatile int    retJudgeIM;
volatile int    retJudgeIM_Max;
int             X_buff, Y_buff;

//Globle(NTSC-video)
//------------------------------------------------------------------//
static volatile int32_t vsync_count;
static volatile int32_t vfield_count;

//SDFileSystem sd(P8_5,P8_6,P8_3,P8_4,"sd");
//Main
//------------------------------------------------------------------//
int main( void )
{
//    int i;
    int lanePattern[4]= {-1,1,-1,1};    // 1 -> right  -1 -> left
    int mem_pattern[mem_count];
    long mem_tripmeter[mem_count];
    int LR_Number = 0;
//    int l;
    unsigned long cntLED;
    int LR,sideLR,clankLR;
    long old_tripmeter;
    int mem;                               //記録回数
//    int saka;
    int SideLine;

    /* NTSC-Video */
    DisplayBase::graphics_error_t error;

    /* Create DisplayBase object */
    DisplayBase Display;

    /* Graphics initialization process */
    error = Display.Graphics_init(NULL);
    if (error != DisplayBase::GRAPHICS_OK) {
        printf("Line %d, error %d\n", __LINE__, error);
        while (1);
    }

    error = Display.Graphics_Video_init( DisplayBase::INPUT_SEL_VDEC, NULL);
    if( error != DisplayBase::GRAPHICS_OK ) {
        while(1);
    }

    /* Interrupt callback function setting (Vsync signal input to scaler 0) */
    error = Display.Graphics_Irq_Handler_Set(DisplayBase::INT_TYPE_S0_VI_VSYNC, 0, IntCallbackFunc_Vsync);
    if (error != DisplayBase::GRAPHICS_OK) {
        printf("Line %d, error %d\n", __LINE__, error);
        while (1);
    }

    /* Video capture setting (progressive form fixed) */
    error = Display.Video_Write_Setting(
                VIDEO_INPUT_CH,
                DisplayBase::COL_SYS_NTSC_358,
                c.write_buff_addr,
                VIDEO_BUFFER_STRIDE,
                DisplayBase::VIDEO_FORMAT_YCBCR422,
                DisplayBase::WR_RD_WRSWA_32_16BIT,
                PIXEL_VW,
                PIXEL_HW
            );
    if (error != DisplayBase::GRAPHICS_OK) {
        printf("Line %d, error %d\n", __LINE__, error);
        while (1);
    }

    /* Interrupt callback function setting (Field end signal for recording function in scaler 0) */
    error = Display.Graphics_Irq_Handler_Set(VIDEO_INT_TYPE, 0, IntCallbackFunc_Vfield);
    if (error != DisplayBase::GRAPHICS_OK) {
        printf("Line %d, error %d\n", __LINE__, error);
        while (1);
    }

    /* Video write process start */
    error = Display.Video_Start (VIDEO_INPUT_CH);
    if (error != DisplayBase::GRAPHICS_OK) {
        printf("Line %d, error %d\n", __LINE__, error);
        while (1);
    }

    /* Video write process stop */
    error = Display.Video_Stop (VIDEO_INPUT_CH);
    if (error != DisplayBase::GRAPHICS_OK) {
        printf("Line %d, error %d\n", __LINE__, error);
        while (1);
    }

    /* Video write process start */
    error = Display.Video_Start (VIDEO_INPUT_CH);
    if (error != DisplayBase::GRAPHICS_OK) {
        printf("Line %d, error %d\n", __LINE__, error);
        while (1);
    }

    /* Wait vsync to update resister */
    WaitVsync(1);

    /* Wait 2 Vfield(Top or bottom field) */
    WaitVfield(2);

    /* Initialize MCU functions */
    interrput.attach(&intTimer, 0.001);
    pc.baud(230400);

    /* Initialize Micon Car state */
    d.led_OUT( 0x0 );
    m.handle( 0 );
    m.motor( 0, 0,0 );
    pattern = 0;
    cnt1 = 0;
//    t = 0;
    flag = 0;
//    saka = 0;
    switch(d.dipsw_get() & 0x03){
    case 0:
    	SPEED = 48;
    	break;
    case 1:
    	SPEED = 50;
    	break;
    case 2:
    	SPEED = 52;
    	break;
    case 3:
    	SPEED = 54;
    	break;
    }
#ifdef  MEMORY
        pc.printf("Cource memory\n\r");
        FILE *fp= fopen("/sd/course.txt","a");
        if(fp == NULL)pc.printf("Could not open file for write\r\n");
#else
        pc.printf("Cource replay\n\r");
        d.led_OUT(0x1);
        FILE *fp= fopen("/sd/course_R.txt","r");
        if(fp == NULL)pc.printf("Could not open file for read\r\n");
        pc.printf("course parameter !\r\n");
        for(i = 0; i < mem_count; i++){
             fscanf(fp,"%d,%ld",&mem_pattern[i],&mem_tripmeter[i]);
             pc.printf("%d,%ld\n\r",mem_pattern[i],mem_tripmeter[i]);
        }
        pc.printf("\n\r\n\r");
        fclose(fp);
#endif
    mem = 0;
    /* wait to stabilize NTSC signal (about 170ms) */
    wait(0.2);
    pc.printf("Hello GR-OEACH\n\r");

    while(1) {
    	if(pattern > 9 && pattern < 1000 && d.pushsw_get()) pattern = 1000;
        c.Capture();
 //       c.Binarization2_view();
 //       c.Binarization_view();
//        c.Full_Binarization_view();
//        c.Full_Raw_view();
//            pc.printf("isSideLine %2d　　All_Black %2d\r\n",c.isSideLine(),c.All_Black());
//            pc.printf("c.isCrank %d  c.isCross %d  c.isBlack %d  c.isEndBlack %d\r\n",c.isCrank(),c.isCross(),c.isBlack(),c.isEndBlack());
//        pc.printf("pattern = %d\n\r",pattern);
//          pc.printf("dipsw %2d\n\r",d.dipsw_get());

 //       sideLR = c.isSideLine();

        switch( pattern ) {
            case 0:
                /* wait for switch input */
                m.handle( 0 );
                m.motor( 0, 0 ,0);
                if( d.pushsw_get() ) {
                    m.reset_tripmeter();
                    m.Max_Speed = SPEED;
                    d.led_OUT(0x3);
                    cnt1 = 0;
                    m_number = 0;
//                    l = 0;
                    cntLED = 300;
                    pattern = 2;
                    wait(2.0);
                    break;
                }
                if(c.isCross()) cntLED = 50;
                else            cntLED = 150;
                if( cnt1 < cntLED ) {
                    d.led_OUT( 0x1 );
                } else if( cnt1 < cntLED*2 ) {
                    d.led_OUT( 0x2 );
                } else {
                    cnt1 = 0;
                }
                break;
            case 2: /* gate check */
                if(c.BlackCount > 20 || c.isCross()) d.led_OUT( 0x3 );
                else                                 d.led_OUT( 0x0 );
                if( d.pushsw_get()){
                    pattern = 3;
                    wait(1.0);
                }
            break;
            case 3: /* gate start */
                if(c.BlackCount == 0 && !c.isCross())pattern = 5;
                if( d.pushsw_get()){
                    wait(1.5);
                    pattern = 10;
                }         
                if( cnt1 < 100 ) {
                    d.led_OUT( 0x1 );
                } else if( cnt1 < 200 ) {
                    d.led_OUT( 0x2 );
                } else {
                    cnt1 = 0;
                }            
            break;
            case 5:
                if(!c.isCross()){
                    pattern = 10;
                    cntCrank = 0;
                    wait(1.0);
                }
                if( cnt1 < cntLED ){
                    d.led_OUT( 0x01 );
                } else if( cnt1 < cntLED*2 ){
                    d.led_OUT( 0x02 );
                } else {
                    cnt1 = 0;
                }
                break;
            case 6:
                if(m.get_tripmeter() > 10000)pattern = 10;  /* 約50cmまっすぐ    */
                m.motor(70,70,0);
//                m.handle( iServo );
                break;
// Trace
            case 10:    // Normal trace
            	if(c.isCurve() == 1 ){
                    m.Max_Speed = 50;
                }else{
                	SideLine = c.isSideLine();
                	if(SideLine == -1){	//left
                		m.Max_Speed = 30;
                		c.offset_Center = -20;
                		cntCrank = 0;
                	}else if(SideLine == 1){ //Right
                		m.Max_Speed = 30;
                		c.offset_Center = 20;
                		cntCrank = 0;
                	}else{
                		m.Max_Speed = SPEED;
                		c.offset_Center = 0;
                	}
                }

                if(c.aa != -999){
            	/* クランク検知   */
/*                	if(c.isCrank_F() == 1 && cntCrank < 250){
                		m.Max_Speed = 30;
                		pattern = 300;
                	}
*/
                    clankLR = c.isHalf_Line();
                    if(c.isCrank() != 0){
                        pattern = 30;
                        old_tripmeter = m.get_tripmeter();
     //                  pc.printf("clank tripmeter = %ld\n\r",old_tripmeter);
     //                   fprintf(fp,"%d,%ld\n\r",pattern+10,old_tripmeter);
                        cnt1 = 0;
                    }
                    /* レーンチェンジ検知    */
                    if(c.isBlack() == 1 ){
                        pattern = 51;     //Lean change
                        LR = c.isSideLine();
//                        LR = lanePattern[LR_Number];
                        old_tripmeter = m.get_tripmeter();
    //                    pc.printf("line chang tripmeter = %ld\n\r",old_tripmeter);
    //                    fprintf(fp,"%d,%ld\n\r",pattern+10,old_tripmeter);
                    }
                }
                if(mem == mem_count){
 //                   fprintf(fp,"END\n\r\n\r");
 //                   fclose(fp);
                    mem++;
                }                
                if(c.isHalf_Line() == 0){	//クランクで大曲と間違えないように
                	if(c.Center[19] > 30) pattern = 12;
                	if(c.Center[19] < -30) pattern = 13;
                }
                m.run( 100-c.Curve_value(), iServo );
                m.handle( iServo );
                break;
            case 11:
            	d.led_OUT(0x0);
                m.run( 100, iServo );
                m.handle( iServo );
            	break;
// Large curve
            case 12:
            	if(c.Center[19] > 25){
                    m.run( 100, 25 * HANDLE_STEP );
                    m.handle( 25 * HANDLE_STEP );
            	}
            	if(c.Center[19] < 0){
                    m.run( 100, 30 * HANDLE_STEP );
                    m.handle( 30 * HANDLE_STEP );
            	}
            	if(c.Center[19] < 25 && c.Center[19] > 0) pattern = 10;
            break;
            case 13:
            	if(c.Center[19] < -25){
                    m.run( 100, -25 * HANDLE_STEP );
                    m.handle( -25 * HANDLE_STEP );
            	}
            	if(c.Center[19] > 0){
                    m.run( 100, -30 * HANDLE_STEP );
                    m.handle( -30 * HANDLE_STEP );
            	}

            	if(c.Center[19] > -25 && c.Center[19] < 0) pattern = 10;
            break;
// clank
            case 300:
            	m.motor(0,0,0);
                m.handle( iServo );
                clankLR = c.isHalf_Line();
                if(c.isCrank() != 0){
                    pattern = 30;
                    cnt1 = 0;
                }
                if(cntCrank > 250){
            		m.Max_Speed = SPEED;
            		pattern = 10;
                }
            	break;
            case 30:    // Brak;
                d.led_OUT( 0x2);
                m.motor(-100,-100,0);
                c.offset_Center = 0;
            //Left Clank
                if(clankLR == -1) m.handle( -45 * HANDLE_STEP);
            //Right Clank
                if(clankLR == 1) m.handle( 45 * HANDLE_STEP);

                if(c.isEndBlack()){
                    pattern = 31;
                }
                if(cnt1 > 150){
                	m.motor(50,50,0);
                	pattern = 320;
                }
                break;
            case 310:
                if(!c.isCross()){
                    pattern = 31;
                }
            	break;
            case 320:
                if(c.isEndBlack()){
                    pattern = 31;
                }
            	break;
            case 31:    // turn 90
                d.led_OUT( 0x1);
            //Left Clank
                if(clankLR == -1){
                    m.handle( -45 * HANDLE_STEP);
                    m.motor(0,50,0);
   //                 if(c.isOut() == -1)pattern = 32;
                }
            //Right Clank
                if(clankLR == 1){
                    m.handle( 45 * HANDLE_STEP);
                    m.motor(50,0,0);
   //                 if(c.isOut() == 1)pattern = 32;
                }
//              if(c.aa > -10 && c.aa < 10)pattern = 32;
              if(c.aa != -999){
            	  cnt1 = 0;
            	  pattern = 330;
              }
                break;
            case 32:  
                d.led_OUT( 0x0);
                //Left Clank
                    if(clankLR == -1 && (c.aa > -10 && c.aa < 0 && cnt1 > 300)){
                        m.motor(30,30,0);
                    	pattern = 33;
                    }
                //Right Clank
                    if(clankLR == 1 && (c.aa < 10 && c.aa > 0 && cnt1 > 300)){
                        m.motor(30,30,0);
                    	pattern = 33;
                    }
                   break;
            case 330:
                //Left Clank
                    if(clankLR == -1 && (c.Center[19] < 10 && c.Center[19] > 0 && cnt1 > 300)){
                    	pattern = 335;
                    }
                //Right Clank
                    if(clankLR == 1 && (c.Center[19] > -10 && c.Center[19] < 0 && cnt1 > 300)){
                    	pattern = 335;
                    }
                   break;
            case 335:
                d.led_OUT( 0x0);
                //Left Clank
                    if(clankLR == -1 && (c.Center[19] > -10 && c.Center[19] < 0)){
                        m.motor(30,30,0);
                    	pattern = 33;
                    }
                //Right Clank
                    if(clankLR == 1 && (c.Center[19] < 10 && c.Center[19] > 0)){
                       m.motor(30,30,0);
                    	pattern = 33;
                    }
                   break;
            case 33:  //Return Nomal Trace
                d.led_OUT( 0x3);
                m.handle( iServo );
                if(c.aa > -10 && c.aa < 10){
                     m.Max_Speed = SPEED;
                     m.reset_tripmeter();
                     mem++;
//                     saka = 0;
                     pattern = 34;
                }
                break;
            case 34:
            	m.run( 100, iServo );
                m.handle( iServo );
                if(c.cc > -5 && c.cc < 5)pattern = 10;
            	break;
//Lane change
            case 50:
                d.led_OUT( 0x1);
                if(c.isEndBlack() == 1)pattern = 51;
                m.motor(30,30,0);
                m.handle( iServo );               
                if(c.isCrank() != 0){
                    pattern = 30;
                    cnt1 = 0;
                }
                break;
            case 51:
                d.led_OUT(0x02);
 //               l++;
                m.handle( 0 );
                m.motor(-100,-100,0);
                c.offset_Center = 0;
            //Right Lane Change
                if(LR == 1){
                    m.handle( 38 * HANDLE_STEP);
                }
            //Left Lane Change
                else{
                    m.handle( -38 * HANDLE_STEP);
                }
                if(c.All_Black()){
                    pattern = 52;
                }
                break;
            case 52:    // Lean change
                d.led_OUT( 0x3 );
                m.motor(30,30,0);
            //Right Lane Change
                if(LR == 1){
                    m.handle( 38 * HANDLE_STEP);
                    if(c.isOut() == 1)pattern =53;
                }
            //Left Lane Change
                else{
                    m.handle( -38 * HANDLE_STEP);
                    if(c.isOut() == -1)pattern =53;
                }
                if(c.All_Black()){
                    pattern = 53;                    
                    wait(0.3);
                }
                break;
            case 53:    // Lean change
                d.led_OUT( 0x2 );
                m.motor(30,30,0);
                //Right Lane Change
                if(LR == 1) m.handle( 10 * HANDLE_STEP);
                //Left Lane Change
                else m.handle( -10 * HANDLE_STEP);
                 if(c.cc > -10 && c.cc < 10) {
                    pattern = 54;
                }
                break;
            case 54:
                d.led_OUT( 0x1);
                m.run( 50, iServo );
                m.handle( iServo);
                if(c.cc > -5 && c.cc < 5) {
                    pattern = 55;
                    cnt1 = 0;
                }
                break;
            case 55:
                d.led_OUT( 0x0);
 //               m.run( 100-c.Curve_value(), iServo );
                if(c.isBlack() == 1) {
                    m.motor(30,30,0);
                } else {
                    m.run( 70, iServo );
                    m.handle( iServo );
                }
                if( cnt1 > 1000 ){
                    m.reset_tripmeter();
//                    saka = 0;
                    mem++;
                    LR_Number++;
                    pattern =10;
                }
                break;
            case 70://坂道走行
                d.led_OUT(0x00);
                /* レーンチェンジ検知    */
                if(c.isBlack() == 1 ){
                    pattern = 51;     //Lean change
                    old_tripmeter = m.get_tripmeter();
//                    pc.printf("line chang tripmeter = %ld\n\r",old_tripmeter);
//                    fprintf(fp,"%d,%ld\n\r",pattern+10,old_tripmeter);
                }
                m.Max_Speed = 80;
                m.run( 100, iServo );
                m.handle( iServo );
                if(c.isSlop() == TOP || m.get_tripmeter() > 110000){
                     m.Max_Speed = SPEED;
                     pattern = 10;
                }
                break;            
            case 71:
                d.led_OUT(0x1);
                m.Max_Speed = SPEED;
                m.run( 100, iServo );
                m.handle( iServo );
                break;                
            case 90:
                led_status_set( MARK );
                m.motor( 0, 0 ,0);
                break;

            case 99:
                led_status_set( ERROR );
                m.motor( 0, 0 ,0);
                break;
            case 100:
            	m.motor(0,0,0);
            	m.handle( 0 );
            break;
            case 200://1mの走行テストモード
                d.led_OUT( 0x0 );
                m.motor(50,50,0);
                while( m.get_tripmeter() > 110000){
                    d.led_OUT( 0x3 );
                    m.motor(0,0,0);                    
                }
                break; 
            case 1000: //ログの出力
           		d.led_OUT( 0x0 );
           		m.motor(0,0,0);
           		m.handle( 0 );
            	wait(2.0);
          		d.led_OUT( 0x3 );
            	wait(2.0);
            	pattern = 1010;
            	cntLED = 300;
            break;
            case 1010:
                if( d.pushsw_get() ){
                	pattern = 1020;
                	m_number = 0;
                }
                if( cnt1 < cntLED ) {
                    d.led_OUT( 0x1 );
                } else if( cnt1 < cntLED*2 ) {
                    d.led_OUT( 0x2 );
                } else {
                    cnt1 = 0;
                }
            break;
            case 1020:
            	pc.printf("%d,%4d,%4d,%4d,%4d,%4d\r\n",m_number,memory[m_number][0],memory[m_number][1],memory[m_number][2],memory[m_number][3],memory[m_number][4]);
            	m_number++;
            	if(m_number > MAX_MEMORY) pattern = 1030;
            break;
            case 1030:
            	d.led_OUT(0x00);
            break;
            default:
                break;
        }
    }
}

//Interrupt Timer
//------------------------------------------------------------------//
void intTimer( void )
{
    static int  counter = 0;

    cnt0++;
    cnt1++;
    cntCrank++;

    /* Trace by image processing */

    switch( counter++ ) {
        case 0:
            c.change_framebuffer_process();
            break;
        case 12:
            retDevi_A = Standard_Deviation( TempBinary_A, TempDevi_A, 15 );
            ServoControl_process();
            break;
        case 33:
        	if(pattern > 9 && pattern < 1000){
        		memory[m_number][0] = pattern;
        		memory[m_number][1] = c.aa;
        		memory[m_number][2] = c.cc;
        		memory[m_number][3] = c.Center[19];
        		memory[m_number][4] = c.isHalf_Line();
        		m_number++;
        		if(m_number > MAX_MEMORY)m_number = MAX_MEMORY;
         	}
            counter = 0;
            break;
        default:
            break;
    }
    m.motor_drive();
    led_status_process();
}

//LED_Status(on GR-PEACH board) Function for only interrupt
//------------------------------------------------------------------//
void led_status_process( void )
{
    static unsigned long    led_timer;
    int                     led_set;
    int                     on_time;
    int                     off_time;

    /* setting */
    switch( status_set ) {
        case RUN:
            led_set  = LED_GREEN;
            on_time  = 500;
            off_time = 500;
            break;

        case SENSOR:
            led_set  = LED_BLUE;
            on_time  = 50;
            off_time = 50;
            break;

        case MARK:
            led_set  = LED_RED;
            on_time  = 250;
            off_time = 250;
            break;

        case STOP:
            led_set  = LED_RED;
            on_time  = 1;
            off_time = 0;
            break;

        case ERROR:
            led_set  = LED_RED;
            on_time  = 50;
            off_time = 50;
            break;

        default:
            led_set  = LED_OFF;
            on_time  = 0;
            off_time = 1;
            break;
    }

    /* Display */
    led_timer++;
    if( led_timer < on_time ) d.led_RGB( led_set );

    else if( led_timer < ( on_time + off_time ) ) d.led_RGB( LED_OFF );
    else led_timer = 0;
}

//LED_Status(on GR-PEACH board) Function for only interrupt
//------------------------------------------------------------------//
void led_status_set( int set )
{
    status_set = set;
}

//ServoControl_process
//------------------------------------------------------------------//
void ServoControl_process( void )
{
	if(c.cc != -999){
		if(c.isCurve())   iServo = c.CurvePID();
		else                iServo = c.StrightPID();
	}
}

// Standard deviation
//------------------------------------------------------------------//
double Standard_Deviation( unsigned char *data, double *Devi, int items )
{
    int         i;
    double      iRet_A, iRet_C, iRet_D;

    /* A 合計値　平均化 */
    iRet_A = 0;
    for( i = 0; i < items; i++ ) {
        iRet_A += data[i];
    }
    iRet_A /= items;

    /* B 偏差値 */
    for( i = 0; i < items; i++ ) {
        Devi[i] = data[i] - iRet_A;
    }

    /* C 分散 */
    iRet_C = 0;
    for( i = 0; i < items; i++ ) {
        iRet_C += ( Devi[i] * Devi[i] );
    }
    iRet_C /= items;

    /* D 標準偏差 */
    iRet_D = sqrt( iRet_C );

    return iRet_D;
}
//******************************************************************//
// @brief       Interrupt callback function
// @param[in]   int_type    : VDC5 interrupt type
// @retval      None
//*******************************************************************/
static void IntCallbackFunc_Vfield(DisplayBase::int_type_t int_type)
{
    if (vfield_count > 0) {
        vfield_count--;
    }
}

//******************************************************************//
// @brief       Wait for the specified number of times Vsync occurs
// @param[in]   wait_count          : Wait count
// @retval      None
//*******************************************************************/
static void WaitVfield(const int32_t wait_count)
{
    vfield_count = wait_count;
    while (vfield_count > 0) {
        /* Do nothing */
    }
}

//******************************************************************//
// @brief       Interrupt callback function for Vsync interruption
// @param[in]   int_type    : VDC5 interrupt type
// @retval      None
//*******************************************************************/
static void IntCallbackFunc_Vsync(DisplayBase::int_type_t int_type)
{
    if (vsync_count > 0) {
        vsync_count--;
    }
}

//******************************************************************//
// @brief       Wait for the specified number of times Vsync occurs
// @param[in]   wait_count          : Wait count
// @retval      None
//*******************************************************************/
static void WaitVsync(const int32_t wait_count)
{
    vsync_count = wait_count;
    while (vsync_count > 0) {
        /* Do nothing */
    }
}
//------------------------------------------------------------------//
// End of file
//------------------------------------------------------------------//
