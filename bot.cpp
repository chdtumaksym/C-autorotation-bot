#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include <windows.h>
#include <richedit.h>
#include <string>
#include <thread>
#include <chrono>
#include <ctime>

enum Lang { EN = 0, RU = 1, UA = 2 };
Lang currentLang = RU;
bool showSettings = false;
bool isTopMost = true;
bool isDarkTheme = true;

const wchar_t* uiStrs[][3] = {
    { L"ON / OFF (F9)", L"ВКЛ / ВЫКЛ (F9)", L"УВІМК / ВИМК (F9)" },
    { L"PROFILE \x25BC", L"ПРОФИЛЬ \x25BC", L"ПРОФІЛЬ \x25BC" },
    { L"SAVE", L"СОХРАНИТЬ", L"ЗБЕРЕГТИ" },
    { L"SETTINGS", L"НАСТРОЙКИ", L"НАЛАШТУВАННЯ" },
    { L"STATUS: ACTIVE (RUNNING)", L"СТАТУС: АКТИВЕН (РАБОТАЕТ)", L"СТАТУС: АКТИВНИЙ (ПРАЦЮЄ)" },
    { L"STATUS: WAITING (PAUSED)", L"СТАТУС: ОЖИДАНИЕ (ПАУЗА)", L"СТАТУС: ОЧІКУВАННЯ (ПАУЗА)" },
    { L"PROFILE: ", L"ПРОФИЛЬ: ", L"ПРОФІЛЬ: " },
    { L"Delay (ms):", L"Задержка (мс):", L"Затримка (мс):" },
    { L"Always on top (Overlay)", L"Оверлей поверх всех окон", L"Оверлей поверх усіх вікон" },
    { L"Interface Language:", L"Язык интерфейса:", L"Мова інтерфейсу:" },
    { L">> BOT STARTED", L">> БОТ ЗАПУЩЕН", L">> БОТ ЗАПУЩЕНО" },
    { L">> BOT STOPPED", L">> БОТ ОСТАНОВЛЕН", L">> БОТ ЗУПИНЕНО" },
    { L"--- BINDS & SETTINGS SAVED ---", L"--- БИНДЫ И НАСТРОЙКИ СОХРАНЕНЫ ---", L"--- БІНДИ ТА НАЛАШТУВАННЯ ЗБЕРЕЖЕНО ---" },
    { L"Profile Activated", L"Профиль активирован", L"Профіль активовано" },
    { L"System loaded. F9 - Start/Stop.", L"Система загружена. F9 - Старт/Стоп.", L"Система завантажена. F9 - Старт/Стоп." },
    { L"BACK", L"НАЗАД", L"НАЗАД" },
    { L"Interface Theme:", L"Тема интерфейса:", L"Тема інтерфейсу:" }
};

struct ProfileBinds {
    std::wstring profileName[3];
    WORD keys[6];
    std::wstring spellNames[6][3];
};

ProfileBinds rogueProfile = {
    { L"ROGUE", L"РОГА", L"РОЗБІЙНИК" },
    { '1', '2', '3', '4', '5', '6' },
    {
        { L"Sinister Strike", L"Коварный удар", L"Підступний удар" },
        { L"Slice and Dice", L"Мясорубка", L"М'ясорубка" },
        { L"Eviscerate", L"Потрошение", L"Потрошіння" },
        { L"Killing Spree", L"Череда убийств", L"Шквал вбивств" },
        { L"Adrenaline Rush", L"Выброс адреналина", L"Викид адреналіну" },
        { L"Blade Flurry", L"Шквал клинков", L"Шквал клинків" }
    }
};

ProfileBinds palaProfile = {
    { L"PALADIN", L"ПАЛАДИН", L"ПАЛАДИН" },
    { 'R', '1', '2', 'Q', '4', '5' },
    {
        { L"Judgement", L"Правосудие", L"Правосуддя" },
        { L"Divine Storm", L"Божественная буря", L"Божественна буря" },
        { L"Crusader Strike", L"Удар воина Света", L"Удар воїна Світла" },
        { L"Hammer of Wrath", L"Молот гнева", L"Молот гніву" },
        { L"Exorcism", L"Экзорцизм", L"Екзорцизм" },
        { L"Consecration", L"Освящение", L"Освячення" }
    }
};

