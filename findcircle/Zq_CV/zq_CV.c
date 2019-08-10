#include "zq_CV.h"  
#include <stdio.h>  
#include <stdlib.h>
#include <math.h>
  
ZqImage* imread(char* path)
{  
    ZqImage* bmpImg;
    FILE* pFile;
    BMPFileHeader bmpFileHeader;
    BMPInfoHeader bmpInfoHeader;
    int channels = 1;
    int width = 0;
    int height = 0;
    int step = 0;
    int offset = 0;
    unsigned char pixVal;//像素指针
    RgbQuad* quad;//BMP图像颜色表
    int i, j, k;
  
    bmpImg = (ZqImage*)malloc(sizeof(ZqImage));

    if ((pFile = fopen(path, "rb")) == NULL)
    {
		printf("Cann't open the file!\n");
        free(bmpImg);
        return NULL;
    }

	//读取文件头
    fread(&bmpFileHeader, sizeof(BMPFileHeader), 1, pFile);
    /*
    printf("=========================================== \n");
    printf("BMP文件头信息：\n");
	printf("文件标识符 ：0X%X \n", bmpFileHeader.bfType);
    printf("文件大小：%d \n", bmpFileHeader.bfSize);
    printf("保留字：%d \n", bmpFileHeader.bfReserved1);
    printf("保留字：%d \n", bmpFileHeader.bfReserved2);
    printf("位图数据偏移字节数：%d \n", bmpFileHeader.bfOffBits);
    */
	//读取信息头
	fread(&bmpInfoHeader, sizeof(BMPInfoHeader), 1, pFile);
	/*
    printf("=========================================== \n");
    printf("BMP文件信息头\n");
    printf("结构体长度：%d \n", bmpInfoHeader.biSize);
    printf("位图宽度：%d \n", bmpInfoHeader.biWidth);
    printf("位图高度：%d \n", bmpInfoHeader.biHeight);
    printf("位图平面数：%d \n", bmpInfoHeader.biPlanes);
    printf("颜色位数：%d \n", bmpInfoHeader.biBitCount);
    printf("压缩方式：%d \n", bmpInfoHeader.biCompression);
    printf("实际位图数据占用的字节数：%d \n", bmpInfoHeader.biSizeImage);
    printf("X方向分辨率：%d \n", bmpInfoHeader.biXPelsPerMeter);
    printf("Y方向分辨率：%d \n", bmpInfoHeader.biYPelsPerMeter);
    printf("使用的颜色数：%d \n", bmpInfoHeader.biClrUsed);
    printf("重要颜色数：%d \n", bmpInfoHeader.biClrImportant);
    printf("=========================================== \n");
	*/
    if (bmpInfoHeader.biBitCount == 8)
    {
        printf("\n该图像为灰度图! \n");
        channels = 1;
        width = bmpInfoHeader.biWidth;
        height = bmpInfoHeader.biHeight;
		//windows规定每一个扫描行为4的倍数，不足补0
        offset = (channels*width)%4;
        if (offset != 0)
        {
            offset = 4 - offset;
        }
        bmpImg->width = width;
        bmpImg->height = height;
        bmpImg->channels = 1;
		//分配图像空间
        bmpImg->imageData = (unsigned char*)malloc(sizeof(unsigned char)*width*height);
		//迭代步长
        step = channels*width;
		//读取图像颜色表
        quad = (RgbQuad*)malloc(sizeof(RgbQuad)*256);
        fread(quad, sizeof(RgbQuad), 256, pFile);
        free(quad);
		//读取灰度图像数据
        for (i=0; i<height; i++)
        {
            for (j=0; j<width; j++)
            {
                fread(&pixVal, sizeof(unsigned char), 1, pFile);
                bmpImg->imageData[(height-1-i)*step+j] = pixVal;
            }
            if (offset != 0)
            {
                for (j=0; j<offset; j++)
                {
                    fread(&pixVal, sizeof(unsigned char), 1, pFile);
                }
            }
        }
    }
    else if (bmpInfoHeader.biBitCount == 24)
    {
        printf("\n该图像为彩色图! \n");
        channels = 3;
        width = bmpInfoHeader.biWidth;
        height = bmpInfoHeader.biHeight;
        bmpImg->width = width;
        bmpImg->height = height;
        bmpImg->channels = 3;
        bmpImg->imageData = (unsigned char*)malloc(sizeof(unsigned char)*width*3*height);
        step = channels*width;
		//windows规定每一个扫描行为4的倍数，不足补0
        offset = (channels*width)%4;
        if (offset != 0)
        {
            offset = 4 - offset;
        }
		//读取彩色图像数据
        for (i=0; i<height; i++)
        {
            for (j=0; j<width; j++)
            {
                for (k=0; k<3; k++)
                {
                    fread(&pixVal, sizeof(unsigned char), 1, pFile);
                    bmpImg->imageData[(height-1-i)*step+j*3+k] = pixVal;
                }
            }
            if (offset != 0)
            {
                for (j=0; j<offset; j++)
                {
                    fread(&pixVal, sizeof(unsigned char), 1, pFile);
                }
            }
        }
    }
    return bmpImg;
}

