#include "isCamera.h"
#include "mbed.h"
#include "iodefine.h"
#include "Camera.h"

// Capture camera image
void isCamera::Capture(void)
{
        image_Extraction_Y();
        image_thinning_out();
        Binarization();
        Full_Binarization();
        WhiteLineWideCenter();
        LeastSquare();
}
//--------------------------------------------------------------------//

//cheack clank
// 0-> non  1-> clank
int isCamera::isCross(void)
{
    int y;
    int count;
    count = 0;
    for(y=5; y < 35; y++) {
        if(Width[y] > 45) {
            count++;
        }
     }
    if(count > 0)return 1;
    else return 0;
}
//--------------------------------------------------------------------//

//cheack Crank
// 0-> non  1-> crank
int isCamera::isCrank_F(void)
{
    int y,cnt;
    int center_X = 40;
    int L[10],R[10];
    int sideWide[10];
    int L_side,R_side;
    int count,Ccnt;

    LR = 0;L_side = 0;R_side = 0;
    for(y = 0; y < 5; y++){
    	R[y] = 0; L[y] = 0;
		if(White2[y] < 30 && Image_binarization2[center_X][y] == 0){
			for(cnt = 20; cnt < 40; cnt++){
				 if(R[y] == 0){
					if(Image_binarization2[center_X + cnt][y] == 1)R[y] = cnt;     //右に発見
				 }
				 if(L[y] == 0){
					if(Image_binarization2[center_X - cnt][y] == 1)L[y] = cnt;      //左に発見
				 }
				}
		}

		sideWide[y] = L[y] - R[y];
		if(sideWide[y] > 15 )L_side++;
		if(sideWide[y] < -15 )R_side++;
		if(R_side > 1)LR = 1;
		if(L_side > 1)LR = 1;
//		printf("%2d L = %2d    R = %2d  R-L %4d\r\n",y,L[y],R[y],sideWide[y]);
    }

    count = 0; Ccnt = 0;
    for(y=5; y < 10; y++) {
        if(White2[y] >= 35 ) {
        	count++;
        }
        if( Image_binarization2[center_X][y] == 1){
        	Ccnt++;
        }
    }
    if(count > 0 && LR == 1 &&  Ccnt > 3)return 1;
    else return 0;
}
//--------------------------------------------------------------------//
//cheack Crank
// 0-> non  1-> crank
int isCamera::isCrank_M(void)
{
    int y,cnt;
    int center_X = 40;
    int L[10],R[10];
    int sideWide[10];
    int L_side,R_side;
    int count,Ccnt;

    LR = 0;L_side = 0;R_side = 0;cnt = 0;
    for(y = 0; y < 5; y++){
    	R[y] = 0; L[y] = 0;
		if(White[y] == 0){
			cnt++;
		}
    }
//	printf("cnt = %2d\r\n",cnt);

    count = 0; Ccnt = 0;
    for(y=5; y < 10; y++) {
        if(White[y] >= 35 ) {
        	count++;
        }
        if( Image_binarization[center_X][y] == 1){
        	Ccnt++;
        }
    }
    if(count > 0 && cnt > 0 &&  Ccnt > 3)return 1;
    else return 0;
}
//--------------------------------------------------------------------//

//cheack Crank
// 0-> non  1-> right crank  -1-> left crank
int isCamera::isCrank(void)
{
    int hl;

    hl = 0;
    if(BlackCount > 2){
//    	if(BlackPlace < 10){
    		hl = isHalf_Line();
//    	}
    }
    return hl;
}
//--------------------------------------------------------------------//

//cheack half
// 0-> non  1-> right half  -1-> left half
int isCamera::isHalf_Line(void)
{
    int y;
    int count_r,count_l;
    count_r = count_l = 0;
    for(y=5; y < 35; y++) {
        if(Width[y] > (25 + (y-5)/6)) {
            if(Center[y]>0)
                count_r++;
            else
                count_l++;
        }
    }
    if(count_l - count_r > 1)return -1;
    else if(count_l - count_r < -1)return 1;
    else return 0;
}
//--------------------------------------------------------------------//

//side white line check
// 0-> non  1-> right   -1-> left
int isCamera::isSideLine(void)
{
    int y,cnt;
    int center_X = 40;
    int L[10],R[10];
    int sideWide[10];
    int L_side,R_side;

    LR = 0;L_side = 0;R_side = 0;
  //  if(aa > 3 && aa < -3){
    for(y = 0; y < 10; y++){
    	R[y] = 0; L[y] = 0;
		if(White2[y] < 30 && Image_binarization2[center_X][y] == 0){
			for(cnt = 10; cnt < 40; cnt++){
				 if(R[y] == 0){
					if(Image_binarization2[center_X + cnt][y] == 1)R[y] = cnt;     //右に発見
				 }
				 if(L[y] == 0){
					if(Image_binarization2[center_X - cnt][y] == 1)L[y] = cnt;      //左に発見
				 }
			}
		}
		sideWide[y] = L[y] - R[y];
		if(sideWide[y] > 15 )L_side++;
		if(sideWide[y] < -15 )R_side++;
		if(R_side > 3)LR = -1;
		if(L_side > 3)LR = 1;
		printf("%2d L = %2d    R = %2d  R-L %4d\r\n",y, L[y],R[y],sideWide[y]);

    }
//    }
//    printf("L_side = %d R_side = %d \r\n",L_side,R_side);
    return LR;
}
//--------------------------------------------------------------------//

