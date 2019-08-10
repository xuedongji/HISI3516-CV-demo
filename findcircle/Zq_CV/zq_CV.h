#ifndef ZQ_CV_H  
#define ZQ_CV_H
#define PI 3.1415926  
/*按字节对齐Start
  如果不按字节对齐，
  第一个struct定义为16bit，
  而BMP文件中实际为14bit，
  故读取会错位。
  --zhangquan
*/
//位图文件头bf
#pragma pack (push ,1)
typedef struct  
{  
    unsigned short bfType;  //文件标识符BM，0x424D 2bit
    unsigned long    bfSize;//文件的大小 4bit
    unsigned short    bfReserved1; //保留值,必须设置为0 2bit
    unsigned short    bfReserved2; //保留值,必须设置为0 2bit
    unsigned long    bfOffBits;//文件头的最后到图像数据位开始的偏移量 4bit
} BMPFileHeader;

//位图信息头bi  
typedef struct  
{  
    unsigned long  biSize;//信息头的大小
    long   biWidth;   //图像宽度
    long   biHeight;   //图像高度
    unsigned short   biPlanes; //图像的位面数
    unsigned short   biBitCount;//每个像素的位数
    unsigned long  biCompression;//压缩类型
    unsigned long  biSizeImage;//图像大小，字节
    long   biXPelsPerMeter; //水平分辨率
    long   biYPelsPerMeter; //垂直分辨率
    unsigned long   biClrUsed; //使用的色彩数
    unsigned long   biClrImportant;//重要的颜色数
} BMPInfoHeader;
//颜色表
typedef struct
{  
    unsigned char rgbBlue; //蓝色分量
    unsigned char rgbGreen; //绿色分量
    unsigned char rgbRed; //红色分量
    unsigned char rgbReserved; //保留值
} RgbQuad;

typedef struct
{
    int width;
    int height;
    int channels;
    unsigned char* imageData;
}ZqImage;
#pragma pack (pop)
/*按字节对齐End*/
ZqImage* imread(char* path);
int imwrite(char* path, ZqImage* bmpImg);
ZqImage* imrotate(ZqImage* bmpImg,int Angle);
ZqImage* imscale(ZqImage* bmpImg,double dy,double dx);
  
#endif
