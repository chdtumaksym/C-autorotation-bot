#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include <windows.h>
#include <string>
#include <thread>
#include <chrono>
#include <ctime>
#include <vector>

struct ProfileBinds {
    std::wstring profileName;
    WORD keyRed, keyGreen, keyBlue, keyYellow, keyCyan, keyMagenta;
    std::wstring nameRed, nameGreen, nameBlue, nameYellow, nameCyan, nameMagenta;
};

ProfileBinds rogueProfile = {
    L"РОГА (ROGUE)",
    '1', '2', '3', '4', '5', '6',
    L"Красный: Коварный удар (SS)", L"Зеленый: Мясорубка (SND)", L"Синий: Потрошение (EVIS)", 
    L"Желтый: Череда (KS)", L"Голубой: Адреналин (AR)", L"Пурпурный: Шквал (BF)"
};

ProfileBinds palaProfile = {
    L"ПАЛАДИН (PALADIN)",
    'R', '1', '2', 'Q', '4', '5',
    L"Красный: Правосудие (JUDGE)", L"Зеленый: Буря (DS)", L"Синий: УВС (CS)", 
    L"Желтый: Молот (HOW)", L"Голубой: Экзорцизм (EXO)", L"Пурпурный: Освящение (CONS)"
};

ProfileBinds* currentProfile = &palaProfile;
bool botActive = false;
bool isRunning = true;

HWND hMainWnd, hBtnToggle, hBtnProfile, hBtnSave, hStatStatus, hStatProfile;
HWND hBindsLabels[6];
HWND hBindsEdits[6];
HWND hLogEdit;

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
    AppendLog(L"[КАСТ] " + spellName + L" (Кнопка: " + btnName + L")");
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
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
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
    SetWindowTextW(hStatProfile, (L"Профиль: " + currentProfile->profileName).c_str());
}

void SaveBindsFromUI() {
    WORD* profileKeys[] = {&currentProfile->keyRed, &currentProfile->keyGreen, &currentProfile->keyBlue, 
                           &currentProfile->keyYellow, &currentProfile->keyCyan, &currentProfile->keyMagenta};
    for (int i = 0; i < 6; i++) {
        wchar_t buf[10];
        GetWindowTextW(hBindsEdits[i], buf, 10);
        if (wcslen(buf) > 0) {
            *profileKeys[i] = towupper(buf[0]);
        }
    }
    AppendLog(L"--- БИНДЫ УСПЕШНО СОХРАНЕНЫ ДЛЯ " + currentProfile->profileName + L" ---");
    LoadBindsToUI(); // Обновить форматирование в полях
}

void UpdateStatusUI() {
    SetWindowTextW(hStatStatus, botActive ? L"Статус бота: В РАБОТЕ (АКТИВЕН)" : L"Статус бота: ПРИОСТАНОВЛЕН");
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
            
            hBtnToggle = CreateWindowW(L"BUTTON", L"Вкл / Выкл (F1)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 15, 150, 35, hWnd, (HMENU)1, NULL, NULL);
            hBtnProfile = CreateWindowW(L"BUTTON", L"Сменить профиль (F2)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 180, 15, 180, 35, hWnd, (HMENU)2, NULL, NULL);
            hStatStatus = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 20, 60, 300, 20, hWnd, NULL, NULL, NULL);
            hStatProfile = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 20, 80, 300, 20, hWnd, NULL, NULL, NULL);

            for (int i = 0; i < 6; i++) {
                hBindsLabels[i] = CreateWindowW(L"STATIC", L"Spell", WS_CHILD | WS_VISIBLE, 20, 110 + (i * 30), 220, 20, hWnd, NULL, NULL, NULL);
                hBindsEdits[i] = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER | ES_UPPERCASE, 250, 108 + (i * 30), 40, 24, hWnd, NULL, NULL, NULL);
                SendMessage(hBindsLabels[i], WM_SETFONT, (WPARAM)hFont, TRUE);
                SendMessage(hBindsEdits[i], WM_SETFONT, (WPARAM)hFont, TRUE);
                SendMessage(hBindsEdits[i], EM_SETLIMITTEXT, 1, 0); // Лимит в 1 символ
            }

            hBtnSave = CreateWindowW(L"BUTTON", L"СОХРАНИТЬ БИНДЫ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 310, 108, 160, 175, hWnd, (HMENU)3, NULL, NULL);
            hLogEdit = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 20, 300, 450, 200, hWnd, NULL, NULL, NULL);
            
            HWND elements[] = {hBtnToggle, hBtnProfile, hStatStatus, hStatProfile, hBtnSave, hLogEdit};
            for (HWND el : elements) SendMessage(el, WM_SETFONT, (WPARAM)hFont, TRUE);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                botActive = !botActive;
                UpdateStatusUI();
                AppendLog(botActive ? L"--- БОТ ЗАПУЩЕН ---" : L"--- БОТ ОСТАНОВЛЕН ---");
            } else if (LOWORD(wParam) == 2) {
                currentProfile = (currentProfile == &palaProfile) ? &rogueProfile : &palaProfile;
                LoadBindsToUI();
                AppendLog(L"--- ПРОФИЛЬ ПЕРЕКЛЮЧЕН НА " + currentProfile->profileName + L" ---");
            } else if (LOWORD(wParam) == 3) {
                SaveBindsFromUI();
            }
            SetFocus(hWnd);
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetBkColor(hdcStatic, GetSysColor(COLOR_WINDOW));
            return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
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
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wc.lpszClassName = L"PixelBotGUIEx";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    hMainWnd = CreateWindowW(L"PixelBotGUIEx", L"WoW Pixel Bot [Advanced UI]", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 
                             (screenW - 510) / 2, (screenH - 560) / 2, 510, 560, NULL, NULL, hInstance, NULL);

    ShowWindow(hMainWnd, nCmdShow);
    LoadBindsToUI();
    UpdateStatusUI();
    AppendLog(L"Инициализация системы завершена. Выберите бинды и нажмите сохранить.");

    std::thread botThread(BotLoop);

    MSG msg;
    bool f1_pressed = false;
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

        if (GetAsyncKeyState(VK_F1) & 0x8000) {
            if (!f1_pressed) { 
                botActive = !botActive; 
                UpdateStatusUI(); 
                AppendLog(botActive ? L"--- БОТ ЗАПУЩЕН ---" : L"--- БОТ ОСТАНОВЛЕН ---");
                f1_pressed = true; 
            }
        } else f1_pressed = false;

        if (GetAsyncKeyState(VK_F2) & 0x8000) {
            if (!f2_pressed) { 
                currentProfile = (currentProfile == &palaProfile) ? &rogueProfile : &palaProfile; 
                LoadBindsToUI(); 
                AppendLog(L"--- ПРОФИЛЬ ПЕРЕКЛЮЧЕН НА " + currentProfile->profileName + L" ---");
                f2_pressed = true; 
            }
        } else f2_pressed = false;

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    botThread.join();
    return 0;
}
