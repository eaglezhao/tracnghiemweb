#include "StdAfx.h"
#include "histogram.h"

#include "math.h"


CImageOp::CImageOp()
: m_pImg(NULL), m_dwWidth(0), m_dwHeight(0), m_lStride(0), m_iBitDepth(0)
{
}

// SetFormat: Sets the image format (height, width, etc).

// Use UYVY only!

HRESULT CImageOp::SetFormat(const VIDEOINFOHEADER& vih)
{
    // Check if UYVY
    if (vih.bmiHeader.biCompression != 'YVYU')
    {
        return E_INVALIDARG;
    }

    int BytesPerPixel = vih.bmiHeader.biBitCount / 8;

    // If the target rectangle (rcTarget) is empty, the image width and the stride are both biWidth.
    // Otherwise, image width is given by rcTarget and the stride is given by biWidth.

    if (IsRectEmpty(&vih.rcTarget))
    {
        m_dwWidth = vih.bmiHeader.biWidth;
        m_lStride = m_dwWidth;   
    }
    else
    {
        m_dwWidth = vih.rcTarget.right;
        m_lStride = vih.bmiHeader.biWidth;
    }

    m_lStride = (m_lStride * BytesPerPixel + 3) & ~3; // stride for UYVY is rounded to the nearest DWORD

    m_dwHeight = abs(vih.bmiHeader.biHeight);  // biHeight can be < 0, but YUV is always top-down
    m_iBitDepth = vih.bmiHeader.biBitCount;
    
    return S_OK;
}


HRESULT CImageOp::SetImage(BYTE *pBuffer)
{
    m_pImg = pBuffer;
    return S_OK;
}

HRESULT CImageOp::ScanImage()
{
    if (!m_pImg)
    {
        return E_UNEXPECTED;
    }

    return _ScanImage();
}

HRESULT CImageOp::ConvertImage()
{
    if (!m_pImg)
    {
        return E_UNEXPECTED;
    }
    return _ConvertImage();
}


HRESULT CImagePointOp::_ConvertImage()
{
    DWORD iRow, iPixel;  // looping variables
    BYTE *pRow = m_pImg; // pointer to the first row in the buffer (don't care about image orientation)

    _ASSERTE(m_iBitDepth == 16);

    for (iRow = 0; iRow < m_dwHeight; iRow++)
    {
        UYVY_PIXEL *pPixel = reinterpret_cast<UYVY_PIXEL*>(pRow);

        for (iPixel = 0; iPixel < m_dwWidth; iPixel++, pPixel++)
        {
            // Normalize luma to 0 - 219 range
            BYTE luma = (BYTE)clipYUV(pPixel->y) - MIN_LUMA;

            // Convert from LUT. The result is already in the correct 16 - 239 range
            pPixel->y = m_LookUpTable[luma];

            _ASSERTE(pPixel->y >= MIN_LUMA);
            _ASSERTE(pPixel->y <= MAX_LUMA);
        }
        pRow += m_lStride;
    }
    return S_OK;
}


// _ScanImage: Scan the image and create a look up table

HRESULT CContrastStretch::_ScanImage()
{
    DWORD iRow, iPixel;  // looping variables
    BYTE *pRow = m_pImg; // pointer to the first row in the buffer (don't care about image orientation)

    BYTE MinLuma = 255;
    BYTE MaxLuma = 0;

    _ASSERTE(m_iBitDepth == 16);

    // Find the minimum and maximum luma values
    for (iRow = 0; iRow < m_dwHeight; iRow++)
    {
        UYVY_PIXEL *pPixel = reinterpret_cast<UYVY_PIXEL*>(pRow);

        for (iPixel = 0; iPixel < m_dwWidth; iPixel++, pPixel++)
        {
            BYTE luma = pPixel->y;

            // normalize to a 0-219 range
            luma -= MIN_LUMA;

            if (luma > MaxLuma)
            {
                MaxLuma =  luma;
            }
            else if (luma < MinLuma)
            {
                MinLuma = luma;
            }
        }
        pRow += m_lStride;
    }

    // Calculate the scaling factor
    double factor = static_cast<double>(LUMA_RANGE - 1) / (MaxLuma - MinLuma);

    // Build the LUT. 

    unsigned int i;
    for (i = 0; i < MinLuma; i++)
    {
        m_LookUpTable[i] = MIN_LUMA;
    }
    for (i = MinLuma; i < LUMA_RANGE; i++)
    {
        double stretched = factor * (i - MinLuma);
        _ASSERTE(stretched >= 0);
        // clip
        if (stretched >= LUMA_RANGE)
        {
            stretched = LUMA_RANGE - 1;
        }
        m_LookUpTable[i] = static_cast<BYTE>(stretched) + MIN_LUMA;
    }

    return S_OK;
}



////

