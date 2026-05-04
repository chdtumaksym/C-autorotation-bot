#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include <windows.h>
#include <string>
#include <thread>
#include <chrono>
#include <ctime>

struct ProfileBinds {
    std::wstring profileName;
    WORD keyRed, keyGreen, keyBlue, keyYellow, keyCyan, keyMagenta;
    std::wstring nameRed, nameGreen, nameBlue, nameYellow, nameCyan, nameMagenta;
};

ProfileBinds rogueProfile = {
    L"РОГА (ROGUE)",
    '1', '2', '3', '4', '5', '6',
    L"Коварный удар", L"Мясорубка", L"Потрошение", 
    L"Череда убийств", L"Выброс адреналина", L"Шквал клинков"
};

ProfileBinds palaProfile = {
    L"ПАЛАДИН (PALADIN)",
    'R', '1', '2', 'Q', '4', '5',
    L"Правосудие", L"Божественная буря", L"Удар воина Света", 
    L"Молот гнева", L"Экзорцизм", L"Освящение"
};

ProfileBinds* currentProfile = &rogueProfile;
bool botActive = false;
bool isRunning = true;
int castDelayMs = 20;

HWND hMainWnd, hBtnToggle, hBtnProfile, hBtnSave, hBtnClose, hStatStatus, hStatProfile, hDelayLabel, hDelayEdit;
HWND hBindsLabels[6];
HWND hBindsEdits[6];
HWND hLogEdit;

HBRUSH bgBrush = CreateSolidBrush(RGB(25, 25, 25));
HBRUSH editBrush = CreateSolidBrush(RGB(15, 15, 15));
HBRUSH btnBrush = CreateSolidBrush(RGB(50, 50, 55));
HBRUSH btnHoverBrush = CreateSolidBrush(RGB(70, 70, 75));
HBRUSH closeBtnBrush = CreateSolidBrush(RGB(150, 40, 40));
HBRUSH closeBtnHoverBrush = CreateSolidBrush(RGB(200, 50, 50));

void AppendLog(const std::wstring& msg) {
    if (!hLogEdit) return;
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    localtime_s(&tm, &t);
    wchar_t timeBuf[64];
    swprintf(timeBuf, 64, L"[%02d:%02d:%02d.%03d] ", tm.tm_hour, tm.tm_min, tm.tm_sec, (int)ms.count());
    std::wstring fullMsg = timeBuf + msg + L"\r\n";
    int len = GetWindowTextLength(hLogEdit);
    SendMessage(hLogEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessage(hLogEdit, EM_REPLACESEL, 0, (LPARAM)fullMsg.c_str());
}

void PressKey(WORD vkCode, const std::wstring& spellName) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vkCode;
    SendInput(1, &input, sizeof(INPUT));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
    wchar_t btnName[2] = {(wchar_t)vkCode, 0};
    AppendLog(L"[КАСТ] " + spellName + L" -> [" + btnName + L"]");
}

int ClassifyColor(int r, int g, int b) {
    if (r > 200 && g < 50 && b < 50) return 1;
    if (r < 50 && g > 200 && b < 50) return 2;
    if (r < 50 && g < 50 && b > 200) return 3;
    if (r > 200 && g > 200 && b < 50) return 4;
    if (r < 50 && g > 200 && b > 200) return 5;
    if (r > 200 && g < 50 && b > 200) return 6;
    return 0;
}

