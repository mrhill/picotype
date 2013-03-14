#include <windows.h>
#include "resource.h"

#include <babel/mem.h>
#include <babel/fixmath.h>
#include <ptGC8.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];

const bbCHAR* text[] =
{
    bbT("AB\0\2\4\0\3\xFFtext\0\1"),
    bbT("Uh yeah. bababa\0\1"),
    bbT("   mmmm\0\1"),
    bbT("akvmevmlewmvlewmvl\0\1"),
    bbT("AKVMEVMLEWMVLEWMVL\0\1"),
    bbT("wqdwqdwqdwqdwqdwdwqd\0\1"),
    bbT("\0\1"),
    bbT("void gc8testrender(void)\0\1"),
    bbT("{\0\1"),
    //bbT("／・・・・・・・・・・・・・\0\1"),
    //bbT("漢字\0\1"),
    //bbT("リンクはこのページへご自由にどうぞ・承諾不要です\0\1"),
    //bbT("漢字\0\1"),
    //bbT("／・・・・・・・・・・・・・\0\1"),
    bbT("\0\1"),
    bbT("    int x,y,j;\0\1"),
    bbT("\0\1"),
    bbT("    // Draw test pattern\0\1"),
    bbT("    for (y=0; y<height; y++)\0\1"),
    bbT("        for (x=0; x<width; x++)\0\1"),
    bbT("            *(pBmp + x + y*width) = x&y;\0\1"),
    bbT("\0\1"),
    bbT("    for (j=0; j<bbSINTABPER; j++)\0\1"),
    bbT("    {\0\1"),
    bbT("        g_gc8.Point( width*4 + (((long)bbgpSinTab[j] * 200) >> (bbSINTABPREC+1-3)),\0\1"),
    bbT("                     height*4 + (((long)bbgpSinTab[j+bbSINTABPER/4] * 200) >> (bbSINTABPREC+1-3)),\0\1"),
    bbT("                     255 );\0\1"),
    bbT("    }\0\1"),
    bbT("\0\1"),
    bbT("    g_gc8.SetPixelWidth( 1);\0\1"),
    bbT("    g_gc8.Clear(200);\0\1"),
    bbT("    g_gc8.SetPixelWidth( width);\0\1"),
    bbT("    const int oldclipminx = m_unit_clipminx;\0\1"),
    bbT("    int x = pTextbox->unit_x - pTextbox->unit_scrollx;\0\1"),
    bbT("    m_unit_clipminx = (pTextbox->unit_x >= oldclipminx) ? pTextbox->unit_x : oldclipminx;\0\1"),
    bbT("    \0\1"),
    bbT("    const int oldclipmaxx = m_unit_clipmaxx;\0\1"),
    bbT("    m_unit_clipmaxx = pTextbox->unit_x + pTextbox->unit_width;\0\1"),
    bbT("    if (m_unit_clipmaxx > oldclipmaxx)  m_unit_clipmaxx = oldclipmaxx;\0\1"),
    bbT("\0\1"),
    bbT("    const int oldclipminy = m_unit_clipminy;\0\1"),
    bbT("    int y = pTextbox->unit_y - pTextbox->unit_scrolly;\0\1"),
    bbT("    m_unit_clipminy = (pTextbox->unit_y >= oldclipminy) ? pTextbox->unit_y : oldclipminy;\0\1"),
    bbT("\0\1"),
    bbT("    const int oldclipmaxy = m_unit_clipmaxy;\0\1"),
    bbT("    m_unit_clipmaxy = pTextbox->unit_y + pTextbox->unit_height;\0\1"),
    bbT("    if (m_unit_clipmaxy > oldclipmaxy)  m_unit_clipmaxy = oldclipmaxy;\0\1"),
    bbT("\0\1"),
    bbT("    const bbCHAR* const* pLines = pTextbox->pLines;\0\1"),
    bbT("    const bbUINT fontheight = m_ppFontPool[ pTextbox->font ]->GetHeight() << ptGCEIGHTX;\0\1"),
    bbT("    const int fillx = bbMAX(x, pTextbox->unit_x);\0\1"),
    bbT("\0\1"),
    bbT("    if (pTextbox->unit_scrollx < 0)\0\1"),
    bbT("    {\0\1"),
    bbT("        FillBox(pTextbox->unit_x, pTextbox->unit_y, -pTextbox->unit_scrollx, pTextbox->unit_height, pTextbox->bgpen);\0\1"),
    bbT("    }\0\1"),
    bbT("    if (pTextbox->unit_scrolly < 0)\0\1"),
    bbT("    {\0\1"),
    bbT("        FillBox(fillx, pTextbox->unit_y, pTextbox->unit_width, -pTextbox->unit_scrolly, pTextbox->bgpen);\0\1"),
    bbT("    }\0\1"),
    bbT("\0\1"),
    NULL
};