//cheack Front Black
// 0-> non  1-> Black
int isCamera::isBlack_F(void)
{
    int y,cnt;
    int center_X = 40;
    int L[10],R[10];
    int sideWide[10];
    int L_side,R_side;
//    int count;

    LR = 0;L_side = 0;R_side = 0;
    for(y = 0; y < 10; y++){
    	R[y] = 0; L[y] = 0;
		if(White2[y] < 10 && Image_binarization2[center_X][y] == 0){
			for(cnt = 20; cnt < 40; cnt++){
				 if(R[y] == 0){
					if(Image_binarization2[center_X + cnt][y] == 1)R[y] = cnt;     //右に発見
				 }
				 if(L[y] == 0){
					if(Image_binarization2[center_X - cnt][y] == 1)L[y] = cnt;      //左に発見
				 }
				}
		}

		sideWide[y] = L[y] - R[y];
		if(sideWide[y] > 15 )L_side++;
		if(sideWide[y] < -15 )R_side++;
		if(R_side > 7)LR = 1;
		if(L_side > 7)LR = 1;
//		printf("%2d L = %2d    R = %2d  R-L %4d\r\n",y,L[y],R[y],sideWide[y]);
    }
    if(LR == 1)return 1;
    else return 0;
}
//--------------------------------------------------------------------//

//cheack Black
// 0 -> non black  1 -> black
int isCamera::isBlack(void)
{
    if(BlackCount > 25)
        return 1;
    else
        return 0;
}
//--------------------------------------------------------------------//
//cheack EndBlack
// 0 -> non black  1 -> black
int isCamera::isEndBlack(void)
{
    int x;
    int count_w;

    count_w = 0;
    for(x = 0;x < 80;x++){
        if(Image_binarization[x][39] == 1)
            count_w++;
    }
    if(count_w < 5)
        return 1;
    else
        return 0;
}
//--------------------------------------------------------------------//

//cheack All Black
// 0 -> non black  1 -> black
int isCamera::All_Black(void)
{
    int x,y;
    int count_w;
    int center_X = 40;
    
    count_w = 0;
/*
    for(x = 0;x < 80;x++){
        if(Image_binarization[x][39] == 1)
            count_w++;
    }
*/
    for(y = 30;y < 40; y++){
    	if(White[y] > 10 && Image_binarization[center_X][y] == 1){
    		count_w++;
    	}
    }
    if(count_w > 3 && BlackCount > 30)
        return 1;
    else
        return 0;
}
//--------------------------------------------------------------------//

// The size of the curve
int isCamera::Curve_value(void)
{
    int s;
    if(aa!= -999) {
    	isCurve();
        if(aa > 0)
            s = aa*CV - curveCounter*50;
        else
            s = -aa*CV - curveCounter*50;
        if(s < 0) s = 0;
    } else {
        s = 0;
    }
    return s;
}
//--------------------------------------------------------------------//

//slop check
// 0-> non  1-> updown  -1-> top  
//--------------------------------------------------------------------//
int isCamera::isSlop(void)
{
    int y,cnt;
    int center_X = 40;
    int L[10],R[10];
    int bothWide[10];
    static int pre_bothWide[10];
    int bothCount;

    LR = 0;bothCount = 0;
  //  if(aa > 3 && aa < -3){
    for(y = 0; y < 10; y++){
    	R[y] = 0; L[y] = 0;
		for(cnt = 10; cnt < 40; cnt++){
			 if(R[y] == 0){
				if(Image_binarization2[center_X + cnt][y] == 1)R[y] = cnt;     //右に発見
			 }
			 if(L[y] == 0){
				if(Image_binarization2[center_X - cnt][y] == 1)L[y] = cnt;      //左に発見
			 }
		}

		bothWide[y] = L[y] + R[y];
		if(pre_bothWide[y] - bothWide[y] > 5) bothCount++;
		pre_bothWide[y] = bothWide[y];

//		printf("%2d L = %2d    R = %2d  R+L %4d\r\n",y,L[y],R[y],bothWide[y]);

    }
//    }
//    printf("L_side = %d R_side = %d \r\n",L_side,R_side);
    return LR;
}
//--------------------------------------------------------------------//

//ckeck Curve
// 0 -> non Curve  1 ->Curve
int isCamera::isCurve(void)
{
    if( aa > 3 || aa < -3) {
        curveCounter++;
        return 1;
    } else {
        curveCounter = 0;
        return 0;
    }
}
//--------------------------------------------------------------------//

//Out Side Line check
// 0-> non  1-> right   -1-> left
int isCamera::isOut(void)
{
    if(Image_binarization[5][5]) return -1;
    else if(Image_binarization[34][5]) return 1;
    else return 0;
}

//--------------------------------------------------------------------//
