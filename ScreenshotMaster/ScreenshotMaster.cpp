/*
Part 1 - write a program, no UI, that takes a screenshotand writes it to .bmp.
Part 2 - modify the program.Take a screenshot, sleep 10 seconds, take another 
screenshot.Pixels that are NOT the same - make them green, and write the new bmp.
*/

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <windows.h>
#include <vector>
#include <gdiplus.h>
#include <fstream>
#include <shtypes.h> // DEV_SCALE_FACTOR
#include <assert.h>
#include <ctime>    
#include <chrono>
#include <algorithm>
#include <thread>
#include <winuser.h>
#include <cmath>
#include "Helper.h"

// https://superkogito.github.io/blog/CaptureScreenUsingGdiplus.html
// https://www.codeproject.com/Articles/2841/How-to-replace-a-color-in-a-HBITMAP

using namespace Gdiplus;

#pragma comment(lib, "Gdiplus.lib") 
namespace app 
{
    class Bitmap
    {
    private:
        HANDLE hDIB;
        HDC hwindowDC;
        HDC hwindowCompatibleDC;
        HWND hWnd;
        HBITMAP hBitmap;
        BITMAP bitmap;
        BITMAPINFOHEADER bi;
        char* bits;
    public:
        Bitmap(HWND hWnd, HBITMAP hB = NULL) :
            hWnd(hWnd)
        {
            hwindowDC = GetDC(hWnd);
            hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
            if (hB != NULL)   // creation mode
            {
                hBitmap = hB;
            }
            bits = nullptr;
        }
        ~Bitmap()
        {
            DeleteDC(hwindowCompatibleDC);
            ReleaseDC(hWnd, hwindowDC);
            /*
            if (bits != nullptr)
            {
                delete bits;
            }*/
        }

        void setBITMAP(BITMAP b)
        {
            bitmap = b;
            allocateBits(b.bmHeight * b.bmWidth);
            if (hBitmap == NULL)
                hBitmap = CreateCompatibleBitmap(hwindowDC, b.bmWidth, b.bmHeight);

            bi = createBitmapHeader(b.bmWidth, b.bmHeight);
        }

        BITMAP getBITMAP() { return bitmap; }
        HDC getCompatibleDC() { return hwindowCompatibleDC; }
        HDC getDC() { return hwindowDC; }
        HBITMAP getHBitmap() { return hBitmap; }
        BITMAPINFOHEADER* getBitmapInfoHeader() { return &bi;  }
        char* getBits() { return bits; }

    private:
        void allocateBits(size_t size) //throw(std::bad_alloc)
        {
            //bits = new char[size];
            DWORD dwBmpSize = 
                ((bitmap.bmWidth * bi.biBitCount + 31) / 32) * 4 * bitmap.bmHeight;
            hDIB = GlobalAlloc(GHND, dwBmpSize);
            bits = (char*)GlobalLock(hDIB);


            GlobalUnlock(hDIB);
        }
        BITMAPINFOHEADER createBitmapHeader(int width, int height)
        {
            BITMAPINFOHEADER  bi;

            // create a bitmap
            bi.biSize = sizeof(BITMAPINFOHEADER);
            bi.biWidth = width;
            bi.biHeight = -height;  //this is the line that makes it draw upside down or not
            bi.biPlanes = 1;
            bi.biBitCount = 32;
            bi.biCompression = BI_RGB;
            bi.biSizeImage = 0;
            bi.biXPelsPerMeter = 0;
            bi.biYPelsPerMeter = 0;
            bi.biClrUsed = 0;
            bi.biClrImportant = 0;

            return bi;
        }
    };
}


class GdiManager
{
private:
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;

public:
    GdiManager()
    {  
        // Initialize GDI+.
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    }

    ~GdiManager()
    {
        GdiplusShutdown(gdiplusToken);
    }
};

class ScreenshotMaster
{
private:
    HWND hWnd;
    std::wstring prefix;
    char* lpbitmapPrevious;
    char* originalSecond; // TODO removal
    int heightPrevious; // TODO removal
    int widthPrevious; // TODO removal
    HBITMAP screenshot[2];
public:
    ScreenshotMaster(std::wstring prefix)
        :prefix(prefix)
    {
        // get the bitmap handle to the bitmap screenshot
        hWnd = GetDesktopWindow();
        lpbitmapPrevious = NULL;
        heightPrevious = 0;
        widthPrevious = 0;
    }