ptTextBox textbox;

DWORD g_pal[256];

HBITMAP g_hBmp = NULL;
HDC     g_hMemDC = NULL;

ptGC8   g_gc8;
ptFont* g_fontpool[2] = { 0, 0 };

int     g_mousex = 0;
int     g_mousey = 0;
int     g_mousex1 = 0;
int     g_mousey1 = 0;
int     g_mousex2 = 0;
int     g_mousey2 = 0;

ptCoord g_poly[5] =
{
    -8, 0,
    99, 04,
    50, 50,
    80,  122,
    0,  99,
/*
    0, 0,
    99, 04,
    99, 99,
    40,  122,
    0,  99,
*/
};

void gc8testrender(void)
{
    bbU8* const pBmp = g_gc8.GetBmp();
    const int width  = (int)g_gc8.GetPixelWidth();
    const int height = (int)g_gc8.GetPixelHeight();

    int x,y,j;

    // Draw test pattern
    for (y=0; y<height; y++)
        for (x=0; x<width; x++)
            *(pBmp + x + y*width) = x&y;

    for (j=0; j<bbSINTABPER; j++)
    {
        g_gc8.Point( width*4 + (((long)bbgpSinTab[j] * 200) >> (bbSINTABPREC+1-3)),
                     height*4 + (((long)bbgpSinTab[j+bbSINTABPER/4] * 200) >> (bbSINTABPREC+1-3)),
                     255 );
    }

    g_gc8.SetPixelWidth( 1);
    g_gc8.Clear(200);
    g_gc8.SetPixelWidth( width);

    g_gc8.HLine(0,0,0*8,255);
    g_gc8.HLine(0,8,1*8,255);
    g_gc8.HLine(0,16,3*8,255);
    g_gc8.HLine(0,24,2*8,255);

    int minx = 100*8;
    int maxx = 116*8;
    int miny = 142*8;
    int maxy = 147*8;

    g_gc8.Box( minx-8, miny-8, maxx-minx+16, maxy-miny+16, 128);
    g_gc8.SetClipBox(minx, miny, maxx, maxy);
    g_gc8.Text( 88*8,140*8,bbT("AB\27\2\4\27\3\xFFtext"), 0, 255);
    g_gc8.ResetClipBox();

    minx = 50*8;
    maxx = 216*8;
    miny = 40*8;
    maxy = 180*8;

    g_gc8.Box( minx-8, miny-8, maxx-minx+16, maxy-miny+16, 64);
    g_gc8.SetClipBox(minx, miny, maxx, maxy);

    g_gc8.ResetClipBox();
    g_gc8.Box( textbox.unit_x-8, textbox.unit_y-8, textbox.unit_width+16, textbox.unit_height+16, 128);
    g_gc8.MarkupTextBox( &textbox);

    #ifdef bbDEBUG
    g_gc8.DebugAA();
    #endif

    g_gc8.Line( g_mousex1*8, g_mousey1*8, g_mousex2*8, g_mousey2*8, 128 | ptPEN_AA);

    int w;
    ptCoord poly[5];

    for (w=0; w<5; w++)
    {
        poly[w].x=g_poly[w].x*2+g_mousex1*4;
        poly[w].y=g_poly[w].y*2+g_mousey1*4;
    }
    //g_gc8.LinearRing( poly, 5, 255 | ptPEN_AA);

    for (w=0; w<5; w++)
    {
        poly[w].x=g_poly[w].x*2+g_mousex1*8;
        poly[w].y=g_poly[w].y*2+g_mousey1*8;
    }
    g_gc8.LineString( poly, 5, 255 | ptPEN_COL);

    for (w=0; w<5; w++)
    {
        poly[w].x=g_poly[w].x*2*4+g_mousex*4;
        poly[w].y=g_poly[w].y*2*4+g_mousey*4;
    }
    g_gc8.Polygon( poly, 5, textbox.bgpen );
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
	LoadString(hInstance, IDC_TESTGC, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_TESTGC);

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

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_TESTGC);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_TESTGC;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;

    hInst = hInstance; // Store instance handle in our global variable

    int width = 640;
    int height = 480;

    hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW &~ WS_THICKFRAME,
        CW_USEDEFAULT, 0,
        width,
        height + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU),
        NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }

    if (bbInitFixMath() != bbEOK)
    {
        return FALSE;
    }

    BITMAPINFO* pInfo = (BITMAPINFO*) malloc(sizeof(BITMAPINFOHEADER) * 257*4);
    memset( &pInfo->bmiHeader, 0, sizeof(BITMAPINFOHEADER));
    pInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pInfo->bmiHeader.biWidth = width;
    pInfo->bmiHeader.biHeight = -height;
    pInfo->bmiHeader.biPlanes = 1;
    pInfo->bmiHeader.biBitCount = 8;
    pInfo->bmiHeader.biCompression = BI_RGB;
    pInfo->bmiHeader.biSizeImage = 0;

    for (int i=0; i<256; i++)
    {
        pInfo->bmiColors[i].rgbBlue = i;
        pInfo->bmiColors[i].rgbGreen = i;
        pInfo->bmiColors[i].rgbRed = i;
        pInfo->bmiColors[i].rgbReserved = 0;
        g_pal[i] = pInfo->bmiColors[i].rgbRed | (pInfo->bmiColors[i].rgbGreen<<8) | (pInfo->bmiColors[i].rgbBlue<<16);
    }
    *(bbU32*)&pInfo->bmiColors[256] = 0xFFFFFFFFUL;

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

    const bbU8 aapal[] = { 0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff };

    g_gc8.Init( g_pScratch, pBmp, width, width, height);
    g_gc8.SetLogPal( g_pal);
    g_gc8.SetPal( g_pal, aapal);

    // Load and attach fonts

    g_fontpool[0] = new ptFont;
    if (g_fontpool[0]->LoadZapFont( "C:\\cygwin\\home\\mrhill\\coding\\gr\\samples\\testgr\\0" ) != bbEOK)
    //if (g_fontpool[0]->LoadZapFont( "C:\\Program Files\\RedSquirrel Release\\Hostfs\\Disk370\\!Zap\\Fonts\\14x21\\Luc.dat" ) != bbEOK)
    //if (g_fontpool[0]->LoadZapFont( "C:\\Program Files\\RedSquirrel Release\\Hostfs\\Disk370\\!Zap\\Fonts\\04x06\\C.dat" ) != bbEOK)
    //if (g_fontpool[0]->LoadZapFont( "C:\\Program Files\\RedSquirrel Release\\Hostfs\\Disk370\\!Zap\\Fonts\\08x08\\C.dat" ) != bbEOK)
    //if (g_fontpool[0]->LoadZapFont( bbT("C:\\cygwin\\home\\mrhill\\gfx\\samples\\testgc\\K16x16.dat") ) != bbEOK)
    {
        return FALSE;
    }

    // prepare data for gc8testrender()

    textbox.unit_x = 88*8;
    textbox.unit_y = 22*8;
    textbox.unit_width  = 880*8;
    textbox.unit_height = 640*8;
    textbox.unit_scrollx = 0;
    textbox.unit_scrolly = 16*8;
    textbox.unit_linespace = 8*4;
    textbox.font = 0;
    textbox.textfgcol = 255;
    textbox.textbgpen = ptPEN_COL | (0<<ptPENBITPOS_OPT2) | 64;
    textbox.bgpen = ptPEN_COL | (0<<ptPENBITPOS_OPT2);
    textbox.pLines = text;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