ProfileBinds* currentProfile = &palaProfile;
bool botActive = false;
bool isRunning = true;
int castDelayMs = 20;

HWND hMainWnd, hBtnToggle, hBtnProfile, hBtnSave, hBtnSettings, hBtnClose, hStatStatus, hStatProfile, hDelayLabel, hDelayEdit;
HWND hBindsLabels[6], hBindsEdits[6];
HWND hLogEdit;
HWND hChkTopMost, hComboLang, hLangLabel, hComboTheme, hThemeLabel;

HBRUSH bgBrush = NULL, editBrush = NULL, btnBrush = NULL, btnHoverBrush = NULL;
HBRUSH closeBtnBrush = NULL, closeBtnHoverBrush = NULL;
COLORREF textColor, logTextColor, bgColor, editBgColor;

void ApplyTheme() {
    if (bgBrush) DeleteObject(bgBrush);
    if (editBrush) DeleteObject(editBrush);
    if (btnBrush) DeleteObject(btnBrush);
    if (btnHoverBrush) DeleteObject(btnHoverBrush);
    if (closeBtnBrush) DeleteObject(closeBtnBrush);
    if (closeBtnHoverBrush) DeleteObject(closeBtnHoverBrush);

    if (isDarkTheme) {
        bgBrush = CreateSolidBrush(RGB(25, 25, 25));
        editBrush = CreateSolidBrush(RGB(15, 15, 15));
        btnBrush = CreateSolidBrush(RGB(50, 50, 55));
        btnHoverBrush = CreateSolidBrush(RGB(70, 70, 75));
        textColor = RGB(220, 220, 220);
        logTextColor = RGB(0, 255, 0); // Матричный зеленый
        bgColor = RGB(25, 25, 25);
        editBgColor = RGB(15, 15, 15);
    } else {
        bgBrush = CreateSolidBrush(RGB(240, 240, 240));
        editBrush = CreateSolidBrush(RGB(255, 255, 255));
        btnBrush = CreateSolidBrush(RGB(200, 200, 200));
        btnHoverBrush = CreateSolidBrush(RGB(170, 170, 170));
        textColor = RGB(10, 10, 10);
        logTextColor = RGB(0, 0, 200); // Синий для светлой темы
        bgColor = RGB(240, 240, 240);
        editBgColor = RGB(255, 255, 255);
    }
    closeBtnBrush = CreateSolidBrush(RGB(180, 50, 50));
    closeBtnHoverBrush = CreateSolidBrush(RGB(220, 60, 60));

    if (hLogEdit) {
        SendMessage(hLogEdit, EM_SETBKGNDCOLOR, 0, editBgColor);
    }
    if (hMainWnd) InvalidateRect(hMainWnd, NULL, TRUE);
}