    std::wstring TakeScreenshots() throw()
    {
        screenshot[0] = GdiPlusScreenCapture();
        std::chrono::seconds timespan(2); // or whatever
        std::this_thread::sleep_for(timespan);
        screenshot[1] = GdiPlusScreenCapture();
        if (screenshot[0] != NULL && screenshot[1] != NULL)
        {
            MarkDifferences(screenshot[0], screenshot[1]);
            SaveScreenshot(screenshot[0], 0);
            SaveScreenshot(screenshot[1], 0);
        }

        return std::wstring();
    }

    void MarkDifferences(HBITMAP sourceScreenshot, HBITMAP destScreenshot)
    {
        std::unique_ptr<app::Bitmap> sourceObj =
            std::make_unique<app::Bitmap>(hWnd, sourceScreenshot);

        std::unique_ptr<app::Bitmap> destObj =
            std::make_unique<app::Bitmap>(hWnd, sourceScreenshot);

        BITMAP sourceBITMAP;
        GetObject(sourceScreenshot, sizeof(BITMAP), &sourceBITMAP);
        sourceObj->setBITMAP(sourceBITMAP);

        BITMAP destBITMAP;
        GetObject(sourceScreenshot, sizeof(BITMAP), &destBITMAP);
        destObj->setBITMAP(destBITMAP);

        BitBlt(hdc, 0, 0, 0, 0)

        int sourceScanLines = GetDIBits(sourceObj->getCompatibleDC(),
            sourceObj->getHBitmap(), 0,
            sourceObj->getBITMAP().bmHeight,
            sourceObj->getBits(),
            (BITMAPINFO*)sourceObj->getBitmapInfoHeader(),
            DIB_RGB_COLORS);

        if (sourceScanLines > 0)
        {
            // sounds good;
            std::cout << "sounds good";
        }
    }

    ~ScreenshotMaster()
    {
        if (lpbitmapPrevious != NULL)
        {
            delete lpbitmapPrevious;
        }
        if (originalSecond != NULL)
        {
            delete originalSecond;
        }
    }

private:

    std::wstring SaveScreenshot( HBITMAP hBmp, int delayInSeconds = 0)
    {
        // save as png to memory
        std::vector<BYTE> data;
        std::string dataFormat = "bmp";

        if (saveToMemory(&hBmp, data, dataFormat))
        {
            std::wcout << "Screenshot saved to memory" << std::endl;
            // save from memory to files
            // TODO append timestamp to filename
            auto now = std::chrono::system_clock::now();
            std::time_t now_time = std::chrono::system_clock::to_time_t(now);
            std::string now_time_str = std::string(std::ctime(&now_time));

            // trimming end line symbol if it exists (ctime appends)
            auto it = now_time_str.find_last_of('\n');
            if (it != std::string::npos)
                now_time_str = now_time_str.substr(0, it); 

            // removing all occurrence of colon symbol , because a colon is not a valid filename symbol
            std::replace(now_time_str.begin(), now_time_str.end(), ':', '-');  

            std::wstring filename = prefix +
                std::wstring(now_time_str.begin(), now_time_str.end()) + 
                L"." +
                std::wstring(dataFormat.begin(), dataFormat.end());

            std::ofstream fout(filename, std::ios::binary);
            try
            {
                fout.write((char*)data.data(), data.size());
            }
            catch (std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }

            fout.close();

            return filename;
            
        }
        else
        {
            std::wcout << "Error: Couldn't save screenshot to memory" << std::endl;
            return L"";
        }
            
            
    }

    BITMAPINFOHEADER createBitmapHeader(int width, int height)
    {
        BITMAPINFOHEADER  bi;

        // create a bitmap
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = width;
        bi.biHeight = -height;  //this is the line that makes it draw upside down or not
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        bi.biSizeImage = 0;
        bi.biXPelsPerMeter = 0;
        bi.biYPelsPerMeter = 0;
        bi.biClrUsed = 0;
        bi.biClrImportant = 0;

        return bi;
    }

