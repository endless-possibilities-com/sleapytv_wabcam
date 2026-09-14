#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <shellapi.h>
#include <mfapi.h>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

static constexpr wchar_t CLASS_NAME[] = L"SleapyCameraWindow";
static constexpr UINT WM_TRAY = WM_APP + 1;
static constexpr UINT TRAY_ID = 100;

static HWND g_hwnd{};
static ID3D11Device* g_device{};
static ID3D11DeviceContext* g_context{};
static IDXGISwapChain* g_swap{};
static ID3D11RenderTargetView* g_rtv{};
static bool g_running = false;
static bool g_settings = false;

static void CleanupD3D() {
    if (g_rtv) g_rtv->Release();
    if (g_swap) g_swap->Release();
    if (g_context) g_context->Release();
    if (g_device) g_device->Release();
    g_rtv = nullptr; g_swap = nullptr; g_context = nullptr; g_device = nullptr;
}

static bool InitD3D(HWND hwnd) {
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL fl{};
    const D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

    if (FAILED(D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        levels, 2, D3D11_SDK_VERSION, &sd,
        &g_swap, &g_device, &fl, &g_context))) {
        return false;
    }

    ID3D11Texture2D* back{};
    if (FAILED(g_swap->GetBuffer(0, IID_PPV_ARGS(&back)))) return false;
    HRESULT hr = g_device->CreateRenderTargetView(back, nullptr, &g_rtv);
    back->Release();
    return SUCCEEDED(hr);
}

static void Render() {
    if (!g_context || !g_rtv || !g_swap) return;
    const float clear[4] = { 0.025f, 0.035f, 0.075f, 1.0f };
    g_context->OMSetRenderTargets(1, &g_rtv, nullptr);
    g_context->ClearRenderTargetView(g_rtv, clear);
    g_swap->Present(1, 0);
}

static void TrayAdd() {
    NOTIFYICONDATAW n{};
    n.cbSize = sizeof(n);
    n.hWnd = g_hwnd;
    n.uID = TRAY_ID;
    n.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    n.uCallbackMessage = WM_TRAY;
    n.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    lstrcpyW(n.szTip, L"Sleapy Camera");
    Shell_NotifyIconW(NIM_ADD, &n);
}

static void TrayRemove() {
    NOTIFYICONDATAW n{};
    n.cbSize = sizeof(n);
    n.hWnd = g_hwnd;
    n.uID = TRAY_ID;
    Shell_NotifyIconW(NIM_DELETE, &n);
}

static void DrawUI(HDC dc) {
    RECT r{};
    GetClientRect(g_hwnd, &r);

    HBRUSH b = CreateSolidBrush(RGB(12,16,30));
    FillRect(dc, &r, b);
    DeleteObject(b);

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(235,240,255));

    HFONT font = CreateFontW(24,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,
        DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
        DEFAULT_PITCH,L"Segoe UI");
    HGDIOBJ old = SelectObject(dc, font);

    TextOutW(dc, 24, 20, L"SLEAPY CAMERA", 13);

    SelectObject(dc, old);
    DeleteObject(font);

    font = CreateFontW(17,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,
        DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
        DEFAULT_PITCH,L"Segoe UI");
    old = SelectObject(dc, font);

    int y = 70;
    if (!g_settings) {
        TextOutW(dc, 24, y, L"CAMERA MODE", 11); y += 35;
        TextOutW(dc, 24, y, L"Webcam: Default", 15); y += 30;
        TextOutW(dc, 24, y, L"Resolution: 720p / 1080p", 25); y += 30;
        TextOutW(dc, 24, y, L"FPS: 30 / 60", 12); y += 30;
        TextOutW(dc, 24, y, L"Background: Original / Blur / Image", 34); y += 30;
        TextOutW(dc, 24, y, L"Face tracking: ON", 17); y += 30;
        TextOutW(dc, 24, y, L"Face blur: ON", 13); y += 42;
        TextOutW(dc, 24, y, g_running ? L"STATUS: RUNNING" : L"STATUS: STOPPED", 15);
        y += 42;
        TextOutW(dc, 24, y, L"[SPACE] Start/Stop   [S] Settings   [ESC] Tray", 44);
    } else {
        TextOutW(dc, 24, y, L"SETTINGS", 8); y += 38;
        TextOutW(dc, 24, y, L"Camera       [ connected webcam ]", 33); y += 30;
        TextOutW(dc, 24, y, L"Resolution   [ 720p / 1080p ]", 31); y += 30;
        TextOutW(dc, 24, y, L"FPS          [ 30 / 60 ]", 24); y += 30;
        TextOutW(dc, 24, y, L"Background   [ Original / Blur / Image ]", 41); y += 30;
        TextOutW(dc, 24, y, L"Privacy      [ Face Blur ]", 26); y += 42;
        TextOutW(dc, 24, y, L"[C] Camera mode   [ESC] Tray", 28);
    }

    SelectObject(dc, old);
    DeleteObject(font);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        TrayAdd();
        SetTimer(hwnd, 1, 16, nullptr);
        return 0;
    case WM_TIMER:
        Render();
        return 0;
    case WM_KEYDOWN:
        if (wp == 'S') { g_settings = true; InvalidateRect(hwnd,nullptr,TRUE); }
        else if (wp == 'C') { g_settings = false; InvalidateRect(hwnd,nullptr,TRUE); }
        else if (wp == VK_SPACE) { g_running = !g_running; InvalidateRect(hwnd,nullptr,TRUE); }
        else if (wp == VK_ESCAPE) ShowWindow(hwnd, SW_HIDE);
        return 0;
    case WM_TRAY:
        if (lp == WM_LBUTTONDBLCLK) {
            ShowWindow(hwnd, SW_SHOW);
            SetForegroundWindow(hwnd);
        }
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps{};
        HDC dc = BeginPaint(hwnd, &ps);
        DrawUI(dc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        KillTimer(hwnd,1);
        TrayRemove();
        CleanupD3D();
        MFShutdown();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd,msg,wp,lp);
}

int WINAPI wWinMain(HINSTANCE h, HINSTANCE, PWSTR, int show) {
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) return 1;
    if (FAILED(MFStartup(MF_VERSION))) {
        CoUninitialize();
        return 2;
    }

    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = h;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    RegisterClassW(&wc);

    g_hwnd = CreateWindowExW(
        0, CLASS_NAME, L"Sleapy Camera",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 820, 620,
        nullptr, nullptr, h, nullptr);

    if (!g_hwnd || !InitD3D(g_hwnd)) {
        if (g_hwnd) DestroyWindow(g_hwnd);
        MFShutdown();
        CoUninitialize();
        return 3;
    }

    ShowWindow(g_hwnd, show);
    UpdateWindow(g_hwnd);

    MSG msg{};
    while (GetMessageW(&msg,nullptr,0,0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    CoUninitialize();
    return (int)msg.wParam;
}
