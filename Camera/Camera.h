#ifndef CAMERA_H
#define CAMERA_H

#include "mbed.h"
#include "iodefine.h"
#include "math.h"
#include "DisplayBace.h" 

#define     Ka 30
//37000
#define     Kp 15.0
//#define     Ki 0.155
#define     Ki 0.0
#define     Kd 30.0

#define     lp 5.0
#define     li 0.0
#define     ld 5.0

#define     UPDOWN  1
#define     TOP     -1
#define     non     0

#define     IMAGE_LINE          160     /* Y Line No                */
 //Define(NTSC-Video)
//------------------------------------------------------------------//
#define VIDEO_INPUT_CH         (DisplayBase::VIDEO_INPUT_CHANNEL_0)
#define VIDEO_INT_TYPE         (DisplayBase::INT_TYPE_S0_VFIELD)
#define DATA_SIZE_PER_PIC      (2u)

/*! Frame buffer stride: Frame buffer stride should be set to a multiple of 32 or 128
    in accordance with the frame buffer burst transfer mode. */
#define PIXEL_HW               (320u)  /* QVGA */
#define PIXEL_VW               (240u)  /* QVGA */

 /*! Frame buffer stride: Frame buffer stride should be set to a multiple of 32 or 128
    in accordance with the frame buffer burst transfer mode. */
#define PIXEL_HW               (320u)  /* QVGA */
#define PIXEL_VW               (240u)  /* QVGA */
#define VIDEO_BUFFER_STRIDE    (((PIXEL_HW * DATA_SIZE_PER_PIC) + 31u) & ~31u)
#define VIDEO_BUFFER_HEIGHT    (PIXEL_VW)

class Camera {
public:
    Camera(void);
    
    void change_framebuffer_process( void );            //Change FrameBuffer Process
    
    void image_Extraction_Y(void);                      //Extraction of Y component
    void image_Extraction_Y_view(void);                 //Extraction of Y component viewer
    
    void image_thinning_out(void);                       //Thinning out the image 
    void image_thinning_out_view( void );               //Thinning out the image viewer
 
    void Binarization(void);                            // Binarization
    void Binarization_view(void);                       // Binarization viewer
    void Binarization2_view(void);                       // Binarization viewer
    void Full_Binarization(void);                            // Binarization
    void Full_Binarization_view(void);                       // Binarization viewer
   void Full_Raw_view(void);
    void WhiteLineWideCenter(void);                           //Meauser the width of the line
    
    void LeastSquare(void);                             //Least square method control 
    
    int CurvePID(void);                                      // Calculation of PID
    int StrightPID(void);                                      // Calculation of PID


 
//    int isSideLine(void);                               // side white line check
 
   
    int Finger_check(void);                             //control by finger
    
    volatile int            Left_sensor;
    volatile int            Right_sensor;
    //Globle(NTSC-video)
    uint8_t FrameBuffer_Video_A[VIDEO_BUFFER_STRIDE * VIDEO_BUFFER_HEIGHT];  //16 bytes aligned!;
    uint8_t FrameBuffer_Video_B[VIDEO_BUFFER_STRIDE * VIDEO_BUFFER_HEIGHT];  //16 bytes aligned!;
//    uint8_t FrameBuffer_Video_A[VIDEO_BUFFER_STRIDE * VIDEO_BUFFER_HEIGHT]__attribute((section("NC_BSS"),aligned(16)));  //16 bytes aligned!;
//    uint8_t FrameBuffer_Video_B[VIDEO_BUFFER_STRIDE * VIDEO_BUFFER_HEIGHT]__attribute((section("NC_BSS"),aligned(16)));  //16 bytes aligned!;
    uint8_t * write_buff_addr;
    uint8_t * save_buff_addr;
    
    uint8_t Raw_Y_component[320][240];      //Rawデータ
    uint8_t Image_thinning_out[80][40];
    uint8_t Image_thinning_out2[80][20];
    uint8_t Image_binarization[80][40];
    uint8_t Image_binarization2[80][10];
    uint8_t Full_binarization[320][240];
    uint8_t Max[40],Min[40],Ave[40];
    uint8_t Max2[40],Min2[40],Ave2[40];
    uint8_t Full_Max[240],Full_Min[240],Full_Ave[240];
    uint8_t Width[40];                      //白線幅
    uint8_t Full_White[240],White[40];
    int Center[40];
    int aa;                                 //slope
    int bb;                                 //intercept
    int cc;                               //center value
    int BlackCount;                      // Black count
    int offset_Center;                        // Camera x axis offset

private:  
    int    iCenter;
    int    preCenter;
    int    prepreCenter;
    int    LR;

};
 
#endif
