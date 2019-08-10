#ifndef __FIND_CIRCLE_H__
#define __FIND_CIRCLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define PI 3.1415926

void XDJ_FindCircleProc(unsigned char* data,int width,int height,
		float WH_ERR,float CR_ERR,float CH_ERR,int MY_BINARY_TH,int SCALE_TH);

#ifdef __cplusplus
}
#endif


#endif
