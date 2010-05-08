#ifndef MYBUTTON_H_INCLUDED
#define MYBUTTON_H_INCLUDED

#include "visualstylesxp.h"

#define WM_THEMECHANGED		0x031A // when windows changes themes

class MyButton : public CWindowImpl<MyButton, CButton>,
                 public CCustomDraw<MyButton>
{
public:
    BEGIN_MSG_MAP(MyButton)
        MESSAGE_HANDLER_EX(WM_THEMECHANGED,OnThemeChanged)
        CHAIN_MSG_MAP_ALT(CCustomDraw<MyButton>, 1)
    END_MSG_MAP()

    bool DrawFocus;
    
    MyButton()
    {
        hTheme = NULL;
        IsThemingEnabled = false;
        DrawFocus = false;
    }

    ~MyButton()
    {
        if (hTheme)
        {
            g_xpStyle.CloseThemeData(hTheme);
        }
    }

    void LoadThemeData()
    {
        if (hTheme)
        {
            g_xpStyle.CloseThemeData(hTheme);
        }
        IsThemingEnabled = (g_xpStyle.IsAppThemed() && g_xpStyle.IsThemeActive());
        if (IsThemingEnabled)
        {
            hTheme = g_xpStyle.OpenThemeData(m_hWnd, L"BUTTON");
        }
    };

    LRESULT OnThemeChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
    {
        LoadThemeData();
        InvalidateRect(NULL, TRUE);
        return 0;
    }

    DWORD OnPreErase(int idCtrl, LPNMCUSTOMDRAW lpNMCustomDraw)
    {
        if (!IsThemingEnabled)
        {
            return CDRF_DODEFAULT;
        }
        int state_id = GetStateID(lpNMCustomDraw->uItemState);
        // erase background (according to parent window's themed background)
        if (g_xpStyle.IsThemeBackgroundPartiallyTransparent(hTheme,BP_PUSHBUTTON,state_id))
        {
            g_xpStyle.DrawThemeParentBackground(m_hWnd, lpNMCustomDraw->hdc, &lpNMCustomDraw->rc);
        }
        return OnPrePaint(idCtrl, lpNMCustomDraw);
    }

    DWORD OnPrePaint(int idCtrl, LPNMCUSTOMDRAW lpNMCustomDraw)
    {
        if (!IsThemingEnabled)
        {
            return CDRF_DODEFAULT;
        }
        int state_id = GetStateID(lpNMCustomDraw->uItemState);

        g_xpStyle.DrawThemeBackground(hTheme, lpNMCustomDraw->hdc, BP_PUSHBUTTON, state_id, &lpNMCustomDraw->rc, NULL);

        // get content rectangle (space inside button for image)
        CRect content_rect(lpNMCustomDraw->rc);
        g_xpStyle.GetThemeBackgroundContentRect(hTheme, lpNMCustomDraw->hdc, BP_PUSHBUTTON, state_id, &lpNMCustomDraw->rc, &content_rect);
        
        // draw the image
        DWORD style = GetStyle();
        if (style & BS_BITMAP)
        {
            draw_bitmap(lpNMCustomDraw->hdc, &content_rect, style);
        }
        else // draw the icon
        {
            draw_icon(lpNMCustomDraw->hdc, &content_rect, style);
        }
        
        // finally, draw the focus rectangle if needed
        if ((lpNMCustomDraw->uItemState & CDIS_FOCUS) && DrawFocus)
        {
            // draw focus rectangle
            DrawFocusRect(lpNMCustomDraw->hdc, &content_rect);
        }
        return CDRF_SKIPDEFAULT;
    }

    bool IsWinVerXPor2003()
    {
        OSVERSIONINFO vi;
        vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx(&vi);
        if ((vi.dwMajorVersion == 5) && ((vi.dwMinorVersion == 1) || (vi.dwMinorVersion == 2)))
        {
            return true;
        }
        return false;
    }

private:
    HTHEME hTheme;
    bool IsThemingEnabled;

    int GetStateID(UINT ItemState)
    {
        DWORD style = GetStyle();
        int state_id = PBS_NORMAL;
        if (style & WS_DISABLED)
            state_id = PBS_DISABLED;
        else if (ItemState & CDIS_SELECTED)
            state_id = PBS_PRESSED;
        else if (ItemState & CDIS_HOT)
            state_id = PBS_HOT;
        else if (style & BS_DEFPUSHBUTTON)
            state_id = PBS_DEFAULTED;
        return state_id;
    }

    void draw_bitmap(HDC hDC, const CRect& Rect, DWORD style)
    {
        HBITMAP hBitmap = GetBitmap();
        if (hBitmap == NULL) return;

        // determine size of bitmap image
        BITMAPINFO bmi;
        memset(&bmi, 0, sizeof (BITMAPINFO));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        GetDIBits(hDC, hBitmap, 0, 0, NULL, &bmi, DIB_RGB_COLORS);

        // determine position of top-left corner of bitmap (positioned according to style)
        int x = image_left(bmi.bmiHeader.biWidth, Rect, style);
        int y = image_top(bmi.bmiHeader.biHeight, Rect, style);

        // Draw the bitmap
        DrawState(hDC, NULL, NULL, (LPARAM)hBitmap, 0, x, y, bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight,
            (style & WS_DISABLED) != 0 ? (DST_BITMAP | DSS_DISABLED) : (DST_BITMAP | DSS_NORMAL));
    }

    void draw_icon(HDC hDC, const CRect& Rect, DWORD style)
    {
        HICON hIcon = GetIcon();
        if (hIcon == NULL) return;

        // determine size of icon image
        ICONINFO ii;
        GetIconInfo(hIcon, &ii);
        BITMAPINFO bmi;
        memset (&bmi, 0, sizeof(BITMAPINFO));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        int cx = 0;
        int cy = 0;
        
        if (ii.hbmColor != NULL)
        {
            // icon has separate image and mask bitmaps - use size directly
            GetDIBits(hDC, ii.hbmColor, 0, 0, NULL, &bmi, DIB_RGB_COLORS);
            cx = bmi.bmiHeader.biWidth;
            cy = bmi.bmiHeader.biHeight;
            DeleteObject(ii.hbmColor);
            if (ii.hbmMask != NULL) DeleteObject(ii.hbmMask);
        }
        else
        {
            // icon has single mask bitmap which is twice as high as icon
            GetDIBits(hDC, ii.hbmMask, 0, 0, NULL, &bmi, DIB_RGB_COLORS);
            cx = bmi.bmiHeader.biWidth;
            cy = bmi.bmiHeader.biHeight/2;
            DeleteObject(ii.hbmMask);
        }
        
        // determine position of top-left corner of icon
        int x = image_left(cx, Rect, style);
        int y = image_top(cy, Rect, style);

        // Draw the icon
        DrawState(hDC, NULL, NULL, (LPARAM)hIcon, 0, x, y, cx, cy,
            (style & WS_DISABLED) != 0 ? (DST_ICON | DSS_DISABLED) : (DST_ICON | DSS_NORMAL));
    }

    int image_left(int cx, const CRect& Rect, DWORD style)
    {
        int x = Rect.left;
        if (cx > Rect.Width ())
            cx = Rect.Width();
        else if ((style & BS_CENTER) == BS_LEFT)
            x = Rect.left;
        else if ((style & BS_CENTER) == BS_RIGHT)
            x = Rect.right - cx;
        else
            x = Rect.left + (Rect.Width () - cx)/2;
        return (x);
    }

    int image_top(int cy, const CRect& Rect, DWORD style)
    {
        int y = Rect.top;
        if (cy > Rect.Height ())
            cy = Rect.Height ();
        if ((style & BS_VCENTER) == BS_TOP)
            y = Rect.top;
        else if ((style & BS_VCENTER) == BS_BOTTOM)
            y = Rect.bottom - cy;
        else
            y = Rect.top + (Rect.Height () - cy)/2;
        return (y);
    }
};

#endif // MYBUTTON_H_INCLUDED

