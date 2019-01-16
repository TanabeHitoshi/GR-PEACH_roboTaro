#ifndef ISCAMERA_H
#define ISCAMERA_H

#include "mbed.h"
#include "iodefine.h"
#include "math.h"
#include "DisplayBace.h" 
#include "Camera.h"

#define     CV 5

class isCamera : public Camera{
    public:
//        isCamera( void );                    //コンストラクタ
        void Capture(void);                 // カメラの撮影
        int isCross(void);                  // クロスラインのチェック
        int isCrank_F(void);				// クランクの前チェック
        int isCrank(void);					// クランクのチェック
        int isHalf_Line(void);              // ハーフラインのチェック
        int isSideLine(void);               // side white line check
        int Curve_value(void);              //カーブの大きさを計算
        int isBlack_F(void);				// 黒色の前チェック
        int isBlack(void);                  // black check
        int isEndBlack(void);				// end black check
        int All_Black(void);                // All Black cheack
        int isSlop(void);                   // 坂 cheack     
        int isCurve(void);                  // Curve check
        int isOut(void);                    // curse out side line check   
 //       int isOut2(void);                    // curse out side line check     
                   
        int    LR;
        uint8_t Out_Line_binarization[320];
    private:
//        Camera* cam;  
        int curveCounter;                       // curve time 
};
 
#endif
