#include "Camera.h"
#include "mbed.h"
#include "iodefine.h"

Serial      p(USBTX, USBRX);

Camera::Camera()
{
    write_buff_addr = FrameBuffer_Video_A;
    save_buff_addr  = FrameBuffer_Video_B;
}

//Change FrameBuffer Process
void Camera::change_framebuffer_process( void )
{
    DisplayBase::graphics_error_t error;
    DisplayBase Display;

    /* Change address buffer */
    if (write_buff_addr == FrameBuffer_Video_A) {
        write_buff_addr = FrameBuffer_Video_B;
        save_buff_addr  = FrameBuffer_Video_A;
    } else {
        write_buff_addr = FrameBuffer_Video_A;
        save_buff_addr  = FrameBuffer_Video_B;
    }

    /* Change write buffer */
    error = Display.Video_Write_Change(
                VIDEO_INPUT_CH,
                write_buff_addr,
                VIDEO_BUFFER_STRIDE);
    if (error != DisplayBase::GRAPHICS_OK) {
        printf("Line %d, error %d\n", __LINE__, error);
        while (1);
    }
}
//------------------------------------------------------------------//

//Extraction of the Y component
void Camera::image_Extraction_Y( void )
{
    long int       Xp;
    long int       Yp;
    int            x;
    int            y;

    for( Yp = 0, y = 0; Yp < 240; Yp+=1, y++ ) {
        for( Xp = 0, x = 0; Xp < 640; Xp+=4, x+=2 ) {
            /*Y0*/Raw_Y_component[x][y] = save_buff_addr[(Xp+0)+(640*Yp)];
            /*Y1*/
            Raw_Y_component[x+1][y] = save_buff_addr[(Xp+2)+(640*Yp)];
        }
    }
}
//------------------------------------------------------------------//

//Extraction of the Y component viewer
void Camera::image_Extraction_Y_view( void )
{
    int x;
    int y;
    for(y=0; y<240; y++) {
        p.printf( "y aix = %d\n\r", y);
        for(x=0; x<320; x++) {
            p.printf("%d,",Raw_Y_component[x][y]);
        }
        p.printf( "\n\r\n\r\n\r" );
    }
}
//------------------------------------------------------------------//

//Thinning out the image
void Camera::image_thinning_out(void)
{
    int            x;
    int            y;

    /* ライントレース用のデータ */
    for(y = 0; y < 40; y++) {
        Max[y] = 0;
        Min[y] = 255;
    }
    for(y = 0; y < 40; y++) {
        for(x = 0; x < 80; x++) {
            Image_thinning_out[x][y] = Raw_Y_component[x*3+40][y*2+F_start];
            if(Image_thinning_out[x][y] > Max[y]) Max[y] = Image_thinning_out[x][y];
            if(Image_thinning_out[x][y] < Min[y]) Min[y] = Image_thinning_out[x][y];
        }
    }
    /* アウトラインを検出用データ */
    for(y = 0; y < 10; y++) {
        Max2[y] = 0;
        Min2[y] = 255;
    }
    for(y = 0; y < 10; y++) {
        for(x = 0; x < 80; x++) {
            Image_thinning_out2[x][y] = Raw_Y_component[x*4+0][y*2+10];
            if(Image_thinning_out2[x][y] > Max2[y]) Max2[y] = Image_thinning_out2[x][y];
            if(Image_thinning_out2[x][y] < Min2[y]) Min2[y] = Image_thinning_out2[x][y];
        }
    }

}
//------------------------------------------------------------------//

//Thinning out the image viewer
void Camera::image_thinning_out_view( void )
{
    int x;
    int y;
    for(y=0; y<40; y++) {
        p.printf( "y aix = %d\n\r", y);
        for(x=0; x<80; x++) {
            p.printf("%d,",Image_thinning_out[x][y]);
        }
        p.printf( "\n\r" );
    }
    p.printf( "\n\r" );
}
//------------------------------------------------------------------//

