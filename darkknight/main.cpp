#include <windows.h>
#include "cppwebpage.h"

#include "include/opencv2/core/core.hpp"
#include "include/opencv2/highgui/highgui.hpp"
#include "include/opencv2/imgproc/imgproc.hpp"
#include "include/zbar.h"
#include <iomanip>


// A running count of how many windows we have open that contain a browser object
unsigned char WindowCount = 0;

// The class name of our Window to host the browser. It can be anything of your choosing.
static const TCHAR	ClassName[] = L"Browser";

using namespace std;
using namespace cv;
using namespace zbar;

#ifdef _DEBUG
#pragma comment(lib, "lib/opencv_core246d.lib")
#pragma comment(lib, "lib/opencv_highgui246d.lib")
#pragma comment(lib, "lib/libtiffd.lib")
#pragma comment(lib, "lib/libpngd.lib")
#pragma comment(lib, "lib/libjpegd.lib")
#pragma comment(lib, "lib/libjasperd.lib")
#pragma comment(lib, "lib/IlmImfd.lib")
#pragma comment(lib, "lib/zlibd.lib")
#pragma comment(lib, "Vfw32.Lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "lib/libzbar-0.lib")
#pragma comment(lib, "lib/opencv_imgproc246d.lib")
#else
#pragma comment(lib, "lib/opencv_core246.lib")
#pragma comment(lib, "lib/opencv_highgui246.lib")
#pragma comment(lib, "lib/libtiff.lib")
#pragma comment(lib, "lib/libpng.lib")
#pragma comment(lib, "lib/libjpeg.lib")
#pragma comment(lib, "lib/libjasper.lib")
#pragma comment(lib, "lib/IlmImf.lib")
#pragma comment(lib, "lib/zlib.lib")
#pragma comment(lib, "Vfw32.Lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "lib/libzbar-0.lib")
#pragma comment(lib, "lib/opencv_imgproc246.lib")
#endif

wchar_t *GetWC(char *c)
{
    size_t cSize = strlen(c)+1;
    wchar_t* wc = new wchar_t[cSize];
    mbstowcs (wc, c, cSize);

    return wc;
}

wchar_t *read_cam(void)
{
	char window_name[] = "Scan ORcode or Barcode...";

	VideoCapture cap(0);
    if (!cap.isOpened()) {
        MessageBox(0, L"Cannot open Camera.", L"ERROR", MB_OK);
        exit(EXIT_FAILURE);
    }
    //cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
    //cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);

    namedWindow(window_name, CV_WINDOW_AUTOSIZE);
    
    // Create a zbar reader
    ImageScanner scanner;
    
    // Configure the reader
    scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

	for (;;) {
        // Capture an OpenCV frame
        cv::Mat frame, frame_grayscale;
        cap >> frame;
        cap >> frame;

        // Convert to grayscale
        cvtColor(frame, frame_grayscale, CV_BGR2GRAY);

        // Obtain image data
        int width = frame_grayscale.cols;
        int height = frame_grayscale.rows;
        uchar *raw = (uchar *)(frame_grayscale.data);

        // Wrap image data
        Image image(width, height, "Y800", raw, width * height);

        // Scan the image for barcodes
        //int n = scanner.scan(image);
        scanner.scan(image);

        // Show captured frame, now with overlays!
        imshow(window_name, frame);

        // Extract results
        int counter = 0;
        for (Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol) {
			destroyWindow(window_name);
			return GetWC(strdup(symbol->get_data().c_str()));
        }
                                                                                                                                                          
        // clean up
        image.set_data(NULL, 0);
        
        waitKey(30);
    }

	return 0;
}

/****************************** WindowProc() ***************************
 * Our message handler for our window to host the browser.
 */

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_SIZE:
		{
			// Resize the browser object to fit the window
			ResizeBrowser(hwnd, LOWORD(lParam), HIWORD(lParam));
			return(0);
		}

		case WM_CREATE:
		{
			// Embed the browser object into our host window. We need do this only
			// once. Note that the browser object will start calling some of our
			// IOleInPlaceFrame and IOleClientSite functions as soon as we start
			// calling browser object functions in EmbedBrowserObject().
			if (EmbedBrowserObject(hwnd)) return(-1);

			// Another window created with an embedded browser object
			++WindowCount;

			// Success
			return(0);
		}

		case WM_DESTROY:
		{
			// Detach the browser object from this window, and free resources.
			UnEmbedBrowserObject(hwnd);

			// One less window
			--WindowCount;

			// If all the windows are now closed, quit this app
			if (!WindowCount) PostQuitMessage(0);

			return(TRUE);
		}


	}

	return(DefWindowProc(hwnd, uMsg, wParam, lParam));
}

/****************************** WinMain() ***************************
 * C program entry point.
 *
 * This creates a window to host the web browser, and displays a web
 * page.
 */

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hInstNULL, LPSTR lpszCmdLine, int nCmdShow)
{
	MSG			msg;

	// Prepare for QR code recognization
    wchar_t *query = read_cam();
	wchar_t zcy[] = L"https://secure.imzcy.com/echo/";
	wchar_t *final = (wchar_t *)malloc(wcslen(zcy) * sizeof(wchar_t) + wcslen(query) * sizeof(wchar_t) + 2);
	wcscpy(final, zcy);
	wcscat(final, query);
	free(query);

	// Initialize the OLE interface. We do this once-only.
	if (OleInitialize(NULL) == S_OK)
	{
		WNDCLASSEX		wc;

		// Register the class of our window to host the browser. 'WindowProc' is our message handler
		// and 'ClassName' is the class name. You can choose any class name you want.
		ZeroMemory(&wc, sizeof(WNDCLASSEX));
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.hInstance = hInstance;
		wc.lpfnWndProc = WindowProc;
		wc.lpszClassName = &ClassName[0];
		RegisterClassEx(&wc);

		// Create a window. NOTE: We embed the browser object duing our WM_CREATE handling for
		// this window.
		if ((msg.hwnd = CreateWindowEx(0, &ClassName[0], L"zcy's Website", WS_OVERLAPPEDWINDOW,
						CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
						HWND_DESKTOP, NULL, hInstance, 0)))
		{
			DisplayHTMLPage(msg.hwnd, final);

			// Show the window.
			ShowWindow(msg.hwnd, nCmdShow);
			UpdateWindow(msg.hwnd);
		}
		
		// Do a message loop until WM_QUIT.
		while (GetMessage(&msg, 0, 0, 0) == 1)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		free(final);
		// Free the OLE library.
		OleUninitialize();

		return(0);
	}
	
	free(final);
	MessageBox(0, L"Can't open OLE!", L"ERROR", MB_OK);
	return(-1);
}
