// DVDPlayer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <dshow.h>


int main(int argc, char* argv[])
{
	
	IDvdGraphBuilder *m_pIDvdGB;
	IMediaControl * m_pIMC;
	
	IGraphBuilder* m_pGraph;
	
	CoInitialize (NULL);
		// Create an instance of the DVD Graph Builder object.
   HRESULT hr;
   hr = CoCreateInstance(CLSID_DvdGraphBuilder,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IDvdGraphBuilder,
                          (void**)&m_pIDvdGB);

   // Build the DVD filter graph.
   AM_DVD_RENDERSTATUS	buildStatus;
   hr = m_pIDvdGB->RenderDvdVideoVolume(NULL, AM_DVD_HWDEC_PREFER   , &buildStatus);

   // Get a pointer to the filter graph manager.
   hr = m_pIDvdGB->GetFiltergraph(&m_pGraph) ;
       
   hr = m_pGraph->QueryInterface(IID_IMediaControl, (void**)&m_pIMC);
   
	
   m_pIMC->Run ();
   MessageBox (NULL, "stop playback", NULL, NULL);
	return 0;
}