// Bainarization
void Camera::Binarization(void)
{
    int x;
    int y;

     for(y = 0; y < 10; y++) {
        //Determine the threshold
        Ave2[y] = (Max2[y] + Min2[y]) * 3 / 5;
        // number of White  to zero
        White2[y] = 0;
        if(Max2[y] > 200) {
            //When white is a straight line
            if(Min2[y] > 200) {
                for(x = 0; x < 80; x++) {
                    Image_binarization2[x][y] = 1;
                    White2[y]++;
                }
            } else {
                for(x = 0; x < 80; x++) {
                    if( Image_thinning_out2[x][y] >  Ave2[y]) {
                        Image_binarization2[x][y] = 1;   //white
                        White2[y]++;
                    } else {
                        Image_binarization2[x][y] = 0;   //black
                    }
                }
            }
            if(White2[y] == 0)F_BlackCount++;
        } else {
            //When black is a straight line
            F_BlackCount++;
            for(x = 0; x < 80; x++) {
                Image_binarization2[x][y] = 0;
            }
        }
    }

    BlackCount = 0;

    for(y = 0; y < 40; y++) {
        //Determine the threshold
        Ave[y] = (Max[y] + Min[y]) * 7 / 10;
        // number of White  to zero
        White[y] = 0;
        if(Max[y] > 200) {
            //When white is a straight line
            if(Min[y] > 200) {
                for(x = 0; x < 80; x++) {
                    Image_binarization[x][y] = 1;
                    White[y]++;
                }
            } else {
                for(x = 0; x < 80; x++) {
                    if( Image_thinning_out[x][y] >  Ave[y]) {
                        Image_binarization[x][y] = 1;   //white
                        White[y]++;
                    } else {
                        Image_binarization[x][y] = 0;   //black
                    }
                }
            }
            if(White[y] == 0){
            	BlackPlace = y;
            	BlackCount++;
            }
        } else {
            //When black is a straight line
        	BlackPlace = y;
        	BlackCount++;
            for(x = 0; x < 80; x++) {
                Image_binarization[x][y] = 0;
            }
        }
    }

}
//------------------------------------------------------------------//

// Bainarization viewer
void Camera::Binarization_view(void)
{
    int x;
    int y;
    p.printf("\033[%dA" ,40);
    for(y=0; y<40; y++) {
        for(x=0; x<80; x++) {
            if(Image_binarization[x][y] == 1)
                p.printf("1");
            else
                p.printf(" ");
        }
        p.printf( "Max%3d Min%3d Ave%3d Width%3d Center%3d  White%3d\n\r",Max[y],Min[y],Ave[y],Width[y],Center[y],White[y]);
    }
//    p.printf( "aa%3d bb%3d cc%3d\n\r",aa,bb,cc);
}
//------------------------------------------------------------------//
// Bainarization viewer
void Camera::Binarization2_view(void)
{
    int x;
    int y;
    p.printf("\033[%dA" ,40);
    for(y=0; y<10; y++) {
        for(x=0; x<80; x++) {
            if(Image_binarization2[x][y] == 1)
                p.printf("1");
            else
                p.printf(" ");
        }
        p.printf( "Max%3d Min%3d Ave%3d White%3d\n\r",Max2[y],Min2[y],Ave2[y],White2[y]);
    }
}
//------------------------------------------------------------------//
// Full Bainarization
void Camera::Full_Binarization(void)
{
    int x;
    int y;
    /* 最大値、最小値の計算 */
    for(y = 0; y < 240; y++) {
        for(x = 0; x < 320; x++) {
            if(Raw_Y_component[x][y] > Full_Max[y]) Full_Max[y] = Raw_Y_component[x][y];
            if(Raw_Y_component[x][y] < Full_Min[y]) Full_Min[y] = Raw_Y_component[x][y];
        }
    }
    
    for(y = 0; y < 240; y++) {
        //Determine the threshold
        Full_Ave[y] = (Full_Max[y] + Full_Min[y]) * 2 / 3;
        // number of White  to zero
        Full_White[y] = 0;
        if(Full_Max[y] > 20) {
            //When white is a straight line
            if(Full_Min[y] > 200) {
                for(x = 0; x < 320; x++) {
                    Full_binarization[x][y] = 1;
                    Full_White[y]++;
                }
            } else {
                for(x = 0; x < 320; x++) {
                    if( Raw_Y_component[x][y] >  Full_Ave[y]) {
                        Full_binarization[x][y] = 1;   //white
                        Full_White[y]++;
                    } else {
                        Full_binarization[x][y] = 0;   //black
                    }
                }
            }
         } else {
            //When black is a straight line
            BlackCount++;
            for(x = 0; x < 320; x++) {
                Full_binarization[x][y] = 0;
            }
        }
    }
}
//------------------------------------------------------------------//

