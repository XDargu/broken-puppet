// engine.cpp : Defines the entry point for the application.
//

#include "mcv_platform.h"
#include "iostatus.h"
#include "app.h"

// Global Variables:
HINSTANCE hInst;								// current instance

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{

	OutputDebugString("Program Start");
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	CApp &app = CApp::get();
	app.loadConfig();

	// Initialize global strings
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
		return FALSE;

	// If the app can't start, exit
	if (!app.create())
		return FALSE;

	// Main message loop:
  MSG msg;
  memset(&msg, 0, sizeof(MSG));
  while (msg.message != WM_QUIT) {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
		app.doFrame();
	}

  //sin(30);
	app.destroy();
	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon = 0; // LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ENGINE));
	wcex.hCursor = 0; //; LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName = 0; // MAKEINTRESOURCE(IDC_ENGINE);
	wcex.lpszClassName	= "MCVClass";
	wcex.hIconSm = 0; // LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
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

   CApp& app = CApp::get();
   
   // Define the client area
   RECT rc = { 0, 0, app.xres, app.yres };

   // We need to tell windows the size of the full windows, including border
   // so the rect is bigger
   AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

   // Create the actual window
   hWnd = CreateWindow("MCVClass", "MCV Engine 2014"
	   , WS_OVERLAPPEDWINDOW
	   , CW_USEDEFAULT, CW_USEDEFAULT		// Position
	   , rc.right - rc.left					// Width
	   , rc.bottom - rc.top					// Height
	   , NULL, NULL
	   , hInstance
	   , NULL);

   if (!hWnd)
      return FALSE;

   app.hWnd = hWnd;
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
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
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_LBUTTONUP:
		//::ReleaseCapture( );
		io.getButtons()[io.MOUSE_LEFT].setPressed(false, 0.f);
		break;
	case WM_LBUTTONDOWN:
		// Capturar el raton para que los eventos vengan a mi ventana
		// incluso cuando el raton esta fuera del area cliente
		//::SetCapture( hWnd );
		io.getButtons()[io.MOUSE_LEFT].setPressed(true, 0.f);
		break;
	case WM_RBUTTONUP:
		io.getButtons()[io.MOUSE_RIGHT].setPressed(false, 0.f);
		break;
	case WM_RBUTTONDOWN:
		io.getButtons()[io.MOUSE_RIGHT].setPressed(true, 0.f);
		break;
	case WM_MBUTTONUP:
		io.getButtons()[io.MOUSE_MIDDLE].setPressed(false, 0.f);
		break;
	case WM_MBUTTONDOWN:
		io.getButtons()[io.MOUSE_MIDDLE].setPressed(true, 0.f);
		break;
	/*case WM_KILLFOCUS:
		app.has_focus = false;
		break;
	case WM_SETFOCUS:
		app.has_focus = true;
		break;*/
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
