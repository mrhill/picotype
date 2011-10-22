#include <windows.h>
#include "resource.h"

#include <babel/mem.h>
#include <babel/fixmath.h>
#include <babel/file.h>
#include <ptGC16.h>
#include <ptImage_jpeglib.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                        // current instance
TCHAR szTitle[MAX_LOADSTRING];          // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];    // The title bar text

DWORD       g_pal[256];
HBITMAP     g_hBmp = NULL;
HDC         g_hMemDC = NULL;
ptSprite*   g_pSprite = NULL;
ptSprite*   g_pSprite2 = NULL;
ptSprite*   g_pSprite3 = NULL;

int         g_spritesel = 0;

ptGC16      g_gc;
ptFont*     g_fontpool[2] = { 0, 0 };

int g_mousex = 0;
int g_mousey = 0;
int g_mousex1 = 0;
int g_mousey1 = 0;
int g_mousex2 = 0;
int g_mousey2 = 0;

void gctestrender(void)
{
    bbU16* const pBmp = (bbU16*) g_gc.GetBmp();

    for (int y=0; y<160; ++y)
        for (int x=0; x<160; ++x)
            *((bbU16*)pBmp + x + (y*160)) = (x>>3)+((y>>3)<<5);

    g_gc.Sprite( g_mousex, g_mousey, g_spritesel ? g_pSprite2 : g_pSprite3);
    g_spritesel = (g_spritesel+1)&1;
    g_gc.Sprite( g_mousex, g_mousey, g_pSprite);
}


ptSprite* LoadJPEGToSprite( const bbCHAR* const pFileName)
{
    ptSprite* pSprite = NULL;

    bbFILEH fileh = bbFileOpen( pFileName, 0);
    if (fileh)
    {
        const bbU32 jpegsize = (bbU32)bbFileExt(fileh);
        if (jpegsize != (bbU32)-1)
        {
            bbU8* const pBuf = (bbU8*) bbMemAlloc(jpegsize);
            if (pBuf)
            {
                if (bbFileRead( fileh, pBuf, jpegsize) == bbEOK)
                {
                    ptImage_jpeglib jpeg;
                    if (jpeg.Open( pBuf, jpegsize) == bbEOK)
                    {
                        ptSprite* const pSpriteJPEG = ptImageGetSprite( &jpeg);
                        if (pSpriteJPEG)
                        {
                            pSprite = ptSpriteConvert(pSpriteJPEG, 4);
                            bbMemFree(pSpriteJPEG);
                        }
                    }
                }
                bbMemFree(pBuf);
            }
        }
        bbFileClose(fileh);
    }

    return pSprite;
}

void DumpSprite(ptSprite* pSprite, char* pFName, char* varname)
{
    FILE* fh = fopen( pFName, "wb");

    if (fh)
    {
        /*
        fprintf( fh, "const unsigned char %s[] = {\n", varname);
        fprintf( fh, "0x00,0xA0,0x00,0x50,0x01,0x40,0x04,0x00,0,0,0,0,\n");
        int size = pSprite->bytepitch * pSprite->height;

        bbU8* pTmp = pSprite->data;

        while (size--)
        {
            if (size==0)
                fprintf(fh, "0x%02X\n", *(pTmp++));
            else
                fprintf(fh, "0x%02X,", *(pTmp++));
            if ((size % pSprite->bytepitch)==0)
                fprintf(fh, "\n");
        }

        fprintf(fh, "};\n");
        */

        static bbU8 hdr[] = { 0x00,0xA0,0x00,0x50,0x01,0x40,0x04,0x00,0,0,0,0 };
        fwrite( hdr, sizeof(hdr), 1, fh);
        fwrite( pSprite->data, pSprite->stride * pSprite->height, 1, fh);
        fclose(fh);
    }
}

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SPRITE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_SPRITE);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_SPRITE);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_SPRITE;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