// Bainarization viewer
void Camera::Full_Binarization_view(void)
{
    int x;
    int y;
    Full_Binarization();
    p.printf("\033[%dA" ,240);
    for(y = 0; y < 240; y++) {
        for(x = 0; x < 320; x++) {
            if(Full_binarization[x][y] == 1)
                p.printf("1");
            else
                p.printf(" ");
        }
        p.printf( "%3d: Ave%d  White%3d\n\r",y,Ave[y/6],Full_White[y]);
    }
}
//------------------------------------------------------------------//

// Bainarization viewer
void Camera::Full_Raw_view(void)
{
    int x;
    int y;
    Full_Binarization();
    p.printf("\033[%dA" ,240);
    for(y = 0; y < 240; y++) {
        for(x = 0; x < 320; x++) {
            p.printf("%d,",Raw_Y_component[x][y]);
        }
        p.printf( "%3d: Ave%d  White%3d\n\r",y,Ave[y/6],Full_White[y]);
    }
}
//------------------------------------------------------------------//

//Meauser the width of the line and Center
void Camera::WhiteLineWideCenter(void)
{
    int x;
    int y;
    int t;
    int Lsensor01;
    int Rsensor01;
    int Lsensor02;
    int Rsensor02;
    int center01;
    int center02;
    int abscenter01;
    int abscenter02;
    int width01;
    int width02;

    //before    nihongo:temae
    Lsensor01 = 0;
    Rsensor01 = 80;
    Lsensor02 = 0;
    Rsensor02 = 80;
    y = 39;
    t = 0;
    for(x=0; x<80; x++) {
        if(t==0) {
            if( Image_binarization[x][y] ) {                 /*The first white from left*/
                Lsensor01 = x;
                t = 1;
            }
        } else if(t==1) {
            if( !Image_binarization[x][y] ) {                /*The first white from right*/
                Rsensor01 = x;
                t = 2;
            }
        }
    }
    Center[y] = (Lsensor01 + Rsensor01)/2 - 40;
    Width[y] = Rsensor01 - Lsensor01;
    for(y=38; y>=0; y--) {
        Lsensor01 = 0;
        Rsensor01 = 80;
        Lsensor02 = 0;
        Rsensor02 = 80;
        t = 0;
        if(Center[y+1] < 0) {
            for(x=0; x<80; x++) {
                if(t==0) {
                    if( Image_binarization[x][y] ) {                 /*The first white from left*/
                        Lsensor01 = x;
                        t = 1;
                    }
                } else if(t==1) {
                    if( !Image_binarization[x][y] ) {                /*The first white from right*/
                        Rsensor01 = x;
                        t = 2;
                    }
                } else if(t==2) {
                    if( Image_binarization[x][y] ) {                 /*The first white from left*/
                        Lsensor02 = x;
                        t = 3;
                    }
                } else if(t==3) {
                    if( !Image_binarization[x][y] ) {                /*The first white from right*/
                        Rsensor02 = x;
                        t = 4;
                    }
                }
            }
        } else {
            for(x=79; x>=0; x--) {
                if(t==0) {
                    if( Image_binarization[x][y] ) {                 /*The first white from left*/
                        Rsensor01 = x;
                        t = 1;
                    }
                } else if(t==1) {
                    if( !Image_binarization[x][y] ) {                /*The first white from right*/
                        Lsensor01 = x;
                        t = 2;
                    }
                } else if(t==2) {
                    if( Image_binarization[x][y] ) {                 /*The first white from left*/
                        Rsensor02 = x;
                        t = 3;
                    }
                } else if(t==3) {
                    if( !Image_binarization[x][y] ) {                /*The first white from right*/
                        Lsensor02 = x;
                        t = 4;
                    }
                }
            }
        }
//        p.printf("t %d Rsensor %d   Lsensor %d\n\r",t,Rsensor,Lsensor)
        center01 = (Lsensor01 + Rsensor01)/2 - 40;
        center02 = (Lsensor02 + Rsensor02)/2 - 40;
        if(White[y] > 4) {
            width01 = Rsensor01 - Lsensor01;
            if(Lsensor02 != 0)  width02 = Rsensor02 - Lsensor02;
            else                width02 = 0;
        } else {
            width01 = 0;
            width02 = 0;
        }
        if((width01 != 0) && (width02 == 0)) {
            Center[y] = center01;
            Width[y] = width01;
        } else if((width01 != 0) && (width02 != 0)) {
            if(center01 - Center[y+1] > 0)abscenter01 = center01 - Center[y+1];
            else                       abscenter01 = -1 * (center01 - Center[y+1]);
            if(center02 - Center[y+1] > 0)abscenter02 = center02 - Center[y+1];
            else                       abscenter02 = -1 * (center02 - Center[y+1]);
            if(abscenter01 < abscenter02) {
                Center[y] = center01;
                Width[y] = width01;
            } else {
                Center[y] = center02;
                Width[y] = width02;
            }
        } else {
            Center[y] = 0;
            Width[y] = 0;
        }
    }
}
//------------------------------------------------------------------//

