// DESClip.cpp
//

#include "DESClip.h"			// This includes all the major include files needed

// And now a few globals
char g_fileName[256];
char g_PathFileName[512];

BOOL GetMediaFileName(void)
{
	OPENFILENAME ofn;

	// Let's have fun with GetOpenFileName, kids!
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = (char*) calloc(1, 30);
	strcpy((char*) ofn.lpstrFilter, "AVI Movies"); // Only AVI files, please.
	strcpy((char*) &(ofn.lpstrFilter[strlen(ofn.lpstrFilter)+1]), "*.AVI");
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = NULL;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = (char*) calloc(1, 512);			// Allocate space, we'll free it later...
	ofn.nMaxFile = 511;
	ofn.lpstrFileTitle = (char*) calloc(1, 256);
	ofn.nMaxFileTitle = 255;
	ofn.lpstrInitialDir = NULL;				// Point to current directory
	ofn.lpstrTitle = "Select an AVI file...";
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = NULL;
	if (!GetOpenFileName(&ofn)) {
		free((void*)ofn.lpstrFilter);
		free(ofn.lpstrFile);
		free(ofn.lpstrFileTitle);
		return(false);
	} else {
		strcpy(g_PathFileName, ofn.lpstrFile);
		strcpy(g_fileName, ofn.lpstrFileTitle);
		free((void*)ofn.lpstrFilter);
		free(ofn.lpstrFile);
		free(ofn.lpstrFileTitle);
	}
	return(true);
}

// This code was also brazenly stolen from the DX9 SDK
// Pass it a file name in wszPath, and it will save the filter graph to that file.
HRESULT SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath) 
{
    const WCHAR wszStreamName[] = L"ActiveMovieGraph"; 
    HRESULT hr;
    IStorage *pStorage = NULL;

	// First, create a document file which will hold the GRF file
	hr = StgCreateDocfile(
        wszPath,
        STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
        0, &pStorage);
    if(FAILED(hr)) 
    {
        return hr;
    }

	// Next, create a stream to store.
    IStream *pStream;
    hr = pStorage->CreateStream(
		wszStreamName,
        STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
        0, 0, &pStream);
    if (FAILED(hr)) 
    {
        pStorage->Release();    
        return hr;
    }

	// The IPersistStream converts a stream into a persistent object.
    IPersistStream *pPersist = NULL;
    pGraph->QueryInterface(IID_IPersistStream, reinterpret_cast<void**>(&pPersist));
    hr = pPersist->Save(pStream, TRUE);
    pStream->Release();
    pPersist->Release();
    if (SUCCEEDED(hr)) 
    {
        hr = pStorage->Commit(STGC_DEFAULT);
    }
    pStorage->Release();
    return hr;
}

// Preview a timeline.
void PreviewTL(IAMTimeline *pTL, IRenderEngine *pRender) 
{
    IGraphBuilder   *pGraph = NULL;
    IMediaControl   *pControl = NULL;
    IMediaEvent     *pEvent = NULL;

    // Build the graph.
    pRender->SetTimelineObject(pTL);
    pRender->ConnectFrontEnd( );
    pRender->RenderOutputPins( );

    // Run the graph.
    pRender->GetFilterGraph(&pGraph);
    pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
    pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
	SaveGraphFile(pGraph, L"C:\\MyGraph_preview.GRF");	// Save the graph file to disk
    pControl->Run();

    long evCode;
    pEvent->WaitForCompletion(INFINITE, &evCode);
    pControl->Stop();

    // Clean up.
    pEvent->Release();
    pControl->Release();
    pGraph->Release();
}

