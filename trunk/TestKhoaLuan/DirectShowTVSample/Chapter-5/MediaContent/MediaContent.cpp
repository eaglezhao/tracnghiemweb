// MediaContent.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <dshow.h>
#include <qnetwork.h>


 
	
	

int main(int argc, char* argv[])
{
	IMediaControl *pMC = NULL;
	IGraphBuilder *pGB = NULL;
	IBaseFilter   *pBF = NULL;
	IAMMediaContent *pAMMC = NULL;
	BSTR pBSTR = NULL;

	CoInitialize (NULL);

	// Get the interface for DirectShow's GraphBuilder
    CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGB);

    // Have the graph construct its the appropriate graph automatically
       
    // QueryInterface for DirectShow interfaces
    pGB->QueryInterface(IID_IMediaControl, (void **)&pMC);
	pMC->RenderFile (L"C:\\My Documents\\My MusicMark_Pistel-Pistel-3-Skin_Up.mp3");
	pGB->FindFilterByName (L"MPEG-1 Stream Splitter", &pBF);
	pBF->QueryInterface (IID_IAMMediaContent, (void **)&pAMMC);
	
	pAMMC->get_AuthorName (&pBSTR);
	printf ("AuthorName: %S \n", pBSTR);
	
	pAMMC->get_Title (&pBSTR);
	printf ("Title: %S \n", pBSTR);

	pAMMC->get_Description(&pBSTR);
	printf ("Description: %S \n", pBSTR);

	pAMMC->get_Copyright(&pBSTR);
	printf ("Copyright: %S \n", pBSTR);

	pAMMC->get_Rating (&pBSTR);
	printf ("Rating: %S \n", pBSTR);

	pAMMC->get_BaseURL(&pBSTR);
	printf ("BaseURL: %S \n", pBSTR);

    pAMMC->get_LogoURL(&pBSTR);
	printf ("LogoURL: %S \n", pBSTR);

	pAMMC->get_LogoIconURL(&pBSTR);
	printf ("LogoIconURL: %S \n", pBSTR);

	pAMMC->get_MoreInfoURL(&pBSTR);
	printf ("MoreInfoURL: %S \n", pBSTR);

	pAMMC->get_MoreInfoBannerImage(&pBSTR);
	printf ("MoreInfoBannerImage: %S \n", pBSTR);

	pAMMC->get_MoreInfoBannerURL(&pBSTR);
	printf ("MoreInfoBannerURL: %S \n", pBSTR);

	pAMMC->get_MoreInfoText(&pBSTR);
	printf ("MoreInfoText: %S \n", pBSTR);






 
	return 0;
}