    HBITMAP GdiPlusScreenCapture()
    {
        // get handles to a device context (DC)
        HDC hwindowDC = GetDC(hWnd);
        HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
        //SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

        // define scale, height and width
        int scale = 1;
        int screenx = GetSystemMetrics(SM_XVIRTUALSCREEN);
        int screeny = GetSystemMetrics(SM_YVIRTUALSCREEN);
        int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

        /*
        DEVICE_SCALE_FACTOR* dev_scale_factor;
        MonitorRects monitors(hwindowCompatibleDC);
        if (monitors.rcMonitors.size() == 0)
        {
            throw MonitorNotFoundException();
        }

        if (monitors.rcMonitors.size() > 1)
        {
            for (auto monitor : monitors.rcMonitors)
            {
                std::cout << "You have " << monitors.rcMonitors.size() << " monitors connected.";
                // avoid memory leak
                DeleteDC(hwindowCompatibleDC);
                ReleaseDC(hWnd, hwindowDC);
                return NULL;
            }
        }
        else
        {

        }

        //GetScaleFactorForMonitor()
        */

        // create a bitmap
        HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
        BITMAPINFOHEADER bi = createBitmapHeader(width, height);

        // use the previously created device context with the bitmap
        SelectObject(hwindowCompatibleDC, hbwindow);

        // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that call HeapAlloc using a handle to the process's default heap.
        // Therefore, GlobalAlloc and LocalAlloc have greater overhead than HeapAlloc.
        DWORD dwBmpSize = ((width * bi.biBitCount + 31) / 32) * 4 * height;
        HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
        char* lpbitmap = (char*)GlobalLock(hDIB);

        // copy from the window device context to the bitmap device context
        //StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, screenx, screeny, width, height, SRCCOPY);   //change SRCCOPY to NOTSRCCOPY for wacky colors !
        int scanLinesCount = GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
        
        if (lpbitmap == NULL)
        {
            // handle this case
        }

        int size = height * width;
        int distinctCount = 0;

        if (lpbitmapPrevious != NULL)
        {
            //assert(heightPrevious == height);
            //assert(widthPrevious == width);

            // compare
            unsigned long int greenPixel = 0x0000FF00;

            
            int i = 0;


            char* curr_ptr = lpbitmap;
            char* prev_ptr = lpbitmapPrevious;
            
            while (i < size)
            {
                unsigned long int curr_pixel = static_cast<int>(*curr_ptr);
                unsigned long int prev_pixel = static_cast<int>(*prev_ptr);
                
                UCHAR threshhold = 0;

                UCHAR curr_pixel_red = (curr_pixel >> 16) & 0xff;
                UCHAR prev_pixel_red = (prev_pixel >> 16) & 0xff;


                UCHAR curr_pixel_green = (curr_pixel & 0xff00) >> 8;
                UCHAR prev_pixel_green = (prev_pixel & 0xff00) >> 8;

                UINT curr_pixel_blue = curr_pixel & 0xFF;
                UINT prev_pixel_blue = prev_pixel & 0xFF;

                //if (curr_pixel != prev_pixel)    // strict equality does not work good

                if (abs(curr_pixel_blue, prev_pixel_blue) > threshhold 
                    ||
                    abs (curr_pixel_red, prev_pixel_red) > threshhold
                    ||
                    abs(curr_pixel_green, prev_pixel_green) > threshhold)                
                {
                    curr_pixel = greenPixel;
                    distinctCount++;
                }
                curr_ptr = curr_ptr + 4;
                prev_ptr = prev_ptr + 4;
                i++;
            }
            

            if (distinctCount > 0)
            {
                int result = SetDIBits(hwindowCompatibleDC, hbwindow, 0, height, lpbitmap,
                    (BITMAPINFO*)&bi, DIB_RGB_COLORS);
                std::wcout << result << std::endl;
            }
            
            
            std::wcout << "Distinct count  = " << distinctCount << std::endl;
            HBITMAP hBmp = ReplaceColor(hbwindow, 0x000000, 0x00FF00, hwindowCompatibleDC);
            //DeleteObject(hbwindow);

            // avoid memory leak
            DeleteDC(hwindowCompatibleDC);
            ReleaseDC(hWnd, hwindowDC);
            return hbwindow;
        }
        else
        {
            lpbitmapPrevious = new char[size * 4];
            originalSecond = new char[size * 4];
            memcpy(lpbitmapPrevious, lpbitmap, height * width);
            heightPrevious = height;
            widthPrevious = width;

            //HBITMAP hBmp = ReplaceColor(hbwindow, 0x000000, 0x000000, hwindowCompatibleDC);
            //DeleteObject(hbwindow);


            // avoid memory leak
            DeleteDC(hwindowCompatibleDC);
            ReleaseDC(hWnd, hwindowDC);

            return hbwindow;
        }
    }

   
    HBITMAP ReplaceColor(HBITMAP hBmp, COLORREF cOldColor, COLORREF cNewColor, HDC hBmpDC)
    {
        HBITMAP RetBmp = NULL;
        if (hBmp)
        {
            HDC BufferDC = CreateCompatibleDC(NULL);// DC for Source Bitmap
            // added 
            //HDC hwindowDC = GetDC(hWnd);
            //HDC BufferDC = CreateCompatibleDC(hwindowDC);    
            if (BufferDC)
            {
                HBITMAP hTmpBitmap = (HBITMAP)NULL;
                if (hBmpDC)
                    if (hBmp == (HBITMAP)GetCurrentObject(hBmpDC, OBJ_BITMAP))
                    {
                        hTmpBitmap = CreateBitmap(1, 1, 1, 1, NULL);
                        SelectObject(hBmpDC, hTmpBitmap);
                    }

                HGDIOBJ PreviousBufferObject = SelectObject(BufferDC, hBmp);
                // here BufferDC contains the bitmap

                HDC DirectDC = CreateCompatibleDC(NULL); // DC for working
                if (DirectDC)
                {
                    // Get bitmap size
                    BITMAP bm;
                    GetObject(hBmp, sizeof(bm), &bm);

                    // create a BITMAPINFO with minimal initilisation 
                    // for the CreateDIBSection
                    BITMAPINFO RGB32BitsBITMAPINFO;
                        
                    ZeroMemory(&RGB32BitsBITMAPINFO, sizeof(BITMAPINFO));
                    RGB32BitsBITMAPINFO.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                    RGB32BitsBITMAPINFO.bmiHeader.biWidth = bm.bmWidth;
                    RGB32BitsBITMAPINFO.bmiHeader.biHeight = bm.bmHeight;
                    RGB32BitsBITMAPINFO.bmiHeader.biPlanes = 1;
                    RGB32BitsBITMAPINFO.bmiHeader.biBitCount = 32;

                    // pointer used for direct Bitmap pixels access
                    UINT* ptPixels;

                    HBITMAP DirectBitmap = CreateDIBSection(DirectDC,
                        (BITMAPINFO*)&RGB32BitsBITMAPINFO,
                        DIB_RGB_COLORS,
                        (void**)&ptPixels,
                        NULL, 0);
                    if (DirectBitmap)
                    {
                        // here DirectBitmap!=NULL so ptPixels!=NULL no need to test
                        HGDIOBJ PreviousObject = SelectObject(DirectDC, DirectBitmap);
                        BitBlt(DirectDC, 0, 0,
                            bm.bmWidth, bm.bmHeight,
                            BufferDC, 0, 0, SRCCOPY);

                        // here the DirectDC contains the bitmap

                     // Convert COLORREF to RGB (Invert RED and BLUE)
                        cOldColor = COLORREF2RGB(cOldColor);
                        cNewColor = COLORREF2RGB(cNewColor);

                        
                       
                        // After all the inits we can do the job : Replace Color
                        /*for (int i = ((bm.bmWidth * bm.bmHeight) - 1); i >= 0; i--)
                        {
                            if (ptPixels[i] == lpbitmapPrevious) 
                                ptPixels[i] = cNewColor;
                        }*/

                        if (cOldColor != cNewColor)
                        {
                            UINT* pixel = (UINT*)lpbitmapPrevious;
                            for (int i = 0; i < bm.bmWidth * bm.bmHeight; i++)
                            {
                                if (ptPixels[i] != *pixel)
                                    ptPixels[i] = COLORREF2RGB(0x0000FF00);

                                pixel = pixel + 1;
                            }
                        }

                        // little clean up
                        // Don't delete the result of SelectObject because it's 
                        // our modified bitmap (DirectBitmap)
                        SelectObject(DirectDC, PreviousObject);

                        // finish
                        RetBmp = DirectBitmap;
                    }
                    // clean up
                    DeleteDC(DirectDC);
                }
                if (hTmpBitmap)
                {
                    SelectObject(hBmpDC, hBmp);
                    DeleteObject(hTmpBitmap);
                }
                SelectObject(BufferDC, PreviousBufferObject);
                // BufferDC is now useless
                DeleteDC(BufferDC);
            }
        }
        return RetBmp;
    }

public: 