void DestroyInstance()
{
    bbMemFreeNull( (void**) &g_pScratch);
    bbDestroyFixMath();
    if (g_hMemDC) DeleteDC(g_hMemDC);
    if (g_hBmp) DeleteObject(g_hBmp);
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
                case IDM_CLS:
                   g_gc8.Clear(0);
                   InvalidateRect( hWnd, NULL, FALSE);
                   break;
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
            BitBlt( hdc, 0, 0, g_gc8.GetPixelWidth(), g_gc8.GetPixelHeight(), g_hMemDC, 0, 0, SRCCOPY);
			EndPaint(hWnd, &ps);
			break;
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
            g_mousex = lParam & 0xFFFF,
            g_mousey = lParam >> 16;

            if (wParam & MK_LBUTTON)
            {
                const UINT t = textbox.bgpen >> ptPENBITPOS_OPT2;
                if (t>0) textbox.bgpen = ptPEN_TRANS | ((t-1)<<ptPENBITPOS_OPT2);

                g_mousex1 = lParam & 0xFFFF,
                g_mousey1 = lParam >> 16;
            }
            else if (wParam & MK_RBUTTON)
            {
                const UINT t = textbox.bgpen >> ptPENBITPOS_OPT2;
                if (t<7) textbox.bgpen = ptPEN_TRANS | ((t+1)<<ptPENBITPOS_OPT2);

                g_mousex2 = lParam & 0xFFFF,
                g_mousey2 = lParam >> 16;
            }

            textbox.unit_scrollx = (lParam & 0xFFFF)*8 - textbox.unit_width/2;
            textbox.unit_scrolly = (lParam >> 16)*8 - textbox.unit_height/2;
            gc8testrender();

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