int imwrite(char* path, ZqImage* bmpImg)
{
    FILE *pFile;
    BMPFileHeader bmpFileHeader;
    BMPInfoHeader bmpInfoHeader;
    int width = 0;
    int height = 0;
    int step = 0;
	int channels = 1;
    int i, j;
    int offset;
    unsigned char pixVal = '\0';
    RgbQuad* quad;
	width = bmpImg->width;
	height = bmpImg->height;
	channels = bmpImg->channels;
	step = width * channels;
    pFile = fopen(path, "wb");
    if (!pFile)
    {
        return -1;
    }
	//写入文件头标识符
	bmpFileHeader.bfType = 0x4D42;
    if (channels == 1)//8位，灰度图
    {
		//windows规定每一个扫描行为4的倍数，不足补0
        offset = step%4;
        if (offset != 0)
        {
			offset=4-offset;
            step += offset;
        }
		//写入文件头
        bmpFileHeader.bfSize = 54 + 256*4 + width;
        bmpFileHeader.bfReserved1 = 0;
        bmpFileHeader.bfReserved2 = 0;
        bmpFileHeader.bfOffBits = 54 + 256*4;
        fwrite(&bmpFileHeader, sizeof(BMPFileHeader), 1, pFile);
		//写入信息头
        bmpInfoHeader.biSize = 40;
        bmpInfoHeader.biWidth = width;
        bmpInfoHeader.biHeight = height;
        bmpInfoHeader.biPlanes = 1;
        bmpInfoHeader.biBitCount = 8;
        bmpInfoHeader.biCompression = 0;
        bmpInfoHeader.biSizeImage = height*step;
        bmpInfoHeader.biXPelsPerMeter = 0;
        bmpInfoHeader.biYPelsPerMeter = 0;
        bmpInfoHeader.biClrUsed = 256;
        bmpInfoHeader.biClrImportant = 256;
        fwrite(&bmpInfoHeader, sizeof(BMPInfoHeader), 1, pFile);
  
        quad = (RgbQuad*)malloc(sizeof(RgbQuad)*256);
        for (i=0; i<256; i++)
        {
            quad[i].rgbBlue = i;
            quad[i].rgbGreen = i;
            quad[i].rgbRed = i;
            quad[i].rgbReserved = 0;
        }
        fwrite(quad, sizeof(RgbQuad), 256, pFile);
        free(quad);
  
        for (i=height-1; i>-1; i--)
        {
            for (j=0; j<width; j++)
            {
                pixVal = bmpImg->imageData[i*width+j];
                fwrite(&pixVal, sizeof(unsigned char), 1, pFile);
            }
            if (offset!=0)
            {
                for (j=0; j<offset; j++)
                {
                    pixVal = 0;
                    fwrite(&pixVal, sizeof(unsigned char), 1, pFile);
                }
            }
        }
    }
	else if (channels == 3)//24位，通道，彩图
    {
		//windows规定每一个扫描行为4的倍数，不足补0
        offset = step%4;
        if (offset != 0)
        {
			offset=4-offset;
            step += 4-offset;
        }
		//写入文件头
        bmpFileHeader.bfSize = height*step + 54;
        bmpFileHeader.bfReserved1 = 0;
        bmpFileHeader.bfReserved2 = 0;
        bmpFileHeader.bfOffBits = 54;
        fwrite(&bmpFileHeader, sizeof(BMPFileHeader), 1, pFile);
		//写入信息头
        bmpInfoHeader.biSize = 40;
        bmpInfoHeader.biWidth = width;
        bmpInfoHeader.biHeight = height;
        bmpInfoHeader.biPlanes = 1;
        bmpInfoHeader.biBitCount = 24;
        bmpInfoHeader.biCompression = 0;
        bmpInfoHeader.biSizeImage = height*step;
        bmpInfoHeader.biXPelsPerMeter = 0;
        bmpInfoHeader.biYPelsPerMeter = 0;
        bmpInfoHeader.biClrUsed = 0;
        bmpInfoHeader.biClrImportant = 0;
        fwrite(&bmpInfoHeader, sizeof(BMPInfoHeader), 1, pFile);
  
        for (i=bmpImg->height-1; i>-1; i--)
        {
            for (j=0; j<bmpImg->width; j++)
            {
                pixVal = bmpImg->imageData[i*width*3+j*3+0];
                fwrite(&pixVal, sizeof(unsigned char), 1, pFile);
                pixVal = bmpImg->imageData[i*width*3+j*3+1];
                fwrite(&pixVal, sizeof(unsigned char), 1, pFile);
                pixVal = bmpImg->imageData[i*width*3+j*3+2];
                fwrite(&pixVal, sizeof(unsigned char), 1, pFile);
            }
            if (offset!=0)
            {
                for (j=0; j<offset; j++)
                {
                    pixVal = 0;
                    fwrite(&pixVal, sizeof(unsigned char), 1, pFile);
                }
            }
        }
    }
    fclose(pFile);
    return 0;
}

