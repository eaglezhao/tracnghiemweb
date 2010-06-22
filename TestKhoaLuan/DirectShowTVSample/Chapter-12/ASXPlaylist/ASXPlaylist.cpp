// ASXPlaylist.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <dshow.h>
#include <playlist.h>

int main(int argc, char* argv[])
{
	// DirectShow interfaces
IGraphBuilder *pGB = NULL;
IMediaControl *pMC = NULL;
IMediaEventEx *pME = NULL;
IBasicAudio   *pBA = NULL;
IMediaSeeking *pMS = NULL;
IBaseFilter   *pBF = NULL;
IAMPlayList   *pPL = NULL;
IAMPlayListItem *pPLI = NULL;

DWORD pdwSources = NULL;

// {D51BD5AE-7548-11CF-A520-0080C77EF58A}

 
    CoInitialize (NULL);

	// Get the interface for DirectShow's GraphBuilder
    CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGB);

    // Have the graph construct its the appropriate graph automatically
       
    // QueryInterface for DirectShow interfaces
    pGB->QueryInterface(IID_IMediaControl, (void **)&pMC);
    pGB->QueryInterface(IID_IMediaEventEx, (void **)&pME);
    pGB->QueryInterface(IID_IMediaSeeking, (void **)&pMS);

//	pMC->RenderFile (L"C:\\WMSDK\\WMSSDK\\Samples\\ASX\\DEMO2.ASX");
//	pMC->RenderFile (L"http://hurl.musicserver2.com/scripts/hurl.exe?clipid=028546001020006550&cid=600024");
	pGB->FindFilterByName (L"XML Playlist", &pBF);

    pBF->QueryInterface (IID_IAMPlayList, (void **) &pPL);
    


//	pMC->Run ();

    pPL->GetItemCount (&pdwSources);
	printf ("ItemCount: %d\n", pdwSources);

	pPL->GetItem (3, &pPLI);

	pPLI->GetSourceCount (&pdwSources);
	printf ("SourceCount: %d\n", pdwSources);
	
	REFERENCE_TIME prtDuration;
	pPLI->GetSourceDuration (0, &prtDuration);

	printf ("SourceDuration: %d\n", (DWORD) prtDuration); 
	
	BSTR pbstrURL;
 	pPLI->GetSourceURL (0, &pbstrURL);

    printf ("SourceURL: %S\n", pbstrURL);

	pPLI->GetLinkURL (&pbstrURL);
	printf ("LinkURL: %S\n", pbstrURL);

	BSTR pbstrStartMarker;

	pPLI->GetSourceStartMarkerName (0, &pbstrStartMarker);
	printf ("SourceStartMarkerName: %S\n", pbstrStartMarker);
	 
return 0;	
}

