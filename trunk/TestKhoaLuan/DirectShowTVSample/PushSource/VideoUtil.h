#pragma once


//----------------------------------------------------------------------------
// GetVideoInfoParameters
//
// Helper function to get the important information out of a VIDEOINFOHEADER
// Author: Robin Speed
//-----------------------------------------------------------------------------

void GetVideoInfoParameters(
    const VIDEOINFOHEADER *pvih, // Pointer to the format header.
    BYTE  * const pbData,        // Pointer to the first address in the buffer.
    DWORD *pdwWidth,         // Returns the width in pixels.
    DWORD *pdwHeight,        // Returns the height in pixels.
    LONG  *plStrideInBytes,  // Add this to a row to get the new row down
    BYTE **ppbTop,           // Returns pointer to the first byte in the top row of pixels.
    bool bYuv                // Is this a YUV format? (true = YUV, false = RGB)
    )
{
    LONG lStride;


    //  For 'normal' formats, biWidth is in pixels. 
    //  Expand to bytes and round up to a multiple of 4.
    if (pvih->bmiHeader.biBitCount != 0 &&
        0 == (7 & pvih->bmiHeader.biBitCount)) 
    {
        lStride = (pvih->bmiHeader.biWidth * (pvih->bmiHeader.biBitCount / 8) + 3) & ~3;
    } 
    else   // Otherwise, biWidth is in bytes.
    {
        lStride = pvih->bmiHeader.biWidth;
    }

    //  If rcTarget is empty, use the whole image.
    if (IsRectEmpty(&pvih->rcTarget)) 
    {
        *pdwWidth = (DWORD)pvih->bmiHeader.biWidth;
        *pdwHeight = (DWORD)(abs(pvih->bmiHeader.biHeight));
        
        if (pvih->bmiHeader.biHeight < 0 || bYuv)   // Top-down bitmap. 
        {
            *plStrideInBytes = lStride; // Stride goes "down"
            *ppbTop           = pbData; // Top row is first.
        } 
        else        // Bottom-up bitmap
        {
            *plStrideInBytes = -lStride;    // Stride goes "up"
            *ppbTop = pbData + lStride * (*pdwHeight - 1);  // Bottom row is first.
        }
    } 
    else   // rcTarget is NOT empty. Use a sub-rectangle in the image.
    {
        *pdwWidth = (DWORD)(pvih->rcTarget.right - pvih->rcTarget.left);
        *pdwHeight = (DWORD)(pvih->rcTarget.bottom - pvih->rcTarget.top);
        
        if (pvih->bmiHeader.biHeight < 0 || bYuv)   // Top-down bitmap.
        {
            // Same stride as above, but first pixel is modified down
            // and and over by the target rectangle.
            *plStrideInBytes = lStride;     
            *ppbTop = pbData +
                     lStride * pvih->rcTarget.top +
                     (pvih->bmiHeader.biBitCount * pvih->rcTarget.left) / 8;
        } 
        else  // Bottom-up bitmap.
        {
            *plStrideInBytes = -lStride;
            *ppbTop = pbData +
                     lStride * (pvih->bmiHeader.biHeight - pvih->rcTarget.top - 1) +
                     (pvih->bmiHeader.biBitCount * pvih->rcTarget.left) / 8;
        }
    }
}



inline REFERENCE_TIME Fps2FrameLength(double fps) 
{ 
    return (REFERENCE_TIME)((double)UNITS / fps);
}


//----------------------------------------------------------------------------
// CreateRGBVideoType
//
// Create a media type for an uncompressed RGB format
//
// pMediaType: The method fills this with the media type
// iBitDepth:  Bits per pixel. Must be 1, 4, 8, 16, 24, or 32
// Width:      Width in pixels
// Height:     Height in pixels. Use > 0 for bottom-up DIBs, < 0 for top-down DIB
// fps:        Frame rate, in frames per second
// 
//-----------------------------------------------------------------------------


HRESULT CreateRGBVideoType(CMediaType *pMediaType, int iBitDepth, long Width, long Height, double fps)
{
    if ((iBitDepth != 1) && (iBitDepth != 4) && (iBitDepth != 8) &&
        (iBitDepth != 16) && (iBitDepth != 24) && (iBitDepth != 32))
    {
        return E_INVALIDARG;
    }

    if (Width < 0)
    {
        return E_INVALIDARG;
    }

    FreeMediaType(*pMediaType);

    VIDEOINFO *pvi = (VIDEOINFO*)pMediaType->AllocFormatBuffer(sizeof(VIDEOINFO));
    if (pvi == 0) 
    {
    	return(E_OUTOFMEMORY);
    }
    ZeroMemory(pvi, sizeof(VIDEOINFO));

    pvi->AvgTimePerFrame = Fps2FrameLength(fps);

    BITMAPINFOHEADER *pBmi = &(pvi->bmiHeader);
    pBmi->biSize = sizeof(BITMAPINFOHEADER);
    pBmi->biWidth = Width;
    pBmi->biHeight = Height;
    pBmi->biPlanes = 1;
    pBmi->biBitCount = iBitDepth;

    if (iBitDepth == 16)
    {
        pBmi->biCompression = BI_BITFIELDS;
        memcpy(pvi->dwBitMasks, bits565, sizeof(DWORD) * 3);
    }
    else
    {
        pBmi->biCompression = BI_RGB;
    }

    if (iBitDepth <= 8)
    {
        // Palettized format.
        pBmi->biClrUsed = PALETTE_ENTRIES(pvi);

        HDC hdc = GetDC(NULL);  // hdc for the current display.
        if (hdc == NULL)
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }
        GetSystemPaletteEntries(hdc, 0, pBmi->biClrUsed, (PALETTEENTRY*)pvi->bmiColors);

        ReleaseDC(NULL, hdc);
    }

    pvi->bmiHeader.biSizeImage = DIBSIZE(pvi->bmiHeader);

    pMediaType->SetType(&MEDIATYPE_Video);
    pMediaType->SetFormatType(&FORMAT_VideoInfo);

    const GUID subtype = GetBitmapSubtype(&pvi->bmiHeader);
    pMediaType->SetSubtype(&subtype);

    pMediaType->SetTemporalCompression(FALSE);
    pMediaType->SetSampleSize(pvi->bmiHeader.biSizeImage);

    return S_OK;
}