// Write a timeline to a disk file.
void WriteTL(IAMTimeline *pTL, IRenderEngine *pRender, WCHAR *fileName) 
{
    IGraphBuilder   *pGraph = NULL;
	ICaptureGraphBuilder2 *pBuilder = NULL;
    IMediaControl   *pControl = NULL;
    IMediaEvent     *pEvent = NULL;

    // Build the graph.
    HRESULT hr = pRender->SetTimelineObject(pTL);
    hr = pRender->ConnectFrontEnd( );

	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, 
		IID_ICaptureGraphBuilder2, (void **)&pBuilder);

	// Get a pointer to the graph front end.
	hr = pRender->GetFilterGraph(&pGraph);
	hr = pBuilder->SetFiltergraph(pGraph);

	// Create the file-writing section.
	IBaseFilter *pMux;
	hr = pBuilder->SetOutputFileName(&MEDIASUBTYPE_Avi, fileName, &pMux, NULL);

	long NumGroups;
	hr = pTL->GetGroupCount(&NumGroups);

	// Loop through the groups and get the output pins.
	for (int i = 0; i < NumGroups; i++)
	{
		IPin *pPin;
		if (pRender->GetGroupOutputPin(i, &pPin) == S_OK) 
		{
			// Connect the pin.
			hr = pBuilder->RenderStream(NULL, NULL, pPin, NULL, pMux);
			pPin->Release();
		}
	}

    // Run the graph.
    hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
    hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
	SaveGraphFile(pGraph, L"C:\\MyGraph_write.GRF");	// Save the graph file to disk
    hr = pControl->Run();

    long evCode;
    hr = pEvent->WaitForCompletion(INFINITE, &evCode);
    hr = pControl->Stop();

    // Clean up.
	if (pMux) {
		pMux->Release();
	}
    pEvent->Release();
    pControl->Release();
    pGraph->Release();
	pBuilder->Release();
}


