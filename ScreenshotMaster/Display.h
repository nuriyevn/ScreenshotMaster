#include <iostream>
#include <vector>
#include <windows.h>

struct MonitorRects
{
    std::vector<RECT> rcMonitors;

    static BOOL CALLBACK MonitorEnum(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
    {
        MonitorRects* pThis = reinterpret_cast<MonitorRects*>(pData);
        pThis->rcMonitors.push_back(*lprcMonitor);
        return TRUE;
    }

    MonitorRects(HDC hdc)
    {
        EnumDisplayMonitors(hdc, 0, MonitorEnum, (LPARAM)this);
    }
};

class MonitorException : public std::exception
{
    const char* what() const throw ()
    {
        return "Monitor Exception";
    }
};

class TooManyMonitors : public MonitorException
{
    const char* what() const throw ()
    {
        return "Too Many Monitors Exception";
    }
};

class MonitorNotFoundException : public MonitorException
{
    const char* what() const throw ()
    {
        return "Monitor could not found";
    }
};