HRESULT CEqualize::_ScanImage()
{
    DWORD iRow, iPixel;  // looping variables
    BYTE *pRow = m_pImg; // pointer to the first row in the buffer (don't care about image orientation)

    DWORD  histogram[LUMA_RANGE];  // basic histogram
    double nrm_histogram[LUMA_RANGE]; // normalized histogram

    ZeroMemory(histogram, sizeof(histogram));

    _ASSERTE(m_iBitDepth == 16);

    // Create a histogram. For each pixel, find the luma and increment the count for that
    // pixel. Luma values are translated from the nominal 16 - 235 range to a 0-219 array

    for (iRow = 0; iRow < m_dwHeight; iRow++)
    {
        UYVY_PIXEL *pPixel = reinterpret_cast<UYVY_PIXEL*>(pRow);

        for (iPixel = 0; iPixel < m_dwWidth; iPixel++, pPixel++)
        {
            BYTE luma = pPixel->y;
            luma = static_cast<BYTE>(clipYUV(luma)) - MIN_LUMA;
            histogram[luma]++;
        }
        pRow += m_lStride;
    }

    // Build the cumulative histogram.  
    for (int i = 1; i < LUMA_RANGE; i++)
    {
        // The i'th entry is the sum of the previous entries
        histogram[i] = histogram[i-1] + histogram[i];
    }

    // Normalize the histogram. 
    DWORD area = NumPixels(); 

    for (int i = 0; i < LUMA_RANGE; i++)
    {
        nrm_histogram[i] = static_cast<double>( LUMA_RANGE * histogram[i] ) / area;
    }

    // Create the LUT
    for (int i = 0; i < LUMA_RANGE; i++)
    {
        // Clip the result to the nominal luma range

        long rounded = static_cast<long>(nrm_histogram[i] + 0.5);
        long clipped = clip(rounded, 0, LUMA_RANGE - 1);
        
        m_LookUpTable[i] = static_cast<BYTE>(clipped) + MIN_LUMA;
    }

    return S_OK;
}




// _ScanImage: Scan the image and create a look up table

HRESULT CLogStretch::_ScanImage()
{
    DWORD iRow, iPixel;  // looping variables
    BYTE *pRow = m_pImg; // pointer to the first row in the buffer (don't care about image orientation)

    BYTE LocalMaxLuma = 0;  // maximum luma value within this image

    _ASSERTE(m_iBitDepth == 16);

    // Find the minimum and maximum luma values
    for (iRow = 0; iRow < m_dwHeight; iRow++)
    {
        UYVY_PIXEL *pPixel = reinterpret_cast<UYVY_PIXEL*>(pRow);

        for (iPixel = 0; iPixel < m_dwWidth; iPixel++, pPixel++)
        {
            BYTE luma = pPixel->y;

            // normalize to a 0-219 range
            luma -= MIN_LUMA;

            if (luma > LocalMaxLuma)
            {
                LocalMaxLuma =  luma;
            }
        }
        pRow += m_lStride;
    }

    // Calculate the scaling constant
    double k = static_cast<double>(LocalMaxLuma) / log10(1.0 + static_cast<double>(LocalMaxLuma));

    // variants:
    // natural log instead of log 10; this changes the shape of the curve
    // k = MAX_LUMA / log(1 + LocalMaxLuma); this stretches the upper end of the luminance range
    
    // Build the LUT. 
    for (unsigned int i = 0; i < LUMA_RANGE; i++)
    {
        double scaled = k * log10(1.0 + i);

        // clip
        if (scaled < 0)
        {
            scaled = 0;
        }
        else if (scaled >= LUMA_RANGE)
        {
            scaled = LUMA_RANGE - 1;
        }
        m_LookUpTable[i] = static_cast<BYTE>(scaled) + MIN_LUMA;
    }

    return S_OK;
}




// Image functions

// For RGB 24 pixels:
void RGB2YUV(const RGBTRIPLE& rgb, YUV_PIXEL& yuv)
{
    yuv.y = RGB2Y(rgb.rgbtRed, rgb.rgbtGreen, rgb.rgbtBlue);
    yuv.u = RGB2U(rgb.rgbtRed, rgb.rgbtGreen, rgb.rgbtBlue);
    yuv.v = RGB2V(rgb.rgbtRed, rgb.rgbtGreen, rgb.rgbtBlue);
}

void YUV2RGB(const YUV_PIXEL& yuv, RGBTRIPLE& rgb)
{
    long y = yuv.y, u = yuv.u, v = yuv.v;
    rgb.rgbtRed = YUV2Red(y, u, v);
    rgb.rgbtGreen = YUV2Green(y, u, v);
    rgb.rgbtBlue = YUV2Blue(y, u, v);
}


// For RGB 32 pixels:
void RGB2YUV(const RGBQUAD& rgb, YUV_PIXEL& yuv)
{
    yuv.y = RGB2Y(rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue);
    yuv.u = RGB2U(rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue);
    yuv.v = RGB2V(rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue);
}

void YUV2RGB(const YUV_PIXEL& yuv, RGBQUAD& rgb)
{
    long y = yuv.y, u = yuv.u, v = yuv.v;
    rgb.rgbRed = YUV2Red(y, u, v);
    rgb.rgbGreen = YUV2Green(y, u, v);
    rgb.rgbBlue = YUV2Blue(y, u, v);
}