ZqImage* imrotate(ZqImage* bmpImg,int Angle)
{
	 //图片旋转处理
	ZqImage* bmpImgRot;
    double angle;//要旋转的弧度数
    int width = 0;
    int height = 0;
    int step = 0;
	int Rot_step = 0;
	int channels = 1;
    int i, j, k;
	width = bmpImg->width;
	height = bmpImg->height;
	channels = bmpImg->channels;
    int midX_pre,midY_pre,midX_aft,midY_aft;//旋转前后的中心点的坐标
    midX_pre = width / 2;
    midY_pre = height / 2;
    int pre_i,pre_j,after_i,after_j;//旋转前后对应的像素点坐标
    angle = 1.0 * Angle * PI / 180;
	//初始化旋转后图片的信息
	bmpImgRot = (ZqImage*)malloc(sizeof(ZqImage));
	bmpImgRot->channels = channels;
	bmpImgRot->width = bmpImg->width;
	bmpImgRot->height = bmpImg->height;
	midX_aft =bmpImgRot->width / 2;
    midY_aft = bmpImgRot->height / 2;
	step = channels * width;
	Rot_step = channels * bmpImgRot->width;
	bmpImgRot->imageData = (unsigned char*)malloc(sizeof(unsigned char)*bmpImgRot->width*bmpImgRot->height*channels);
    if (channels == 1)
    {
		//初始化旋转图像
		for (i=0; i<bmpImgRot->height; i++)
		{
			for (j=0; j<bmpImgRot->width; j++)
			{
				bmpImgRot->imageData[(bmpImgRot->height-1-i)*Rot_step+j] = 0;
			}
		}
		//坐标变换
		for(i = 0;i < bmpImgRot->height;i++)
		{
			for(j = 0;j < bmpImgRot->width;j++)
			{
				after_i = i - midX_aft;
				after_j = j - midY_aft;
				pre_i = (int)(cos((double)angle) * after_i - sin((double)angle) * after_j) + midX_pre;
				pre_j = (int)(sin((double)angle) * after_i + cos((double)angle) * after_j) + midY_pre;
				if(pre_i >= 0 && pre_i < height && pre_j >= 0 && pre_j < width)//在原图范围内
					 bmpImgRot->imageData[i * Rot_step + j] = bmpImg->imageData[pre_i * step + pre_j];
			}
		}
    }
    else if (channels == 3)
    {
		//初始化旋转图像
		for(i=0; i<bmpImgRot->height; i++)
		{
			for(j=0; j<bmpImgRot->width; j++)
			{
				for(k=0; k<3; k++)
                {
					bmpImgRot->imageData[(bmpImgRot->height-1-i)*Rot_step+j*3+k] = 0;
                }
			}
		}
		//坐标变换
		for(i = 0;i < bmpImgRot->height;i++)
		{
			for(j = 0;j < bmpImgRot->width;j++)
			{
				after_i = i - midX_aft;
				after_j = j - midY_aft;
				pre_i = (int)(cos((double)angle) * after_i - sin((double)angle) * after_j) + midX_pre;
				pre_j = (int)(sin((double)angle) * after_i + cos((double)angle) * after_j) + midY_pre;

				if(pre_i >= 0 && pre_i < height && pre_j >= 0 && pre_j < width)//在原图范围内
					for(k=0; k<3; k++)
					{
						bmpImgRot->imageData[i * Rot_step + j*3 +k] = bmpImg->imageData[pre_i * step + pre_j*3 + k];
					}
			}
		}
    }
	return bmpImgRot;
}

