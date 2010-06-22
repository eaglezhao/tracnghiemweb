// filerender.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <dshow.h>

int main(int argc, char* argv[])
{

IGraphBuilder *pGraph;
IMediaControl *pMediaControl;

CoInitialize (NULL); // Initialize COM

//Create an instance of the Filter Graph Manager and obtain a pointer to the GraphBuilder
CoCreateInstance (CLSID_FilterGraph, NULL, CLSCTX_INPROC,     IID_IGraphBuilder, (void **) &pGraph);

//obtain a pointer to the MediaControl
pGraph->QueryInterface (IID_IMediaControl, (void **) &pMediaControl);

//Build the graph based on the file format type
pMediaControl->RenderFile (L"C:\\Program Files\\Microsoft Money\\Media\\STDIUE2.AVI");

//Run the graph
pMediaControl->Run ();

//block
MessageBox (NULL, "Click to terminate", "DirectShow", MB_OK);

//Release
pMediaControl->Release ();
pGraph->Release ();
CoUninitialize ();

return 0;
}