// A fairly simple program to render a file using DirectShow Editing Services
//
int main(int argc, char* argv[])
{

	// Let's get a media file to render
	if (!GetMediaFileName()) {
		return 0;
	}

   // Start by making an empty timeline.  Add a media detector as well.
    IAMTimeline *pTL = NULL;
	IMediaDet *pMD = NULL;
    CoInitialize(NULL);
    HRESULT hr = CoCreateInstance(CLSID_AMTimeline, NULL, CLSCTX_INPROC_SERVER, 
        IID_IAMTimeline, (void**)&pTL);
	hr = CoCreateInstance(CLSID_MediaDet, NULL, CLSCTX_INPROC_SERVER, 
        IID_IMediaDet, (void**)&pMD);

    // GROUP: Add a video group to the timeline. 

    IAMTimelineGroup    *pGroup = NULL;
    IAMTimelineObj      *pGroupObj = NULL;
    hr = pTL->CreateEmptyNode(&pGroupObj, TIMELINE_MAJOR_TYPE_GROUP);
    hr = pGroupObj->QueryInterface(IID_IAMTimelineGroup, (void **)&pGroup);

    // Set the group media type. This example sets the type to "video" and
    // lets DES pick the default settings. For a more detailed example,
    // see "Setting the Group Media Type."
    AM_MEDIA_TYPE mtGroup;  
    ZeroMemory(&mtGroup, sizeof(AM_MEDIA_TYPE));
    mtGroup.majortype = MEDIATYPE_Video;
    hr = pGroup->SetMediaType(&mtGroup);
    hr = pTL->AddGroup(pGroupObj);
    pGroupObj->Release();

    // TRACK: Add a track to the group. 

    IAMTimelineObj      *pTrackObj;
    IAMTimelineTrack    *pTrack;
    IAMTimelineComp     *pComp = NULL;

    hr = pTL->CreateEmptyNode(&pTrackObj, TIMELINE_MAJOR_TYPE_TRACK);
    hr = pGroup->QueryInterface(IID_IAMTimelineComp, (void **)&pComp);
    hr = pComp->VTrackInsBefore(pTrackObj, 0);
    hr = pTrackObj->QueryInterface(IID_IAMTimelineTrack, (void **)&pTrack);

    pTrackObj->Release();


	// SOURCE: Add a source clip to the track.
	
    IAMTimelineSrc *pSource = NULL;
    IAMTimelineObj *pSourceObj;

    hr = pTL->CreateEmptyNode(&pSourceObj, TIMELINE_MAJOR_TYPE_SOURCE);
    hr = pSourceObj->QueryInterface(IID_IAMTimelineSrc, (void **)&pSource);

    // Set the times and the file name.
	BSTR bstrFile = SysAllocString(OLESTR("C:\\DShow.avi"));
    hr = pSource->SetMediaName(bstrFile); 
    SysFreeString(bstrFile);
	hr = pSource->SetMediaTimes(00000000, 50000000);
    hr = pSourceObj->SetStartStop(0, 50000000);
    hr = pTrack->SrcAdd(pSourceObj);

    pSourceObj->Release();
    pSource->Release();
	
	// TRANSITION:  Add a transition object to the track.
	// Note that the GUID for the transition is hard-coded
	// But we'd probably want the user to select it.

	IAMTimelineObj *pTransObj = NULL;
	pTL->CreateEmptyNode(&pTransObj, TIMELINE_MAJOR_TYPE_TRANSITION);
	hr = pTransObj->SetSubObjectGUID(CLSID_DxtJpeg);  // SMPTE Wipe
	hr = pTransObj->SetStartStop(40000000, 50000000);	// The last second
	IAMTimelineTransable *pTransable = NULL;
	hr = pTrack->QueryInterface(IID_IAMTimelineTransable, (void **)&pTransable);
	hr = pTransable->TransAdd(pTransObj);
	IAMTimelineTrans *pTrans = NULL;
	hr = pTransObj->QueryInterface(IID_IAMTimelineTrans, (void**)&pTrans);
	hr = pTrans->SetSwapInputs(true);

	// PROPERTY: Set a property on this transition

	IPropertySetter     *pProp;   // Property setter
	hr = CoCreateInstance(CLSID_PropertySetter, NULL, CLSCTX_INPROC_SERVER,
		IID_IPropertySetter, (void**) &pProp);
	DEXTER_PARAM param;
	DEXTER_VALUE *pValue = (DEXTER_VALUE*)CoTaskMemAlloc(sizeof(DEXTER_VALUE));

	// Initialize the parameter. 
	param.Name = SysAllocString(L"MaskNum");
	param.dispID = 0;
	param.nValues = 1;

	// Initialize the value.
	pValue->v.vt = VT_BSTR;
	pValue->v.bstrVal = SysAllocString(L"103"); // Triangle, up
	pValue->rt = 0;
	pValue->dwInterp = DEXTERF_JUMP;

	hr = pProp->AddProp(param, pValue);

	// Free allocated resources.
	SysFreeString(param.Name);
	VariantClear(&(pValue->v));
	CoTaskMemFree(pValue);

	// Set the property on the transition.
	hr = pTransObj->SetPropertySetter(pProp);
	pProp->Release();
	pTrans->Release();
	pTransObj->Release();
	pTransable->Release();
    pTrack->Release();

	// TRACK:  Add another video track to the Timeline

    hr = pTL->CreateEmptyNode(&pTrackObj, TIMELINE_MAJOR_TYPE_TRACK);
    hr = pComp->VTrackInsBefore(pTrackObj, 0);
    hr = pTrackObj->QueryInterface(IID_IAMTimelineTrack, (void **)&pTrack);
    pTrackObj->Release();

    // SOURCE: Add a source to the track.

    hr = pTL->CreateEmptyNode(&pSourceObj, TIMELINE_MAJOR_TYPE_SOURCE);
    hr = pSourceObj->QueryInterface(IID_IAMTimelineSrc, (void **)&pSource);

    // Set file name.
#ifndef UNICODE
    WCHAR wFileName[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, g_PathFileName, -1, wFileName, MAX_PATH);
	bstrFile = SysAllocString((const OLECHAR*) wFileName);
	// This is all that’s required to create a filter graph
    // Which will render a media file!
#else
    BSTR bstrFile = SysAllocString((const OLECHAR*) g_PathFileName);
#endif
    hr = pSource->SetMediaName(bstrFile); 

	// Figure out how big the track is, and add it in at that length.
	// We'll use the IMediaDet interface to do this work
	hr = pMD->put_Filename(bstrFile);
	double psl = 0;
	hr = pMD->get_StreamLength(&psl);
	psl = psl * 10000000;		// Convert to reference units
	REFERENCE_TIME pSourceLength = psl;	// and put it in the proper variable
    hr = pSource->SetMediaTimes(0, pSourceLength);
	hr = pSourceObj->SetStartStop(40000000, 40000000+pSourceLength);
    hr = pTrack->SrcAdd(pSourceObj);

	SysFreeString(bstrFile);
    pSourceObj->Release();
    pSource->Release();

	// SOURCE: Add another source to the track, after that sample.

    hr = pTL->CreateEmptyNode(&pSourceObj, TIMELINE_MAJOR_TYPE_SOURCE);
    hr = pSourceObj->QueryInterface(IID_IAMTimelineSrc, (void **)&pSource);

    // Set the times and the file name.
    hr =  pSourceObj->SetStartStop(40000000+pSourceLength, 40000000+pSourceLength+50000000);
    bstrFile = SysAllocString(OLESTR("C:\\MSPress.avi"));
    hr = pSource->SetMediaName(bstrFile); 
    SysFreeString(bstrFile);
    hr = pSource->SetMediaTimes(00000000, 50000000);
    hr = pTrack->SrcAdd(pSourceObj);

    pSourceObj->Release();
    pSource->Release();
	pComp->Release(); 

    pTrack->Release();
    pGroup->Release();


	// GROUP: Add an audio group to the timeline. 

    IAMTimelineGroup    *pAudioGroup = NULL;
    IAMTimelineObj      *pAudioGroupObj = NULL;
    hr = pTL->CreateEmptyNode(&pAudioGroupObj, TIMELINE_MAJOR_TYPE_GROUP);
    hr = pAudioGroupObj->QueryInterface(IID_IAMTimelineGroup, (void **)&pAudioGroup);

    // Set the group media type. 
	// We'll use the IMediaDet object to make this painless.
	AM_MEDIA_TYPE amtGroup; 
	bstrFile = SysAllocString(OLESTR("C:\\FoggyDay.wav"));		// Soundtrack file to use.
	hr = pMD->put_Filename(bstrFile);
	hr = pMD->get_StreamMediaType(&amtGroup);
    hr = pAudioGroup->SetMediaType(&amtGroup);
    hr = pTL->AddGroup(pAudioGroupObj);
    pAudioGroupObj->Release();


    // TRACK: Add an audio track to the group. 

    IAMTimelineObj      *pAudioTrackObj;
    IAMTimelineTrack    *pAudioTrack;
    IAMTimelineComp     *pAudioComp = NULL;

    hr = pTL->CreateEmptyNode(&pAudioTrackObj, TIMELINE_MAJOR_TYPE_TRACK);
    hr = pAudioGroup->QueryInterface(IID_IAMTimelineComp, (void **)&pAudioComp);
    hr = pAudioComp->VTrackInsBefore(pAudioTrackObj, 0);
    hr = pAudioTrackObj->QueryInterface(IID_IAMTimelineTrack, (void **)&pAudioTrack);

    pAudioTrackObj->Release();
    pAudioComp->Release();    
    pAudioGroup->Release();


	// SOURCE: Add another source to the track.

    hr = pTL->CreateEmptyNode(&pSourceObj, TIMELINE_MAJOR_TYPE_SOURCE);
    hr = pSourceObj->QueryInterface(IID_IAMTimelineSrc, (void **)&pSource);

    // Set the times and the file name.
    hr =  pSourceObj->SetStartStop(0, 50000000+pSourceLength+50000000);

    hr = pSource->SetMediaName(bstrFile); 
    SysFreeString(bstrFile);
    hr = pSource->SetMediaTimes(00000000, 50000000+pSourceLength+50000000);
    hr = pAudioTrack->SrcAdd(pSourceObj);

    pSourceObj->Release();
    pSource->Release();
	pAudioTrack->Release();

    // Preview the timeline.
    IRenderEngine *pRenderEngine = NULL;
    CoCreateInstance(CLSID_RenderEngine, NULL, CLSCTX_INPROC_SERVER,
        IID_IRenderEngine, (void**) &pRenderEngine);
    PreviewTL(pTL, pRenderEngine);
	pRenderEngine->ScrapIt();
	WriteTL(pTL, pRenderEngine, L"C:\\MyMovie.avi");

    // Clean up.
    pRenderEngine->Release();
	pMD->Release();
    pTL->Release();
    CoUninitialize();


	return 0;
}


