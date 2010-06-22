#pragma once

// nominal luma range
const BYTE MIN_LUMA = 16;
const BYTE MAX_LUMA = 235;
const BYTE LUMA_RANGE = MAX_LUMA - MIN_LUMA + 1; // Total number of valid luma values

// structure for 8-bit-per-channel YUV pixels.
typedef struct YUVPIXELtag
{
    BYTE y;
    BYTE u;
    BYTE v;
} YUV_PIXEL;

// structure for UYVY pixels

typedef struct UYVYPIXELtag
{
    BYTE chroma;
    BYTE y;
} UYVY_PIXEL;


// CImageOp: Base class for image operations
class CImageOp
{
public:

    CImageOp();

    virtual HRESULT SetFormat(const VIDEOINFOHEADER& vih);
    HRESULT SetImage(BYTE *pBuffer);
    HRESULT ScanImage();
    HRESULT ConvertImage();

protected:

    DWORD NumPixels() { return m_dwHeight * m_dwWidth; }
    virtual HRESULT _ScanImage() = 0;
    virtual HRESULT _ConvertImage() = 0;

    BYTE*   m_pImg;
    DWORD   m_dwWidth;
    DWORD   m_dwHeight;
    LONG    m_lStride;
    int     m_iBitDepth;
};

// CImagePointOp: Base class for image point operations

class CImagePointOp : public CImageOp
{
protected:
    HRESULT _ConvertImage();

    BYTE   m_LookUpTable[LUMA_RANGE];  // Look up table (LUT) for point operations
};

// CContrastStretch: Implements a histogram stretch, aka automatic gain control
class CContrastStretch : public CImagePointOp
{
protected:
    HRESULT _ScanImage();
};

// CEqualize: Implements histogram equalization.
class CEqualize : public CImagePointOp
{
protected:
    HRESULT _ScanImage();
};


class CLogStretch : public CImagePointOp
{
protected:
    HRESULT _ScanImage();
};


// Image functions

// These conversion functions are taken from "Video Rendering with 8-Bit YUV Formats", 
// by Stephen Estrop and Gary Sullivan
// http://msdn.microsoft.com/library/en-us/dnwmt/html/YUVFormats.asp

// (I'm not using everything here, but may be useful one day...)

inline long clip(long i, long min, long max)
{
    if (i < min) return min;
    if (i > max) return max;
    return i;
}


#define clipRGB(x) clip((x), 0, 255)
#define clipYUV(x) clip((x), MIN_LUMA, MAX_LUMA)


// RGB2YUV: Convert an RGB pixel to YUV

inline BYTE RGB2Y(DWORD r, DWORD g, DWORD b)
{
    BYTE luma = static_cast<BYTE>(( (  66 * r + 129 * g +  25 * b + 128) >> 8) +  16);
    _ASSERTE(luma >= MIN_LUMA);
    _ASSERTE(luma <= MAX_LUMA);
    return luma;
}

inline BYTE RGB2U(DWORD r, DWORD g, DWORD b)
{
    return static_cast<BYTE>(( ( -38 * r -  74 * g + 112 * b + 128) >> 8) + 128);
}

inline BYTE RGB2V(DWORD r, DWORD g, DWORD b)
{
    return static_cast<BYTE>(( ( 112 * r -  94 * g -  18 * b + 128) >> 8) + 128);
}


// For RGB 24 pixels:
void RGB2YUV(const RGBTRIPLE& rgb, YUV_PIXEL& yuv);

// For RGB 32 pixels:
void RGB2YUV(const RGBQUAD& rgb, YUV_PIXEL& yuv);


// Convert YUV to RGB

inline BYTE YUV2Red(long y, long u, long v)
{
    return static_cast<BYTE>(clipRGB(( 298 * (y - 16) + 409 * (v - 128) + 128) >> 8));
}


inline BYTE YUV2Green(long y, long u, long v)
{
    return static_cast<BYTE>((clipRGB(( 298 * (y - 16) - 100 * (u - 128) - 208 * (v - 128) + 128) >> 8)));
}

inline BYTE YUV2Blue(long y, long u, long v)
{
    return static_cast<BYTE>((clipRGB(( 298 * (y - 16) + 516 * (u - 128) + 128) >> 8)));
}


void YUV2RGB(const YUV_PIXEL& yuv, RGBTRIPLE& rgb);

void YUV2RGB(const YUV_PIXEL& yuv, RGBQUAD& rgb);
