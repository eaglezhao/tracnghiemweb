#include <dmusici.h>
#include <stdio.h>

// Link with dxguid.lib

#include <initguid.h>

DEFINE_GUID(CLSID_Delay,
			0xAD90F22E, 0xC51D, 0x4F1C, 0xB4, 0x40, 0xC1, 0xF2, 0xA0, 0x1D, 0x5F, 0x1F);


#define NUM_PCHANNELS 1

#define SAFE_RELEASE(x) { if (x) { (x)->Release(); (x) = NULL; } }

void main(int argc, char *argv[])
{

	HRESULT hr;
	IDirectMusicLoader8			*pLoader = NULL;
	IDirectMusicPerformance8	*pPerformance = NULL;
	IDirectMusicSegment8		*pSegment = NULL;
	IDirectMusicAudioPath		*pAudioPath = NULL;
	IDirectSoundBuffer8			*pBuffer = NULL;

	WCHAR wstrPath[] = L"C:\\";
	WCHAR wstrFileName[] = L"Test.wav";

	CoInitialize(NULL);

	hr = CoCreateInstance(CLSID_DirectMusicLoader, 0, CLSCTX_INPROC, 
		IID_IDirectMusicLoader8, (void**)&pLoader);

	if (FAILED(hr))
	{
		fprintf(stderr, "Could not create Loader object.\n");
		goto CleanUp;
	}

	hr = CoCreateInstance(CLSID_DirectMusicPerformance, 0, CLSCTX_INPROC,
		IID_IDirectMusicPerformance8, (void**)&pPerformance);

	if (FAILED(hr))
	{
		fprintf(stderr, "Could not create Performance object.\n");
		goto CleanUp;
	}

	pPerformance->InitAudio(
		NULL,	// get IDirectMusic
		NULL,	// get IDirectSound
		NULL,	// Window handle
		DMUS_APATH_DYNAMIC_STEREO, // default audiopath
		NUM_PCHANNELS,		// PChannels
		DMUS_AUDIOF_ALL,	// Synth features
		NULL	// Audio params
	);

	// Set the default path
	pLoader->SetSearchDirectory(GUID_DirectMusicAllTypes, wstrPath, 0);

	// Load the file as a segment
	hr = pLoader->LoadObjectFromFile(CLSID_DirectMusicSegment, IID_IDirectMusicSegment8, 
			wstrFileName, (void**)&pSegment);
	if (FAILED(hr))
	{
		fprintf(stderr, "Could not open file.\n");
		goto CleanUp;
	}

	hr = pSegment->Download(pPerformance);

	// Add the effect to the buffer 
	hr = pPerformance->GetDefaultAudioPath(&pAudioPath);

	hr = pAudioPath->GetObjectInPath(DMUS_PCHANNEL_ALL, DMUS_PATH_BUFFER, 0, GUID_NULL, 0, 
			IID_IDirectSoundBuffer8, (void**)&pBuffer);

	// Temporarily deactivate the audio path.
	pAudioPath->Activate(FALSE);

	DSEFFECTDESC dsEffect;
	ZeroMemory(&dsEffect, sizeof(DSEFFECTDESC));
	dsEffect.dwSize = sizeof(DSEFFECTDESC);
	dsEffect.guidDSFXClass = CLSID_Delay;
	dsEffect.dwFlags = 0;

	DWORD dwResults;
	hr = pBuffer->SetFX(1, &dsEffect, &dwResults);

	hr = pAudioPath->Activate(TRUE);

	// Play the sound
	hr = pPerformance->PlaySegmentEx(
		pSegment, 
		NULL,	// reserved
		NULL,	// transition
		0,		// flags
		0,		// Start time
		NULL,	// receives segment state
		NULL,	// Object to stop (?)
		0		// Audio path, if not default
	);

	MessageBox(NULL, "Click OK to Stop", "DirectX Audio Demo", MB_OK);

	
CleanUp:
	pPerformance->Stop(0, 0, 0, 0);
	SAFE_RELEASE(pBuffer); 
	SAFE_RELEASE(pLoader);
	SAFE_RELEASE(pSegment);
	SAFE_RELEASE(pAudioPath);
	pPerformance->CloseDown();
	SAFE_RELEASE(pPerformance);

	CoUninitialize();


}


// Here's some code for setting effects on an echo. 

/*
	IDirectSoundFXEcho8	*pEffectDMO;
	DSFXEcho echoParams;

	hr = pBuffer->GetObjectInPath(GUID_DSFX_STANDARD_ECHO, 0, IID_IDirectSoundFXEcho8, (void**)&pEffectDMO);

	hr = pEffectDMO->GetAllParameters(&echoParams);
	echoParams.fLeftDelay = 1000;
	echoParams.fRightDelay = 1000;
	echoParams.fWetDryMix = .5;
	echoParams.fFeedback = DSFXECHO_FEEDBACK_MAX;
	hr = pEffectDMO->SetAllParameters(&echoParams);

*/