void BotLoop() {
    HDC hdc = GetDC(NULL);
    while (isRunning) {
        if (botActive) {
            COLORREF color = GetPixel(hdc, 0, 0);
            int colorId = ClassifyColor(GetRValue(color), GetGValue(color), GetBValue(color));
            if (colorId != 0) {
                switch (colorId) {
                    case 1: PressKey(currentProfile->keyRed, currentProfile->nameRed); break;
                    case 2: PressKey(currentProfile->keyGreen, currentProfile->nameGreen); break;
                    case 3: PressKey(currentProfile->keyBlue, currentProfile->nameBlue); break;
                    case 4: PressKey(currentProfile->keyYellow, currentProfile->nameYellow); break;
                    case 5: PressKey(currentProfile->keyCyan, currentProfile->nameCyan); break;
                    case 6: PressKey(currentProfile->keyMagenta, currentProfile->nameMagenta); break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(castDelayMs));
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    ReleaseDC(NULL, hdc);
}

void LoadBindsToUI() {
    std::wstring names[] = {currentProfile->nameRed, currentProfile->nameGreen, currentProfile->nameBlue, 
                            currentProfile->nameYellow, currentProfile->nameCyan, currentProfile->nameMagenta};
    WORD keys[] = {currentProfile->keyRed, currentProfile->keyGreen, currentProfile->keyBlue, 
                   currentProfile->keyYellow, currentProfile->keyCyan, currentProfile->keyMagenta};
    for (int i = 0; i < 6; i++) {
        SetWindowTextW(hBindsLabels[i], names[i].c_str());
        wchar_t buf[2] = {(wchar_t)keys[i], 0};
        SetWindowTextW(hBindsEdits[i], buf);
    }
    SetWindowTextW(hStatProfile, (L"ПРОФИЛЬ: " + currentProfile->profileName).c_str());
    
    wchar_t delayBuf[10];
    swprintf(delayBuf, 10, L"%d", castDelayMs);
    SetWindowTextW(hDelayEdit, delayBuf);
}

void SaveBindsFromUI() {
    WORD* profileKeys[] = {&currentProfile->keyRed, &currentProfile->keyGreen, &currentProfile->keyBlue, 
                           &currentProfile->keyYellow, &currentProfile->keyCyan, &currentProfile->keyMagenta};
    for (int i = 0; i < 6; i++) {
        wchar_t buf[10];
        GetWindowTextW(hBindsEdits[i], buf, 10);
        if (wcslen(buf) > 0) *profileKeys[i] = towupper(buf[0]);
    }
    
    wchar_t delayBuf[10];
    GetWindowTextW(hDelayEdit, delayBuf, 10);
    castDelayMs = _wtoi(delayBuf);
    if (castDelayMs < 0) castDelayMs = 0;
    
    AppendLog(L"--- БИНДЫ И НАСТРОЙКИ СОХРАНЕНЫ ---");
    LoadBindsToUI();
}

void UpdateStatusUI() {
    SetWindowTextW(hStatStatus, botActive ? L"СТАТУС: АКТИВЕН (РАБОТАЕТ)" : L"СТАТУС: ОЖИДАНИЕ (ПАУЗА)");
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontW(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
            HFONT hLogFont = CreateFontW(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, L"Consolas");
            
            hBtnClose = CreateWindowW(L"BUTTON", L"X", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 475, 5, 30, 30, hWnd, (HMENU)4, NULL, NULL);
            hBtnToggle = CreateWindowW(L"BUTTON", L"ВКЛ / ВЫКЛ (F9)", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 20, 45, 150, 40, hWnd, (HMENU)1, NULL, NULL);
            hBtnProfile = CreateWindowW(L"BUTTON", L"ПРОФИЛЬ (F2)", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 180, 45, 150, 40, hWnd, (HMENU)3, NULL, NULL);
            hBtnSave = CreateWindowW(L"BUTTON", L"СОХРАНИТЬ", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 340, 45, 130, 40, hWnd, (HMENU)2, NULL, NULL);
            
            hStatStatus = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 20, 95, 300, 20, hWnd, NULL, NULL, NULL);
            hStatProfile = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 20, 115, 300, 20, hWnd, NULL, NULL, NULL);
            
            hDelayLabel = CreateWindowW(L"STATIC", L"Задержка (мс):", WS_CHILD | WS_VISIBLE, 340, 95, 100, 20, hWnd, NULL, NULL, NULL);
            hDelayEdit = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER | ES_NUMBER, 440, 93, 30, 22, hWnd, NULL, NULL, NULL);

            for (int i = 0; i < 6; i++) {
                hBindsLabels[i] = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 20, 150 + (i * 35), 220, 20, hWnd, NULL, NULL, NULL);
                hBindsEdits[i] = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER | ES_UPPERCASE, 250, 148 + (i * 35), 40, 24, hWnd, NULL, NULL, NULL);
                SendMessage(hBindsLabels[i], WM_SETFONT, (WPARAM)hFont, TRUE);
                SendMessage(hBindsEdits[i], WM_SETFONT, (WPARAM)hFont, TRUE);
                SendMessage(hBindsEdits[i], EM_SETLIMITTEXT, 1, 0);
            }

            hLogEdit = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 310, 148, 180, 199, hWnd, NULL, NULL, NULL);
            
            HWND elements[] = {hBtnClose, hStatStatus, hStatProfile, hDelayLabel, hDelayEdit, hLogEdit};
            for (HWND el : elements) SendMessage(el, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLogEdit, WM_SETFONT, (WPARAM)hLogFont, TRUE);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            SetTextColor(hdc, RGB(200, 200, 200));
            SetBkMode(hdc, TRANSPARENT);
            HFONT hFont = CreateFontW(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
            SelectObject(hdc, hFont);
            RECT rt = {20, 10, 300, 30};
            DrawTextW(hdc, L"WoW Pixel Bot - Overlay Edition", -1, &rt, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            DeleteObject(hFont);
            EndPaint(hWnd, &ps);
            break;
        }
        case WM_NCHITTEST: {
            LRESULT hit = DefWindowProcW(hWnd, msg, wParam, lParam);
            if (hit == HTCLIENT) {
                POINT pt;
                pt.x = (short)LOWORD(lParam);
                pt.y = (short)HIWORD(lParam);
                ScreenToClient(hWnd, &pt);
                if (pt.y < 40) return HTCAPTION; // Позволяет таскать окно за верхнюю часть
            }
            return hit;
        }
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lParam;
            if (pdis->CtlID == 4) { // Кнопка закрытия
                FillRect(pdis->hDC, &pdis->rcItem, (pdis->itemState & ODS_SELECTED) ? closeBtnHoverBrush : closeBtnBrush);
                SetTextColor(pdis->hDC, RGB(255, 255, 255));
                SetBkMode(pdis->hDC, TRANSPARENT);
                DrawTextW(pdis->hDC, L"X", -1, &pdis->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                return TRUE;
            } else if (pdis->CtlID == 1 || pdis->CtlID == 2 || pdis->CtlID == 3) {
                FillRect(pdis->hDC, &pdis->rcItem, (pdis->itemState & ODS_SELECTED) ? btnHoverBrush : btnBrush);
                SetTextColor(pdis->hDC, RGB(255, 255, 255));
                SetBkMode(pdis->hDC, TRANSPARENT);
                wchar_t text[64];
                GetWindowTextW(pdis->hwndItem, text, 64);
                DrawTextW(pdis->hDC, text, -1, &pdis->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                return TRUE;
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            HWND hwnd = (HWND)lParam;
            SetBkMode(hdc, TRANSPARENT);
            if (hwnd == hStatStatus) SetTextColor(hdc, botActive ? RGB(0, 255, 100) : RGB(255, 100, 100));
            else if (hwnd == hStatProfile) SetTextColor(hdc, RGB(100, 200, 255));
            else SetTextColor(hdc, RGB(220, 220, 220));
            return (INT_PTR)bgBrush;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            HWND hwnd = (HWND)lParam;
            SetBkColor(hdc, RGB(20, 20, 20));
            if (hwnd == hLogEdit) SetTextColor(hdc, RGB(0, 255, 0));
            else SetTextColor(hdc, RGB(255, 255, 255));
            return (INT_PTR)editBrush;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                botActive = !botActive;
                UpdateStatusUI();
                AppendLog(botActive ? L">> БОТ ЗАПУЩЕН" : L">> БОТ ОСТАНОВЛЕН");
                InvalidateRect(hStatStatus, NULL, TRUE);
            } else if (LOWORD(wParam) == 2) {
                SaveBindsFromUI();
            } else if (LOWORD(wParam) == 3) {
                currentProfile = (currentProfile == &palaProfile) ? &rogueProfile : &palaProfile;
                LoadBindsToUI();
                AppendLog(L">> СМЕНА ПРОФИЛЯ: " + currentProfile->profileName);
                InvalidateRect(hStatProfile, NULL, TRUE);
            } else if (LOWORD(wParam) == 4) {
                isRunning = false;
                PostQuitMessage(0);
            }
            SetFocus(hWnd);
            break;
        }
        case WM_DESTROY:
            isRunning = false;
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = bgBrush;
    wc.lpszClassName = L"PixelBotOverlay";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    
    // WS_POPUP убирает рамки винды, WS_EX_TOPMOST поверх всех окон, WS_EX_LAYERED для прозрачности
    hMainWnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED, L"PixelBotOverlay", L"WoW Pixel Bot", 
                               WS_POPUP | WS_VISIBLE, (screenW - 510) / 2, (screenH - 370) / 2, 510, 370, NULL, NULL, hInstance, NULL);

    SetLayeredWindowAttributes(hMainWnd, 0, 230, LWA_ALPHA); // 230 из 255 (90% непрозрачности)

    LoadBindsToUI();
    UpdateStatusUI();
    AppendLog(L"Оверлей загружен. F9 - Старт/Стоп.");

    std::thread botThread(BotLoop);

    MSG msg;
    bool f9_pressed = false;
    bool f2_pressed = false;

    while (isRunning) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                isRunning = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (GetAsyncKeyState(VK_F9) & 0x8000) {
            if (!f9_pressed) { 
                botActive = !botActive; 
                UpdateStatusUI(); 
                AppendLog(botActive ? L">> БОТ ЗАПУЩЕН" : L">> БОТ ОСТАНОВЛЕН");
                InvalidateRect(hStatStatus, NULL, TRUE);
                f9_pressed = true; 
            }
        } else f9_pressed = false;

        if (GetAsyncKeyState(VK_F2) & 0x8000) {
            if (!f2_pressed) { 
                currentProfile = (currentProfile == &palaProfile) ? &rogueProfile : &palaProfile;
                LoadBindsToUI();
                AppendLog(L">> СМЕНА ПРОФИЛЯ: " + currentProfile->profileName);
                InvalidateRect(hStatProfile, NULL, TRUE);
                f2_pressed = true; 
            }
        } else f2_pressed = false;

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    botThread.join();
    DeleteObject(bgBrush);
    DeleteObject(editBrush);
    DeleteObject(btnBrush);
    DeleteObject(btnHoverBrush);
    DeleteObject(closeBtnBrush);
    DeleteObject(closeBtnHoverBrush);
    return 0;
}