ZqImage* imscale(ZqImage* bmpImg,double dy,double dx)
{
	 //图片缩放处理
	ZqImage* bmpImgSca;
    int width = 0;
    int height = 0;
	int channels = 1;
    int step = 0;
	int Sca_step = 0;
    int i, j, k;
	width = bmpImg->width;
	height = bmpImg->height;
	channels = bmpImg->channels;
    int pre_i,pre_j,after_i,after_j;//缩放前对应的像素点坐标
	//初始化缩放后图片的信息
	bmpImgSca = (ZqImage*)malloc(sizeof(ZqImage));
	bmpImgSca->channels = channels;
	bmpImgSca->width = (int)(bmpImg->width*dy + 0.5);
	bmpImgSca->height = (int)(bmpImg->height*dx + 0.5);
	step = channels * width;
	Sca_step = channels * bmpImgSca->width;
	bmpImgSca->imageData = (unsigned char*)malloc(sizeof(unsigned char)*bmpImgSca->width*bmpImgSca->height*channels);

    if (channels == 1)
    {
		//初始化缩放图像
		for (i=0; i<bmpImgSca->height; i++)
		{
			for (j=0; j<bmpImgSca->width; j++)
			{
				bmpImgSca->imageData[(bmpImgSca->height-1-i)*Sca_step+j] = 0;
			}
		}
		//坐标变换
		for(i = 0;i < bmpImgSca->height;i++)
		{
			for(j = 0;j < bmpImgSca->width;j++)
			{
				after_i = i;
				after_j = j;
				pre_i = (int)(after_i / dx + 0);
				pre_j = (int)(after_j / dy + 0);
				if(pre_i >= 0 && pre_i < height && pre_j >= 0 && pre_j < width)//在原图范围内
				{
					bmpImgSca->imageData[i * Sca_step + j] = bmpImg->imageData[pre_i * step + pre_j];
				}
			}
		}
    }
    else if (channels == 3)
    {
		//初始化缩放图像
		for(i=0; i<bmpImgSca->height; i++)
		{
			for(j=0; j<bmpImgSca->width; j++)
			{
				for(k=0; k<3; k++)
                {
					bmpImgSca->imageData[(bmpImgSca->height-1-i)*Sca_step+j*3+k] = 0;
                }
			}
		}
		//坐标变换
		for(i = 0;i < bmpImgSca->height;i++)
		{
			for(j = 0;j < bmpImgSca->width;j++)
			{
				after_i = i;
				after_j = j;
				pre_i = (int)(after_i / dx + 0.5);
				pre_j = (int)(after_j / dy + 0.5);
				if(pre_i >= 0 && pre_i < height && pre_j >= 0 && pre_j < width)//在原图范围内
					for(k=0; k<3; k++)
					{
						bmpImgSca->imageData[i * Sca_step + j*3 +k] = bmpImg->imageData[pre_i * step + pre_j*3 + k];
					}
			}
		}
    }
	return bmpImgSca;
}
