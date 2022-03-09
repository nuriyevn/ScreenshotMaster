#include <windows.h>.

#define COLORREF2RGB(Color) (Color & 0xff00) | ((Color >> 16) & 0xff) \
                                 | ((Color << 16) & 0xff0000)
inline UCHAR abs(UCHAR a, UCHAR b)
{
    int c = a - b;
    if (c < 0)
        c *= -1;
    return (UCHAR)c;
}