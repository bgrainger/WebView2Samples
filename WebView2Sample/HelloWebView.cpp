// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c

#include <windows.h>
#include <stdlib.h>
#include <string>
#include <tchar.h>
#include <wrl.h>
// include WebView2 header
#include "WebView2.h"


using namespace Microsoft::WRL;

// Global variables

// The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("WebView sample");

HINSTANCE hInst;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Pointer to WebView window
static ComPtr<IWebView2WebView> webviewWindow;

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			_T("Windows Desktop Guided Tour"),
			NULL);

		return 1;
	}

	// Store instance handle in our global variable
	hInst = hInstance;

	// The parameters to CreateWindow explained:
	// szWindowClass: the name of the application
	// szTitle: the text that appears in the title bar
	// WS_OVERLAPPEDWINDOW: the type of window to create
	// CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
	// 500, 100: initial size (width, length)
	// NULL: the parent of this window
	// NULL: this application does not have a menu bar
	// hInstance: the first parameter from WinMain
	// NULL: not used in this application
	HWND hWnd = CreateWindow(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		1200, 900,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd)
	{
		MessageBox(NULL,
			_T("Call to CreateWindow failed!"),
			_T("Windows Desktop Guided Tour"),
			NULL);

		return 1;
	}

	// The parameters to ShowWindow explained:
	// hWnd: the value returned from CreateWindow
	// nCmdShow: the fourth parameter from WinMain
	ShowWindow(hWnd,
		nCmdShow);
	UpdateWindow(hWnd);

	// <-- WebView2 sample code starts here -->

	// Step 3 - Create a single WebView within the parent window
	// Known issue - app needs to run on PerMonitorV2 DPI awareness for WebView to look properly
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	// Locate the browser and set up the environment for WebView
	CreateWebView2EnvironmentWithDetails(nullptr, nullptr, WEBVIEW2_RELEASE_CHANNEL_PREFERENCE_CANARY, nullptr,
		Callback<IWebView2CreateWebView2EnvironmentCompletedHandler>(
			[hWnd](HRESULT result, IWebView2Environment* env) -> HRESULT {

				// Create a WebView, whose parent is the main window hWnd
				env->CreateWebView(hWnd, Callback<IWebView2CreateWebViewCompletedHandler>(
					[hWnd](HRESULT result, IWebView2WebView* webview) -> HRESULT {
					if (webview != nullptr) {
						webviewWindow = webview;
					}

					// Add a few settings for the webview
					// this is a redundant demo step as they are the default settings values
					IWebView2Settings* Settings;
					webviewWindow->get_Settings(&Settings);
					Settings->put_IsScriptEnabled(TRUE);
					Settings->put_AreDefaultScriptDialogsEnabled(TRUE);
					Settings->put_IsWebMessageEnabled(TRUE);

					// Resize WebView to fit the bounds of the parent window
					RECT bounds;
					GetClientRect(hWnd, &bounds);
					webviewWindow->put_Bounds(bounds);

					// Step 4 - Navigation events
					// register an IWebView2NavigationStartingEventHandler to cancel any non-https navigation
					EventRegistrationToken token;
					webviewWindow->add_NavigationStarting(Callback<IWebView2NavigationStartingEventHandler>(
						[](IWebView2WebView* webview, IWebView2NavigationStartingEventArgs * args) -> HRESULT {
							IWebView2HttpRequestHeaders* headers = nullptr;
							args->get_RequestHeaders(&headers);
							HRESULT hr = headers->SetHeader(L"X-Test", L"test");
							headers->Release();
							return S_OK;
						}).Get(), &token);

					// Schedule an async task to navigate to Bing
					webviewWindow->Navigate(L"https://www.bing.com/");

					// Step 5 - Scripting


					// Step 6 - Communication between host and web content


					return S_OK;
				}).Get());
			return S_OK;
		}).Get());

	// <-- WebView2 sample code ends here -->

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR greeting[] = _T("Hello, Windows desktop!");

	switch (message)
	{
	case WM_SIZE:
		if (webviewWindow != nullptr) {
			RECT bounds;
			GetClientRect(hWnd, &bounds);
			webviewWindow->put_Bounds(bounds);
		};
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}