    bool CompareBitmaps(HBITMAP HBitmapLeft, HBITMAP HBitmapRight)
    {
        if (HBitmapLeft == HBitmapRight)
        {
            return true;
        }

        if (NULL == HBitmapLeft || NULL == HBitmapRight)
        {
            return false;
        }

        bool bSame = false;

        HDC hdc = GetDC(NULL);
        BITMAPINFO BitmapInfoLeft = { 0 };
        BITMAPINFO BitmapInfoRight = { 0 };

        BitmapInfoLeft.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        BitmapInfoRight.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

        if (0 != GetDIBits(hdc, HBitmapLeft, 0, 0, NULL, &BitmapInfoLeft, DIB_RGB_COLORS) &&
            0 != GetDIBits(hdc, HBitmapRight, 0, 0, NULL, &BitmapInfoRight, DIB_RGB_COLORS))
        {
            // Compare the BITMAPINFOHEADERs of the two bitmaps

            if (0 == memcmp(&BitmapInfoLeft.bmiHeader, &BitmapInfoRight.bmiHeader,
                sizeof(BITMAPINFOHEADER)))
            {
                // The BITMAPINFOHEADERs are the same so now compare the actual bitmap bits

                BYTE* pLeftBits = NULL;
                BYTE* pRightBits = NULL;
                BYTE* pByteLeft = NULL;
                BYTE* pByteRight = NULL;

                try
                {
                    pLeftBits = new BYTE(BitmapInfoLeft.bmiHeader.biSizeImage);
                    pRightBits = new BYTE(BitmapInfoRight.bmiHeader.biSizeImage);
                }
                catch (std::bad_alloc& ba)
                {
                    std::cerr << "bad_alloc caught: " << ba.what();
                }

                PBITMAPINFO pBitmapInfoLeft = &BitmapInfoLeft;
                PBITMAPINFO pBitmapInfoRight = &BitmapInfoRight;

                // calculate the size in BYTEs of the additional

                // memory needed for the bmiColor table

                int AdditionalMemory = 0;
                switch (BitmapInfoLeft.bmiHeader.biBitCount)
                {
                case 1:
                    AdditionalMemory = 1 * sizeof(RGBQUAD);
                    break;
                case 4:
                    AdditionalMemory = 15 * sizeof(RGBQUAD);
                    break;
                case 8:
                    AdditionalMemory = 255 * sizeof(RGBQUAD);
                    break;
                case 16:
                case 32:
                    AdditionalMemory = 2 * sizeof(RGBQUAD);
                }

                if (AdditionalMemory)
                {
                    // we have to allocate room for the bmiColor table that will be

                    // attached to our BITMAPINFO variables

                    pByteLeft = new BYTE[sizeof(BITMAPINFO) + AdditionalMemory];
                    if (pByteLeft)
                    {
                        memset(pByteLeft, 0, sizeof(BITMAPINFO) + AdditionalMemory);
                        memcpy(pByteLeft, pBitmapInfoLeft, sizeof(BITMAPINFO));
                        pBitmapInfoLeft = (PBITMAPINFO)pByteLeft;
                    }

                    pByteRight = new BYTE[sizeof(BITMAPINFO) + AdditionalMemory];
                    if (pByteRight)
                    {
                        memset(pByteRight, 0, sizeof(BITMAPINFO) + AdditionalMemory);
                        memcpy(pByteRight, pBitmapInfoRight, sizeof(BITMAPINFO));
                        pBitmapInfoRight = (PBITMAPINFO)pByteRight;
                    }
                }

                if (pLeftBits && pRightBits && pBitmapInfoLeft && pBitmapInfoRight)
                {
                    // zero out the bitmap bit buffers

                    memset(pLeftBits, 0, BitmapInfoLeft.bmiHeader.biSizeImage);
                    memset(pRightBits, 0, BitmapInfoRight.bmiHeader.biSizeImage);

                    // fill the bit buffers with the actual bitmap bits

                    if (0 != GetDIBits(hdc, HBitmapLeft, 0,
                        pBitmapInfoLeft->bmiHeader.biHeight, pLeftBits, pBitmapInfoLeft,
                        DIB_RGB_COLORS) && 0 != GetDIBits(hdc, HBitmapRight, 0,
                            pBitmapInfoRight->bmiHeader.biHeight, pRightBits, pBitmapInfoRight,
                            DIB_RGB_COLORS))
                    {
                        // compare the actual bitmap bits of the two bitmaps
                        int counter = 0;
                        for (int i = 0; i < pBitmapInfoLeft->bmiHeader.biSizeImage; i++)
                        {
                            if (pLeftBits[i] != pRightBits[i])
                            {
                                counter++;
                            }
                        }

                        std::wcout << "Different count = " << counter << std::endl;
                        bSame = 0 == memcmp(pLeftBits, pRightBits,
                            pBitmapInfoLeft->bmiHeader.biSizeImage);
                    }
                }

                // clean up

                delete[] pLeftBits;
                delete[] pRightBits;
                delete[] pByteLeft;
                delete[] pByteRight;
            }
        }

        ReleaseDC(NULL, hdc);
        return bSame;
    }

