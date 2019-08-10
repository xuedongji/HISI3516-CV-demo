/******************************************************************************
  A simple program of Hisilicon Hi35xx video encode implementation.
  Copyright (C), 2010-2011, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2011-2 Created
******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "hi_comm_video.h"
#include "mpi_vi.h"
#include "sample_comm.h"

#include "FindCircle_cpp/findCircle.h"
#include "Zq_CV/zq_CV.h"

VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_NTSC;

static pthread_t gs_GetFramePid;

int vi_getYUV(HI_U8* yData,VI_CHN ViChn)
{
	VIDEO_FRAME_INFO_S stFrame;
	HI_VOID* yPtr = HI_NULL;
	HI_S32 len = 0;

	HI_S32 s32ret = HI_MPI_VI_GetFrame(ViChn, &stFrame,-1);
    if (HI_SUCCESS != s32ret)
    {
		printf("get frame err:0x%x\n", s32ret);
		return s32ret;
    }
	// 获取视频帧的Y分量大小
    len = stFrame.stVFrame.u32Width* stFrame.stVFrame.u32Height;
    //printf(" Width: %d  Height: %d\n\n",stFrame.stVFrame.u32Width,stFrame.stVFrame.u32Height);
	yPtr = HI_MPI_SYS_Mmap(stFrame.stVFrame.u32PhyAddr[0], len);

    memcpy(yData, yPtr, len);   // 似乎无法直接对视频帧直接进行处理，只能把数据拷贝出来才能够进行处理，目前遇到似乎都是如此。
    HI_MPI_SYS_Munmap(yPtr, len);

    HI_MPI_VI_ReleaseFrame(ViChn, &stFrame);
    return len;
}

void* GetVIFrameThread(void* p)
{
	unsigned char imgBuf[1920*1080];
	ZqImage img;
	img.height = 1080;
	img.width = 1920;
	img.channels = 1;
	img.imageData = imgBuf;

	HI_U32 u32Depth = 8;
	VI_CHN ViChn = 0;
	HI_S32 s32ret = HI_MPI_VI_SetFrameDepth(ViChn, u32Depth);
    if (HI_SUCCESS != s32ret)
    {
		printf("set frame depth err:0x%x\n", s32ret);
		return HI_NULL;
    }
    static struct timeval node_hc_tik, node_hc_toc;
    static struct timezone node_hc_tz;
	static unsigned long cnt = 0;
	char fileName[30] = {0};
	while(1)
	{
		gettimeofday(&node_hc_tik, &node_hc_tz);
		memset(img.imageData,0,1920*1080);
		vi_getYUV(img.imageData,ViChn);
		//resize
		ZqImage* bmpImgSca = imscale(&img,0.05,0.05);
		//find circle
		XDJ_FindCircleProc(bmpImgSca->imageData,bmpImgSca->width,bmpImgSca->height,0.3,0.2,0.2,120,10);
		//write
		sprintf(fileName,"./data/%ld.bmp",cnt);
		if(imwrite(fileName, bmpImgSca) != 0) printf("write bmp failed\n");
		free(bmpImgSca->imageData);
		++cnt;
		gettimeofday(&node_hc_toc, &node_hc_tz);
		HI_U32 duration = (node_hc_toc.tv_usec - node_hc_tik.tv_usec) + (node_hc_toc.tv_sec - node_hc_tik.tv_sec) * 1000 * 1000;
		printf("\n\n cost time : %lf s\n\n",(double)duration/1000000);
		if (duration < 100000) usleep(100000 - duration);
	}
}



/******************************************************************************
* function : to process abnormal case
******************************************************************************/
void SAMPLE_VENC_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTERM == signo)
    {
        SAMPLE_COMM_ISP_Stop();
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

/******************************************************************************
* function :  H.264@1080p@30fps+H.265@1080p@30fps+H.264@D1@30fps
******************************************************************************/
HI_S32 SAMPLE_VENC_1080P_CLASSIC(HI_VOID)
{
    PIC_SIZE_E enSize[3] = {PIC_HD1080, PIC_HD1080, PIC_D1};
    VB_CONF_S stVbConf;
    SAMPLE_VI_CONFIG_S stViConfig = {0};
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;

    /******************************************
     step  1: init sys variable
    ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONF_S));

    SAMPLE_COMM_VI_GetSizeBySensor(&enSize[0]);
    switch (SENSOR_TYPE)
    {
        case SONY_IMX178_LVDS_5M_30FPS:
        case APTINA_AR0330_MIPI_1536P_25FPS:
        case APTINA_AR0330_MIPI_1296P_25FPS:
            enSize[1] = PIC_VGA;
            break;
        default:
            break;
    }

    stVbConf.u32MaxPoolCnt = 128;

    /*video buffer*/
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSize[0], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 10;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSize[1], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt = 10;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSize[2], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[2].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[2].u32BlkCnt = 10;


    /******************************************
     step 2: mpp system init.
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_1080P_CLASSIC_0;
    }

    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    stViConfig.enViMode   = SENSOR_TYPE;
    stViConfig.enRotate   = ROTATE_NONE;
    stViConfig.enNorm     = VIDEO_ENCODING_MODE_AUTO;
    stViConfig.enViChnSet = VI_CHN_SET_NORMAL;
    stViConfig.enWDRMode  = WDR_MODE_NONE;
    s32Ret = SAMPLE_COMM_VI_StartVi(&stViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_VENC_1080P_CLASSIC_1;
    }

    /******************************************
     step 3.5: get frame
    ******************************************/
    pthread_create(&gs_GetFramePid, 0, GetVIFrameThread,HI_NULL);



    printf("please press twice ENTER to exit this sample\n");
    getchar();
    getchar();

    /******************************************
     step 7: exit process
    ******************************************/


END_VENC_1080P_CLASSIC_1:	//vi stop
    SAMPLE_COMM_VI_StopVi(&stViConfig);
END_VENC_1080P_CLASSIC_0:	//system exit
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}


/******************************************************************************
* function    : main()
* Description : video venc sample
******************************************************************************/
int main(int argc, char* argv[])
{
    HI_S32 s32Ret;

    signal(SIGINT, SAMPLE_VENC_HandleSig);
    signal(SIGTERM, SAMPLE_VENC_HandleSig);

    s32Ret = SAMPLE_VENC_1080P_CLASSIC();

    if (HI_SUCCESS == s32Ret)
    { printf("program exit normally!\n"); }
    else
    { printf("program exit abnormally!\n"); }
    pthread_join(gs_GetFramePid, 0);
    exit(s32Ret);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