void AppendLog(const std::wstring& msg) {
    if (!hLogEdit) return;
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm; localtime_s(&tm, &t);
    wchar_t timeBuf[64];
    swprintf(timeBuf, 64, L"[%02d:%02d:%02d.%03d] ", tm.tm_hour, tm.tm_min, tm.tm_sec, (int)ms.count());
    std::wstring fullMsg = timeBuf + msg + L"\r\n";
    
    // Выделяем конец текста, чтобы применить цвет только к новой строчке
    CHARRANGE cr; cr.cpMin = -1; cr.cpMax = -1;
    SendMessage(hLogEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
    
    // Жестко форсируем цвет именно для места вставки
    CHARFORMAT2W cf; ZeroMemory(&cf, sizeof(cf));
    cf.cbSize = sizeof(cf); cf.dwMask = CFM_COLOR; cf.crTextColor = logTextColor;
    SendMessage(hLogEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

    SendMessage(hLogEdit, EM_REPLACESEL, 0, (LPARAM)fullMsg.c_str());
    SendMessage(hLogEdit, WM_VSCROLL, SB_BOTTOM, 0);
}

void PressKey(WORD vkCode, const std::wstring& spellName) {
    INPUT input = {0}; input.type = INPUT_KEYBOARD; input.ki.wVk = vkCode;
    SendInput(1, &input, sizeof(INPUT));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    input.ki.dwFlags = KEYEVENTF_KEYUP; SendInput(1, &input, sizeof(INPUT));
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
                PressKey(currentProfile->keys[colorId - 1], currentProfile->spellNames[colorId - 1][currentLang]);
                std::this_thread::sleep_for(std::chrono::milliseconds(castDelayMs));
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    ReleaseDC(NULL, hdc);
}

void UpdateUIStrings() {
    SetWindowTextW(hBtnToggle, uiStrs[0][currentLang]);
    SetWindowTextW(hBtnProfile, uiStrs[1][currentLang]);
    SetWindowTextW(hBtnSave, uiStrs[2][currentLang]);
    SetWindowTextW(hBtnSettings, showSettings ? uiStrs[15][currentLang] : uiStrs[3][currentLang]);
    SetWindowTextW(hStatStatus, botActive ? uiStrs[4][currentLang] : uiStrs[5][currentLang]);
    SetWindowTextW(hStatProfile, (std::wstring(uiStrs[6][currentLang]) + currentProfile->profileName[currentLang]).c_str());
    SetWindowTextW(hDelayLabel, uiStrs[7][currentLang]);
    SetWindowTextW(hChkTopMost, uiStrs[8][currentLang]);
    SetWindowTextW(hLangLabel, uiStrs[9][currentLang]);
    SetWindowTextW(hThemeLabel, uiStrs[16][currentLang]);
    
    for (int i = 0; i < 6; i++) {
        SetWindowTextW(hBindsLabels[i], currentProfile->spellNames[i][currentLang].c_str());
    }
}

void LoadBindsToUI() {
    for (int i = 0; i < 6; i++) {
        wchar_t buf[2] = {(wchar_t)currentProfile->keys[i], 0};
        SetWindowTextW(hBindsEdits[i], buf);
    }
    wchar_t delayBuf[10]; swprintf(delayBuf, 10, L"%d", castDelayMs);
    SetWindowTextW(hDelayEdit, delayBuf);
    UpdateUIStrings();
    InvalidateRect(hMainWnd, NULL, TRUE);
}

void SaveBindsFromUI() {
    for (int i = 0; i < 6; i++) {
        wchar_t buf[10]; GetWindowTextW(hBindsEdits[i], buf, 10);
        if (wcslen(buf) > 0) currentProfile->keys[i] = towupper(buf[0]);
    }
    wchar_t delayBuf[10]; GetWindowTextW(hDelayEdit, delayBuf, 10);
    castDelayMs = _wtoi(delayBuf); if (castDelayMs < 0) castDelayMs = 0;
    AppendLog(uiStrs[12][currentLang]);
    LoadBindsToUI();
}

void ToggleSettingsView() {
    showSettings = !showSettings;
    int showMain = showSettings ? SW_HIDE : SW_SHOW;
    int showSet = showSettings ? SW_SHOW : SW_HIDE;
    
    ShowWindow(hStatStatus, showMain); ShowWindow(hStatProfile, showMain);
    ShowWindow(hDelayLabel, showMain); ShowWindow(hDelayEdit, showMain);
    ShowWindow(hLogEdit, showMain);
    for (int i = 0; i < 6; i++) { ShowWindow(hBindsLabels[i], showMain); ShowWindow(hBindsEdits[i], showMain); }
    
    ShowWindow(hChkTopMost, showSet);
    ShowWindow(hComboLang, showSet); ShowWindow(hLangLabel, showSet);
    ShowWindow(hComboTheme, showSet); ShowWindow(hThemeLabel, showSet);
    UpdateUIStrings();
    InvalidateRect(hMainWnd, NULL, TRUE);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            LoadLibraryW(L"Msftedit.dll"); 
            ApplyTheme();
            HFONT hFont = CreateFontW(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
            HFONT hLogFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, L"Consolas");
            
            hBtnClose = CreateWindowW(L"BUTTON", L"X", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 475, 5, 30, 30, hWnd, (HMENU)4, NULL, NULL);
            hBtnToggle = CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 20, 45, 120, 35, hWnd, (HMENU)1, NULL, NULL);
            hBtnProfile = CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 145, 45, 110, 35, hWnd, (HMENU)3, NULL, NULL);
            hBtnSettings = CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 260, 45, 120, 35, hWnd, (HMENU)5, NULL, NULL);
            hBtnSave = CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 385, 45, 105, 35, hWnd, (HMENU)2, NULL, NULL);
            
            hStatStatus = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 20, 95, 300, 20, hWnd, NULL, NULL, NULL);
            hStatProfile = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 20, 115, 300, 20, hWnd, NULL, NULL, NULL);
            hDelayLabel = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 340, 95, 100, 20, hWnd, NULL, NULL, NULL);
            hDelayEdit = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER | ES_NUMBER, 450, 93, 40, 22, hWnd, NULL, NULL, NULL);

            for (int i = 0; i < 6; i++) {
                hBindsLabels[i] = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 20, 150 + (i * 35), 220, 20, hWnd, NULL, NULL, NULL);
                hBindsEdits[i] = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER | ES_UPPERCASE, 250, 148 + (i * 35), 40, 24, hWnd, NULL, NULL, NULL);
                SendMessage(hBindsLabels[i], WM_SETFONT, (WPARAM)hFont, TRUE);
                SendMessage(hBindsEdits[i], WM_SETFONT, (WPARAM)hFont, TRUE);
                SendMessage(hBindsEdits[i], EM_SETLIMITTEXT, 1, 0);
            }

            hLogEdit = CreateWindowExW(0, L"RICHEDIT50W", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 310, 148, 180, 199, hWnd, NULL, NULL, NULL);
            SendMessage(hLogEdit, EM_SETBKGNDCOLOR, 0, editBgColor);

            hChkTopMost = CreateWindowW(L"BUTTON", L"", WS_CHILD | BS_AUTOCHECKBOX, 50, 120, 300, 30, hWnd, (HMENU)6, NULL, NULL);
            SendMessage(hChkTopMost, BM_SETCHECK, isTopMost ? BST_CHECKED : BST_UNCHECKED, 0);
            
            hLangLabel = CreateWindowW(L"STATIC", L"", WS_CHILD, 50, 170, 150, 20, hWnd, NULL, NULL, NULL);
            hComboLang = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL, 200, 168, 150, 100, hWnd, (HMENU)7, NULL, NULL);
            SendMessage(hComboLang, CB_ADDSTRING, 0, (LPARAM)L"English");
            SendMessage(hComboLang, CB_ADDSTRING, 0, (LPARAM)L"Русский");
            SendMessage(hComboLang, CB_ADDSTRING, 0, (LPARAM)L"Українська");
            SendMessage(hComboLang, CB_SETCURSEL, currentLang, 0);

            hThemeLabel = CreateWindowW(L"STATIC", L"", WS_CHILD, 50, 210, 150, 20, hWnd, NULL, NULL, NULL);
            hComboTheme = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL, 200, 208, 150, 100, hWnd, (HMENU)8, NULL, NULL);
            SendMessage(hComboTheme, CB_ADDSTRING, 0, (LPARAM)L"Светлая / Light");
            SendMessage(hComboTheme, CB_ADDSTRING, 0, (LPARAM)L"Темная / Dark");
            SendMessage(hComboTheme, CB_SETCURSEL, isDarkTheme ? 1 : 0, 0);

            HWND elements[] = {hBtnClose, hStatStatus, hStatProfile, hDelayLabel, hDelayEdit, hLogEdit, hChkTopMost, hComboLang, hLangLabel, hComboTheme, hThemeLabel};
            for (HWND el : elements) SendMessage(el, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLogEdit, WM_SETFONT, (WPARAM)hLogFont, TRUE);
            break;
        }
        case WM_MEASUREITEM: {
            LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;
            if (lpmis->CtlType == ODT_MENU) {
                lpmis->itemWidth = 140; // Ширина выпадающего меню
                lpmis->itemHeight = 35; // Высота каждого пункта
                return TRUE;
            }
            break;
        }
        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hWnd, &rc);
            FillRect(hdc, &rc, bgBrush);
            return 1; 
        }
        case WM_PAINT: {
            PAINTSTRUCT ps; HDC hdc = BeginPaint(hWnd, &ps);
            SetTextColor(hdc, textColor); SetBkMode(hdc, TRANSPARENT);
            HFONT hFont = CreateFontW(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
            SelectObject(hdc, hFont); RECT rt = {20, 10, 300, 30};
            DrawTextW(hdc, L"WoW Pixel Bot - Premium Overlay", -1, &rt, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            DeleteObject(hFont); EndPaint(hWnd, &ps);
            break;
        }
        case WM_NCHITTEST: {
            LRESULT hit = DefWindowProcW(hWnd, msg, wParam, lParam);
            if (hit == HTCLIENT) {
                POINT pt; pt.x = (short)LOWORD(lParam); pt.y = (short)HIWORD(lParam);
                ScreenToClient(hWnd, &pt); if (pt.y < 40) return HTCAPTION;
            }
            return hit;
        }
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lParam;
            if (pdis->CtlType == ODT_MENU) {
                // Отрисовка кастомного выпадающего меню профилей
                bool isSelected = (pdis->itemState & ODS_SELECTED);
                FillRect(pdis->hDC, &pdis->rcItem, isSelected ? btnHoverBrush : bgBrush);
                SetTextColor(pdis->hDC, textColor);
                SetBkMode(pdis->hDC, TRANSPARENT);
                
                ProfileBinds* prof = (ProfileBinds*)pdis->itemData;
                std::wstring text = prof->profileName[currentLang];
                if (prof == currentProfile) text = L"\x2713 " + text; // Галочка
                else text = L"    " + text;

                HFONT hFont = CreateFontW(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
                HGDIOBJ oldFont = SelectObject(pdis->hDC, hFont);

                RECT rcText = pdis->rcItem;
                rcText.left += 10;
                DrawTextW(pdis->hDC, text.c_str(), -1, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                
                SelectObject(pdis->hDC, oldFont); DeleteObject(hFont);
                return TRUE;
            }
            else if (pdis->CtlID == 4) {
                FillRect(pdis->hDC, &pdis->rcItem, (pdis->itemState & ODS_SELECTED) ? closeBtnHoverBrush : closeBtnBrush);
            } else if (pdis->CtlID >= 1 && pdis->CtlID <= 5) {
                FillRect(pdis->hDC, &pdis->rcItem, (pdis->itemState & ODS_SELECTED) ? btnHoverBrush : btnBrush);
            } else return FALSE;
            
            SetTextColor(pdis->hDC, (isDarkTheme || pdis->CtlID == 4) ? RGB(255, 255, 255) : RGB(10, 10, 10)); 
            SetBkMode(pdis->hDC, TRANSPARENT);
            wchar_t text[64]; GetWindowTextW(pdis->hwndItem, text, 64);
            DrawTextW(pdis->hDC, text, -1, &pdis->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            return TRUE;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam; HWND hwnd = (HWND)lParam;
            SetBkMode(hdc, TRANSPARENT);
            if (hwnd == hStatStatus) SetTextColor(hdc, botActive ? RGB(0, 200, 0) : RGB(220, 50, 50));
            else if (hwnd == hStatProfile) SetTextColor(hdc, isDarkTheme ? RGB(100, 200, 255) : RGB(0, 100, 200));
            else SetTextColor(hdc, textColor);
            return (INT_PTR)bgBrush;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam; HWND hwnd = (HWND)lParam;
            SetBkMode(hdc, OPAQUE); SetBkColor(hdc, editBgColor);
            SetTextColor(hdc, textColor);
            return (INT_PTR)editBrush;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam); int wmEvent = HIWORD(wParam);
            if (wmEvent == EN_SETFOCUS) {
                for (int i=0; i<6; i++) if ((HWND)lParam == hBindsEdits[i]) SendMessage(hBindsEdits[i], EM_SETSEL, 0, -1);
            }
            if (wmId == 1) {
                botActive = !botActive; UpdateUIStrings();
                AppendLog(botActive ? uiStrs[10][currentLang] : uiStrs[11][currentLang]);
                InvalidateRect(hStatStatus, NULL, TRUE);
            } else if (wmId == 2) {
                SaveBindsFromUI();
            } else if (wmId == 3) {
                RECT rect; GetWindowRect(hBtnProfile, &rect);
                HMENU hMenu = CreatePopupMenu();
                
                MENUINFO mi = { sizeof(MENUINFO) };
                mi.fMask = MIM_BACKGROUND | MIM_STYLE;
                mi.dwStyle = MNS_NOCHECK; // Убираем дефолтные галочки винды
                mi.hbrBack = bgBrush; // Красим сам фон выпадающего списка
                SetMenuInfo(hMenu, &mi);

                // Добавляем наши кастомные отрисовываемые элементы
                AppendMenuW(hMenu, MF_OWNERDRAW, 1001, (LPCWSTR)&rogueProfile);
                AppendMenuW(hMenu, MF_OWNERDRAW, 1002, (LPCWSTR)&palaProfile);
                
                int selection = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_NONOTIFY,
                                               rect.left, rect.bottom, 0, hWnd, NULL);
                DestroyMenu(hMenu);

                if (selection == 1001 && currentProfile != &rogueProfile) {
                    currentProfile = &rogueProfile;
                    LoadBindsToUI();
                    AppendLog(std::wstring(uiStrs[13][currentLang]) + L": " + currentProfile->profileName[currentLang]);
                    InvalidateRect(hMainWnd, NULL, TRUE);
                } else if (selection == 1002 && currentProfile != &palaProfile) {
                    currentProfile = &palaProfile;
                    LoadBindsToUI();
                    AppendLog(std::wstring(uiStrs[13][currentLang]) + L": " + currentProfile->profileName[currentLang]);
                    InvalidateRect(hMainWnd, NULL, TRUE);
                }
            } else if (wmId == 4) {
                isRunning = false; PostQuitMessage(0);
            } else if (wmId == 5) {
                ToggleSettingsView();
            } else if (wmId == 6) { 
                isTopMost = (SendMessage(hChkTopMost, BM_GETCHECK, 0, 0) == BST_CHECKED);
                SetWindowPos(hMainWnd, isTopMost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            } else if (wmId == 7 && wmEvent == CBN_SELCHANGE) { 
                currentLang = (Lang)SendMessage(hComboLang, CB_GETCURSEL, 0, 0);
                UpdateUIStrings();
                InvalidateRect(hMainWnd, NULL, TRUE);
            } else if (wmId == 8 && wmEvent == CBN_SELCHANGE) { 
                isDarkTheme = (SendMessage(hComboTheme, CB_GETCURSEL, 0, 0) == 1);
                ApplyTheme();
            }
            if (wmId >= 1 && wmId <= 5) SetFocus(hWnd);
            break;
        }
        case WM_DESTROY:
            isRunning = false; PostQuitMessage(0);
            break;
        default:
            return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSW wc = {0}; wc.lpfnWndProc = WndProc; wc.hInstance = hInstance; wc.hbrBackground = NULL;
    wc.lpszClassName = L"PixelBotOverlay"; wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    int screenW = GetSystemMetrics(SM_CXSCREEN); int screenH = GetSystemMetrics(SM_CYSCREEN);
    hMainWnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED, L"PixelBotOverlay", L"WoW Pixel Bot", 
                               WS_POPUP | WS_VISIBLE, (screenW - 510) / 2, (screenH - 370) / 2, 510, 370, NULL, NULL, hInstance, NULL);

    SetLayeredWindowAttributes(hMainWnd, 0, 240, LWA_ALPHA); 

    LoadBindsToUI(); UpdateUIStrings();
    AppendLog(uiStrs[14][currentLang]);

    std::thread botThread(BotLoop);
    MSG msg; bool f9_pressed = false;

    while (isRunning) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { isRunning = false; break; }
            TranslateMessage(&msg); DispatchMessage(&msg);
        }

        if (GetAsyncKeyState(VK_F9) & 0x8000) {
            if (!f9_pressed) { 
                botActive = !botActive; UpdateUIStrings(); 
                AppendLog(botActive ? uiStrs[10][currentLang] : uiStrs[11][currentLang]);
                InvalidateRect(hStatStatus, NULL, TRUE); f9_pressed = true; 
            }
        } else f9_pressed = false;

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    botThread.join();
    if (bgBrush) DeleteObject(bgBrush); if (editBrush) DeleteObject(editBrush); if (btnBrush) DeleteObject(btnBrush);
    if (btnHoverBrush) DeleteObject(btnHoverBrush); if (closeBtnBrush) DeleteObject(closeBtnBrush); if (closeBtnHoverBrush) DeleteObject(closeBtnHoverBrush);
    return 0;
}
