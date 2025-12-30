#ifndef _BMP_H
#define _BMP_H

#include <string>

using namespace std;


/// \位图处理
class CBmp
{
public:

    CBmp();
    ~CBmp();

    ///\ 设置热度背景图路径
    ///\ szBmpPathName : 背景图片路径名称
    void SetBmpPathName(const char *szBmpPathName,int iImgWidth,int iImgHeight);

    ///\ 设置original
    void SetoriginalValue(int ioriginalWidth,int ioriginalHeight);

    ///\ 生成热度图
    bool CreateHeatMapBmp( unsigned char * szData);

private:

    //  彩色之前先处理24位灰度图

    // 获取模板窗口内的直方图
    void Gethistogram(BYTE *pCur, int width, int height, int *histogram);

    // 获取模板内的中值
    void GetMedian(int *histogram,int N,int  M,int *med);

    //  拉伸图像的灰度范围
    bool EnlargeGrayRange(unsigned char* &pImg, int length, int Gmin, int Gmax);

    ///\ 二进制数据转化为24位位图
    BYTE * Create24bitGrayImg(unsigned char *szBmpData);

    ///\ 放大灰度图到背景图尺寸
    bool GrayImgZoomInToBkSize( BYTE *pGrayImg);

    ///\ 中值平滑处理灰度图像     
    bool SmoothGrayImgByMeida();

    ///\ 灰度图像转化为伪彩色图像- 彩虹图算法
    bool ImgChangeGray2ColorByRainbow();

    ///\ 合成热度图图像
    bool ComposeColorImg();

    ///\ 读取Bmp图片
    BYTE *ReadBmp_Test(const char *szSavedName, int & bmpWidth, int & bmpHeight, int & biBitCount);

    ///\ 保存Bmp图片
    bool SaveBmp_Test(const char *szSavedName, BYTE *pImgBuf, int width, int height, int biBitCount);

private:	
    // 背景图像指针
    BYTE *m_pBkColorImg;
    // 灰度浮层指针
    BYTE *m_pGrayOverLayerbuf;
    // 彩色浮层指针
    BYTE *m_pColorOverLayerbuf;
    // 背景图片名称
    string m_strBkImgName;	
    // 背景图像宽度
    int m_bkImgWidth;
    // 背景图像高度
    int m_bkImgHeight;	
};

#endif //_BMP_H


