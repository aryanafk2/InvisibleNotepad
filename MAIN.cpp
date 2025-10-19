#include <windows.h>
#include <windowsx.h>
#include <richedit.h>

#define IDC_RICHEDIT 101
#define IDC_BTN_UP 201
#define IDC_BTN_DOWN 202
#define IDC_BTN_MINIMIZE 203
#define IDC_BTN_MINVIEW 204
#define IDC_BTN_TOPMOST 205
#define IDC_BTN_CLOSE 206

HWND hRichEdit;
HWND btnUp, btnDown, btnMinimize, btnMinView, btnTopMost, btnClose;
int alpha = 200;

POINT dragStart;
bool dragging = false;

const int TOOLBAR_HEIGHT = 35;
const int MIN_WIDTH = 250;

bool minView = false;
RECT prevSize = { 0 };
bool alwaysOnTop = true;

// Dark theme colors
COLORREF BACKGROUND_COLOR = RGB(25, 25, 25);
COLORREF TEXT_COLOR = RGB(255, 255, 255);
COLORREF TOOLBAR_COLOR = RGB(40, 40, 40);
COLORREF BUTTON_COLOR = RGB(60, 60, 60);
COLORREF BUTTON_HIGHLIGHT = RGB(80, 80, 80);

HFONT CreateButtonFont() {
    return CreateFontW(16, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        VARIABLE_PITCH, L"Segoe UI");
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
    {
        LoadLibrary(L"Riched20.dll");

        hRichEdit = CreateWindowExW(0, RICHEDIT_CLASS, L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
            0, TOOLBAR_HEIGHT, 600, 365,
            hwnd, (HMENU)IDC_RICHEDIT, GetModuleHandle(NULL), NULL);

        HFONT hFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
            OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
        SendMessageW(hRichEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        SendMessage(hRichEdit, EM_SETBKGNDCOLOR, 0, (LPARAM)BACKGROUND_COLOR);
        CHARFORMAT2 cf = {};
        cf.cbSize = sizeof(cf);
        cf.dwMask = CFM_COLOR;
        cf.crTextColor = TEXT_COLOR;
        SendMessage(hRichEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);

        SetFocus(hRichEdit);

        // Toolbar buttons
        btnUp = CreateWindowW(L"BUTTON", L"BRIGHTEN", WS_CHILD | WS_VISIBLE,
            0, 0, 70, TOOLBAR_HEIGHT, hwnd, (HMENU)IDC_BTN_UP, GetModuleHandle(NULL), NULL);
        btnDown = CreateWindowW(L"BUTTON", L"DIMMER", WS_CHILD | WS_VISIBLE,
            75, 0, 70, TOOLBAR_HEIGHT, hwnd, (HMENU)IDC_BTN_DOWN, GetModuleHandle(NULL), NULL);
        btnMinView = CreateWindowW(L"BUTTON", L"HIDE", WS_CHILD | WS_VISIBLE,
            150, 0, 70, TOOLBAR_HEIGHT, hwnd, (HMENU)IDC_BTN_MINVIEW, GetModuleHandle(NULL), NULL);
        btnTopMost = CreateWindowW(L"BUTTON", L"TOP", WS_CHILD | WS_VISIBLE,
            225, 0, 70, TOOLBAR_HEIGHT, hwnd, (HMENU)IDC_BTN_TOPMOST, GetModuleHandle(NULL), NULL);
        btnMinimize = CreateWindowW(L"BUTTON", L"—", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            480, 0, 70, TOOLBAR_HEIGHT, hwnd, (HMENU)IDC_BTN_MINIMIZE, GetModuleHandle(NULL), NULL);
        btnClose = CreateWindowW(L"BUTTON", L"X", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            555, 0, 70, TOOLBAR_HEIGHT, hwnd, (HMENU)IDC_BTN_CLOSE, GetModuleHandle(NULL), NULL);

        HFONT btnFont = CreateButtonFont();
        SendMessage(btnUp, WM_SETFONT, (WPARAM)btnFont, TRUE);
        SendMessage(btnDown, WM_SETFONT, (WPARAM)btnFont, TRUE);
        SendMessage(btnMinView, WM_SETFONT, (WPARAM)btnFont, TRUE);
        SendMessage(btnTopMost, WM_SETFONT, (WPARAM)btnFont, TRUE);
        SendMessage(btnMinimize, WM_SETFONT, (WPARAM)btnFont, TRUE);
        SendMessage(btnClose, WM_SETFONT, (WPARAM)btnFont, TRUE);

        SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
    }
    return 0;

    case WM_SIZE:
    {
        int w = LOWORD(lParam);
        int h = HIWORD(lParam);

        if (h > TOOLBAR_HEIGHT)
            ShowWindow(hRichEdit, SW_SHOW);
        else
            ShowWindow(hRichEdit, SW_HIDE);

        MoveWindow(hRichEdit, 0, TOOLBAR_HEIGHT, w, max(h - TOOLBAR_HEIGHT, 0), TRUE);
        MoveWindow(btnUp, 0, 0, 70, TOOLBAR_HEIGHT, TRUE);
        MoveWindow(btnDown, 75, 0, 70, TOOLBAR_HEIGHT, TRUE);
        MoveWindow(btnMinView, 150, 0, 70, TOOLBAR_HEIGHT, TRUE);
        MoveWindow(btnTopMost, 225, 0, 70, TOOLBAR_HEIGHT, TRUE);
        MoveWindow(btnMinimize, w - 140, 0, 70, TOOLBAR_HEIGHT, TRUE);
        MoveWindow(btnClose, w - 70, 0, 70, TOOLBAR_HEIGHT, TRUE);
    }
    return 0;

    case WM_LBUTTONDOWN:
        if (HIWORD(lParam) <= TOOLBAR_HEIGHT) {
            dragging = true;
            dragStart.x = LOWORD(lParam);
            dragStart.y = HIWORD(lParam);
            SetCapture(hwnd);
        }
        return 0;

    case WM_LBUTTONUP:
        dragging = false;
        ReleaseCapture();
        return 0;

    case WM_MOUSEMOVE:
        if (dragging) {
            POINT p; GetCursorPos(&p);
            SetWindowPos(hwnd, NULL, p.x - dragStart.x, p.y - dragStart.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BTN_UP:
            alpha = min(alpha + 10, 255);
            SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
            break;
        case IDC_BTN_DOWN:
            alpha = max(alpha - 10, 50);
            SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
            break;
        case IDC_BTN_MINIMIZE:
            ShowWindow(hwnd, SW_MINIMIZE);
            break;
        case IDC_BTN_MINVIEW:
        {
            if (!minView) {
                GetWindowRect(hwnd, &prevSize);
                SetWindowPos(hwnd, NULL, 0, 0, prevSize.right - prevSize.left, TOOLBAR_HEIGHT, SWP_NOMOVE | SWP_NOZORDER);
                minView = true;
            }
            else {
                int w = prevSize.right - prevSize.left;
                int h = prevSize.bottom - prevSize.top;
                SetWindowPos(hwnd, NULL, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);
                minView = false;
            }
            break;
        }
        case IDC_BTN_TOPMOST:
        {
            alwaysOnTop = !alwaysOnTop;
            SetWindowPos(hwnd, alwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            break;
        }
        case IDC_BTN_CLOSE:
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            break;
        }
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        HBRUSH brush = CreateSolidBrush(BACKGROUND_COLOR);
        FillRect(hdc, &ps.rcPaint, brush);
        DeleteObject(brush);

        HBRUSH tbBrush = CreateSolidBrush(TOOLBAR_COLOR);
        RECT rc; GetClientRect(hwnd, &rc);
        rc.bottom = TOOLBAR_HEIGHT;
        FillRect(hdc, &rc, tbBrush);
        DeleteObject(tbBrush);

        EndPaint(hwnd, &ps);
    }
    return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_NCHITTEST:
    {
        LRESULT hit = DefWindowProc(hwnd, msg, wParam, lParam);
        if (hit == HTCLIENT) {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ScreenToClient(hwnd, &pt);
            RECT rc; GetClientRect(hwnd, &rc);
            const int border = 8;
            if (pt.x < border && pt.y < border) return HTTOPLEFT;
            if (pt.x > rc.right - border && pt.y < border) return HTTOPRIGHT;
            if (pt.x < border && pt.y > rc.bottom - border) return HTBOTTOMLEFT;
            if (pt.x > rc.right - border && pt.y > rc.bottom - border) return HTBOTTOMRIGHT;
            if (pt.x < border) return HTLEFT;
            if (pt.x > rc.right - border) return HTRIGHT;
            if (pt.y < border) return HTTOP;
            if (pt.y > rc.bottom - border) return HTBOTTOM;
            return HTCLIENT;
        }
        return hit;
    }

    case WM_GETMINMAXINFO:
    {
        LPMINMAXINFO mmi = (LPMINMAXINFO)lParam;
        mmi->ptMinTrackSize.x = MIN_WIDTH;
        mmi->ptMinTrackSize.y = TOOLBAR_HEIGHT + 20;
    }
    return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int) {
    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"GhostNotepad";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST,
        L"GhostNotepad",
        L"Ghost Notepad",
        WS_POPUP | WS_VISIBLE | WS_THICKFRAME,
        300, 200, 600, 400,
        NULL, NULL, hInstance, NULL
    );

    SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
