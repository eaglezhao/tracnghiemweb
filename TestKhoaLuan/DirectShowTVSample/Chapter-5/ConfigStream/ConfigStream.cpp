// ConfigStream.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <dshow.h>
#include <stdio.h>



int main(int argc, char* argv[])
{

	IGraphBuilder       *pGraph = NULL;
	IMediaControl       *pControl =NULL;
	IBaseFilter         *pMJPEG = NULL;
	IBaseFilter         *pADPCM = NULL;
	IAMVideoCompression *pVC = NULL;
	IAMStreamConfig     *pSC = NULL;
	AM_MEDIA_TYPE       *pmt = NULL;
	HRESULT             hr;

	
	
	
	CoInitialize(NULL);

    CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, 
            IID_IGraphBuilder, (void **)&pGraph);

	pGraph->RenderFile (L"C:\\My Documents\\DirectShow Documents\\Multimedia Compression\\Applications\\ME-video.grf", NULL);
	hr= pGraph->FindFilterByName (L"MJPEG Compressor", &pMJPEG);
	if (FAILED (hr)) printf ("could not find filter in graph\n");

 	hr = pMJPEG->QueryInterface (IID_IAMVideoCompression, (void **) &pVC);
	if (FAILED (hr)) printf ("could not obtain interface\n");

    hr = pVC->put_KeyFrameRate (4);
	
	pMJPEG->QueryInterface (IID_IAMStreamConfig, (void **) &pSC);
	pSC->GetFormat(&pmt);

	((VIDEOINFOHEADER *)(pmt->pbFormat))->dwBitRate = 100000;
	pSC->SetFormat (pmt);

	pGraph->FindFilterByName (L"Microsoft ADPCM",  &pADPCM);
	
	pADPCM->QueryInterface (IID_IAMStreamConfig, (void **) &pSC);
	pSC->GetFormat (&pmt);

	((WAVEFORMATEX *)(pmt->pbFormat))->wBitsPerSample = 16000;
	pSC->SetFormat (pmt);

	pControl->Run ();
	MessageBox (NULL, "hi", NULL, NULL);

	
	
	return 0;
}