//Least square method control
void Camera::LeastSquare(void)
{
    int y,count;
    int A,B,C,D,E;

    count = 0;
    A = B = C = D = E = 0;
    for(y = 0; y < 40; y++) {
        if(Width[y] > 5) {
            count++;
            A += Center[y] *Center[y];
            B += (40-y)*(40-y);
            C += Center[y];
            D += Center[y] * (40-y);
            E += (40-y);
        }
    }
    if(count > 20) {
        aa = -Ka*(count*D - C*E) / (count*B - E*E);
        bb = (B*C - D*E) / (count*B - E*E);
        cc = 20 * (count*D - C*E) / (count*B - E*E) + (B*C - D*E) / (count*B - E*E);
    } else {
        aa = bb = cc = -999;
    }
}
//------------------------------------------------------------------//

// Calculation of PID
int Camera::CurvePID(void)
{
    int           center;

    static int    h;

    if(aa != -999) {

        if(aa > 10 || aa < -10) {
            center = Center[19];            // Out side white line
        } else {
            center = cc;                   // slop and intercept
        }

//        center = cc;
//        center = Center[19];
        iCenter +=  center - preCenter;
        h = center * Kp + iCenter * Ki + (center - preCenter) * Kd;
        preCenter = center;
        prepreCenter = preCenter;
    } else {
        center = Center[39];                    // slop and intercept
        iCenter +=  center - preCenter;
        h = center * Kp + iCenter * Ki + (center - preCenter) * Kd;
        preCenter = center;
        prepreCenter = preCenter;
    }
    return h;
}
//------------------------------------------------------------------//

// Calculation of PID
int Camera::StrightPID(void)
{
    int           center;

    static int    h;
    if(aa != -999) {
        center = cc + offset_Center;                   // slop and intercept

        iCenter +=  center - preCenter;
        h = center * lp + iCenter * li + (center - preCenter) * ld;
        preCenter = center;
        prepreCenter = preCenter;
    } else {
        center = Center[38] + offset_Center;                    // slop and intercept
        iCenter +=  center - preCenter;
        h = center * lp + iCenter * li + (center - preCenter) * ld;
        preCenter = center;
        prepreCenter = preCenter;
    }
    return h;
}
//------------------------------------------------------------------//

//control by finger
// -1 -> left  0 -> center  1 -> right
int Camera::Finger_check()
{
    int x;
    int Lcount,Rcount;

    //Left serch
    x = 0;
    Lcount = 0;
    while(Image_thinning_out[x][10] > 100) {
        if(Image_thinning_out[x][10] < Max[10]-50) Lcount++;
        x++;
    }
//    p.printf("Lcount %d\n\r",Lcount);
    //Right serch
    x = 79;
    Rcount = 0;
    while(Image_thinning_out[x][10] > 100) {
        if(Image_thinning_out[x][10] < Max[10]-50) Rcount++;
        x--;
    }
//    p.printf("Rcount %d\n\r",Rcount);
    return Rcount - Lcount;
}
//------------------------------------------------------------------//