void DestroyInstance()
{
    bbMemFreeNull( (void**)&g_pSprite2);
    bbMemFreeNull( (void**)&g_pSprite);
    bbMemFreeNull( (void**)&g_pScratch);
    bbDestroyFixMath();
    if (g_hMemDC) DeleteDC(g_hMemDC);
    if (g_hBmp) DeleteObject(g_hBmp);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;

    hInst = hInstance; // Store instance handle in our global variable

    int width = 160;
    int height = 160;

    hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW &~ WS_THICKFRAME,
        CW_USEDEFAULT, 0, 
        width, 
        height + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU),
        NULL, NULL, hInstance, NULL);

    if ( (!hWnd) ||
         (bbInitFixMath() != bbEOK))
    {
        return FALSE;
    }

    BITMAPINFO* pInfo = (BITMAPINFO*) malloc(sizeof(BITMAPINFOHEADER) + 3*4);
    memset( &pInfo->bmiHeader, 0, sizeof(BITMAPINFOHEADER));
    pInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pInfo->bmiHeader.biWidth = width;
    pInfo->bmiHeader.biHeight = -height;
    pInfo->bmiHeader.biPlanes = 1;
    pInfo->bmiHeader.biBitCount = 16;
    pInfo->bmiHeader.biCompression = BI_BITFIELDS;
    pInfo->bmiHeader.biSizeImage = 0;

    *(bbU32*)&pInfo->bmiColors[0] = 0x001FUL;
    *(bbU32*)&pInfo->bmiColors[1] = 0x03E0UL;
    *(bbU32*)&pInfo->bmiColors[2] = 0x7C00UL;

    bbU8* pBmp;
    g_hBmp = CreateDIBSection( GetDC(hWnd), pInfo, DIB_RGB_COLORS, (void**)&pBmp, NULL, 0);

    free(pInfo);
    if (!g_hBmp || !pBmp) return FALSE;

    // create a memory DC for selecting offscreen bitmap into
    g_hMemDC = CreateCompatibleDC( GetDC(hWnd) );

    if (!g_hMemDC)
    {
        DeleteObject(g_hBmp);
        return FALSE;
    }

    // Attach graphics context to offscreen bitmap

    g_gc.Init( g_pScratch, pBmp, width, width, height);
    g_gc.SetLogPal( g_pal);
    if ((g_pSprite = (ptSprite*) bbMemAlloc(ptSIZEOF_SPRITEHDR + 2*80*80)) == NULL)
    {
        DestroyInstance();
        return FALSE;
    }

    g_pSprite->width = 80;
    g_pSprite->height = 80;
    g_pSprite->colfmt = ptCOLFMT_RGB565;
    g_pSprite->flags = 0;

    FILE* fh = fopen("1.bmp", "rb");
    if (!fh)
    {
        for (UINT k=0; k<80*80/2; ++k)
            g_pSprite->data[k] = k;
    }
    else
    {
        bbU8 buf[3*80*80];

        fseek( fh, 54, 0);
        fread( buf, 3*80*80, 1, fh);

        for (UINT k=0; k<80*80; ++k)
        {
            *((bbU16*)(&g_pSprite->data[0]) + k) = 
                (buf[k*3]>>3) | ((buf[k*3+1]>>3)<<5) | ((buf[k*3+2]>>3)<<10);
        }

        fclose(fh);
    }

    g_pSprite2 = LoadJPEGToSprite( bbT("dream.jpg"));
    g_pSprite3 = LoadJPEGToSprite( bbT("dream2.jpg"));

    /*

    g_pSprite2 = LoadJPEGToSprite( bbT("p0.jpg"));

    bbU16 o = g_pSprite2->height;
    g_pSprite2->height = 80;
    DumpSprite(g_pSprite2, "p0.dat", "p0");
    g_pSprite2->height = o;

    static bbCHAR* zz[] = { 
        bbT("p1.jpg"),bbT("p1.dat"),bbT("p1"),
        bbT("p2.jpg"),bbT("p2.dat"),bbT("p2"),
        bbT("p3.jpg"),bbT("p3.dat"),bbT("p3"),
        bbT("p4.jpg"),bbT("p4.dat"),bbT("p4"),
        bbT("p5.jpg"),bbT("p5.dat"),bbT("p5")
    };

    for (int z=0; z<5; z++)
    {
        bbMemFreeNull( (void**) &g_pSprite3);

        g_pSprite3 = LoadJPEGToSprite( zz[z*3]);

        o = g_pSprite3->height;
        g_pSprite3->height = 80;
        DumpSprite(g_pSprite3, zz[z*3+1], zz[z*3+2]);
        g_pSprite3->height = o;
    }
*/    
    gctestrender();
    
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR szHello[MAX_LOADSTRING];
	LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);

	switch (message) 
	{
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_ABOUT:
				   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				   break;
				case IDM_EXIT:
				   DestroyWindow(hWnd);
				   break;
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_PAINT:
			/* Select offscreen bitmap into memory DC */
            SelectObject( g_hMemDC, g_hBmp);
            hdc = BeginPaint(hWnd, &ps);
            BitBlt( hdc, 0, 0, g_gc.GetPixelWidth(), g_gc.GetPixelHeight(), g_hMemDC, 0, 0, SRCCOPY);
			EndPaint(hWnd, &ps);
			break;
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
            gctestrender();
            InvalidateRect( hWnd, NULL, FALSE);
            break;

        case WM_DESTROY:
            DestroyInstance();
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}