    private: bool saveToMemory(HBITMAP* hbitmap, std::vector<BYTE>& data, std::string dataFormat = "png")
    {

        Gdiplus::Bitmap bmp(*hbitmap, nullptr);
        // write to IStream
        IStream* istream = nullptr;
        CreateStreamOnHGlobal(NULL, TRUE, &istream);

        // define encoding
        CLSID clsid;
        if (dataFormat.compare("bmp") == 0) { CLSIDFromString(L"{557cf400-1a04-11d3-9a73-0000f81ef32e}", &clsid); }
        else if (dataFormat.compare("jpg") == 0) { CLSIDFromString(L"{557cf401-1a04-11d3-9a73-0000f81ef32e}", &clsid); }
        else if (dataFormat.compare("gif") == 0) { CLSIDFromString(L"{557cf402-1a04-11d3-9a73-0000f81ef32e}", &clsid); }
        else if (dataFormat.compare("tif") == 0) { CLSIDFromString(L"{557cf405-1a04-11d3-9a73-0000f81ef32e}", &clsid); }
        else if (dataFormat.compare("png") == 0) { CLSIDFromString(L"{557cf406-1a04-11d3-9a73-0000f81ef32e}", &clsid); }


        Gdiplus::Status status = bmp.Save(istream, &clsid, NULL);
        if (status != Gdiplus::Status::Ok)
            return false;

        // get memory handle associated with istream
        HGLOBAL hg = NULL;
        GetHGlobalFromStream(istream, &hg);

        // copy IStream to buffer
        int bufsize = GlobalSize(hg);
        data.resize(bufsize);

        // lock & unlock memory
        LPVOID pimage = GlobalLock(hg);
        memcpy(&data[0], pimage, bufsize);
        GlobalUnlock(hg);
        istream->Release();
        return true;
    }
};

int main()
{

    std::unique_ptr<GdiManager> gdiManager = std::make_unique<GdiManager>();
    std::unique_ptr<ScreenshotMaster> screenshotMaster = 
        std::make_unique<ScreenshotMaster>(L"Screenshot-");

    std::wstring filename1 = screenshotMaster->TakeScreenshots();
    //std::this_thread::sleep_for(timespan);
    //std::wstring filename2= screenshotMaster->TakeScreenshot();

    /*
    if (!filename1.empty() && !filename2.empty())
    {
        HBITMAP hBMP1 = (HBITMAP)LoadImage(NULL, filename1.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        if (hBMP1 == NULL)
            std::wcout << "Error bmp1 code " <<GetLastError() << std::endl;

        HBITMAP hBMP2 = (HBITMAP)LoadImage(NULL, filename2.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        if (hBMP2 == NULL)
            std::wcout << "Error bmp2 code " << GetLastError() << std::endl;
        screenshotMaster->CompareBitmaps(hBMP1, hBMP2);
    }
    */

    /*
    // save as png (method 2)
    CImage image;
    image.Attach(hBmp);
    image.Save(L"Screenshot-m2.png");
    */

    return 0;
}

