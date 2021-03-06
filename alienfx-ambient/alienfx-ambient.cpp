// alienfx-ambient.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "alienfx-ambient.h"
#include "DaramCam/DaramCam.h"
#pragma comment ( lib, "DaramCam.lib" )
#include "CaptureHelper.h"
#include "ConfigHandler.h"
#include "FXHelper.h"
#include <windowsx.h>
#include <CommCtrl.h>
#include <Commdlg.h>
#include <shellapi.h>
#include "..\AlienFX-SDK\AlienFX_SDK\AlienFX_SDK.h"
#include <algorithm>

#pragma comment(linker, \
  "\"/manifestdependency:type='Win32' "\
  "name='Microsoft.Windows.Common-Controls' "\
  "version='6.0.0.0' "\
  "processorArchitecture='*' "\
  "publicKeyToken='6595b64144ccf1df' "\
  "language='*'\"")
#pragma comment(lib,"Version.lib")

#define MAX_LOADSTRING 100

FXHelper* fxhl;
CaptureHelper* cap;
ConfigHandler* conf;

BOOL CALLBACK DialogConfigStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
NOTIFYICONDATA niData;

// Forward declarations of functions included in this code module:
HWND                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);

    // Perform application initialization:
    HWND hDlg;

    conf = new ConfigHandler();
    fxhl = new FXHelper(conf);
    conf->Load();

    if (!(hDlg=InitInstance (hInstance, nCmdShow)))
    {
        return FALSE;
    }

    cap = new CaptureHelper(hDlg, conf, fxhl);
    //HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ALIENFXAMBIENT));

    MSG msg; //bool ret;

    RegisterPowerSettingNotification(hDlg, &GUID_MONITOR_POWER_ON, 0);

    //cap->Start();

    // Main message loop:
    while ((GetMessage(&msg, 0, 0, 0)) != 0) {
        //if (!IsDialogMessage(hDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        //}
    }

    cap->Stop();
    conf->Save();
    delete cap;
    delete fxhl;
    delete conf;

    return 0;// (int)msg.wParam;
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND dlg;
   dlg = CreateDialogParam(hInstance,//GetModuleHandle(NULL),         /// instance handle
    	MAKEINTRESOURCE(IDD_DIALOG_MAIN),    /// dialog box template
    	NULL,                    /// handle to parent
    	(DLGPROC)DialogConfigStatic, 0);
   if (!dlg) return NULL;

   SendMessage(dlg, WM_SETICON, ICON_BIG, (LPARAM) LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ALIENFXAMBIENT)));
   SendMessage(dlg, WM_SETICON, ICON_SMALL, (LPARAM) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ALIENFXAMBIENT), IMAGE_ICON, 16, 16, 0));

   ShowWindow(dlg, nCmdShow);

   return dlg;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG: {
        HRSRC hResInfo;
        DWORD dwSize;
        HGLOBAL hResData;
        LPVOID pRes, pResCopy;
        UINT uLen;
        VS_FIXEDFILEINFO* lpFfi;

        HWND version_text = GetDlgItem(hDlg, IDC_STATIC_VERSION);

        hResInfo = FindResource(hInst, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
        dwSize = SizeofResource(hInst, hResInfo);
        hResData = LoadResource(hInst, hResInfo);
        pRes = LockResource(hResData);
        pResCopy = LocalAlloc(LMEM_FIXED, dwSize);
        CopyMemory(pResCopy, pRes, dwSize);
        FreeResource(hResData);

        VerQueryValue(pResCopy, TEXT("\\"), (LPVOID*)&lpFfi, &uLen);
        char buf[255];

        DWORD dwFileVersionMS = lpFfi->dwFileVersionMS;
        DWORD dwFileVersionLS = lpFfi->dwFileVersionLS;

        sprintf_s(buf, 255, "Version: %d.%d.%d.%d", HIWORD(dwFileVersionMS), LOWORD(dwFileVersionMS), HIWORD(dwFileVersionLS), LOWORD(dwFileVersionLS));

        Static_SetText(version_text, buf);

        LocalFree(pResCopy);
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    case WM_NOTIFY:
        switch (LOWORD(wParam)) {
        case IDC_SYSLINK_HOMEPAGE:
            switch (((LPNMHDR)lParam)->code)
            {

            case NM_CLICK:
            case NM_RETURN:
                ShellExecute(NULL, "open", "https://github.com/T-Troll/alienfx-tools", NULL, NULL, SW_SHOWNORMAL);
                break;
            } break;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

mapping* FindMapping(int did, int lid) {
    for (int i = 0; i < conf->mappings.size(); i++)
        if (conf->mappings[i].devid == did && conf->mappings[i].lightid == lid)
                return &conf->mappings[i];
    return NULL;
}

BOOL CALLBACK DialogConfigStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    HWND dev_list = GetDlgItem(hDlg, IDC_DEVICE);
    HWND light_list = GetDlgItem(hDlg, IDC_LIGHTS);
    HWND divider = GetDlgItem(hDlg, IDC_EDIT_DIVIDER);
    HWND brSlider = GetDlgItem(hDlg, IDC_SLIDER_BR);
    mapping* map = NULL;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        int pid = AlienFX_SDK::Functions::GetPID();
        size_t lights = AlienFX_SDK::Functions::GetMappings()->size();
        size_t numdev = AlienFX_SDK::Functions::GetDevices()->size();

        if (pid == -1) {
            std::string devName = "No device found";
            int pos = (int)SendMessage(dev_list, CB_ADDSTRING, 0, (LPARAM)(devName.c_str()));
            SendMessage(dev_list, CB_SETITEMDATA, pos, (LPARAM)pid);
        }
        else {
            int cpid = (-1), cpos = (-1);
            for (int i = 0; i < numdev; i++) {
                cpid = AlienFX_SDK::Functions::GetDevices()->at(i).devid;
                std::string dname = AlienFX_SDK::Functions::GetDevices()->at(i).name;
                int pos = (int)SendMessage(dev_list, CB_ADDSTRING, 0, (LPARAM)(dname.c_str()));
                SendMessage(dev_list, CB_SETITEMDATA, pos, (LPARAM)cpid);
                if (cpid == pid) {
                    // select this device.
                    SendMessage(dev_list, CB_SETCURSEL, pos, (LPARAM)0);
                    cpos = pos;
                }
            }
            if (cpos == -1) { // device have no name!
                char devName[256];
                sprintf_s(devName, 255, "Device #%X", pid);
                int pos = (int)SendMessage(dev_list, CB_ADDSTRING, 0, (LPARAM)(devName));
                SendMessage(dev_list, CB_SETITEMDATA, pos, (LPARAM)pid);
                SendMessage(dev_list, CB_SETCURSEL, pos, (LPARAM)0);
            }
            for (int i = 0; i < lights; i++) {
                AlienFX_SDK::mapping lgh = AlienFX_SDK::Functions::GetMappings()->at(i);
                if (lgh.devid == pid && AlienFX_SDK::Functions::GetFlags(pid, lgh.lightid) == 0) {
                    int pos = (int)SendMessage(light_list, LB_ADDSTRING, 0, (LPARAM)(TEXT(lgh.name.c_str())));
                    SendMessage(light_list, LB_SETITEMDATA, pos, (LPARAM)lgh.lightid);
                }
            }
        }
        // divider....
        SetDlgItemInt(hDlg, IDC_EDIT_DIVIDER, conf->divider, false);
        // Mode...
        if (conf->mode) {
            CheckDlgButton(hDlg, IDC_RADIO_PRIMARY, BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_RADIO_SECONDARY, BST_CHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_RADIO_PRIMARY, BST_CHECKED);
            CheckDlgButton(hDlg, IDC_RADIO_SECONDARY, BST_UNCHECKED);
        }
        CheckDlgButton(hDlg, IDC_CHECK_GAMMA, conf->gammaCorrection ? BST_CHECKED : BST_UNCHECKED);
        SendMessage(brSlider, TBM_SETRANGE, true, MAKELPARAM(0, 256));
        SendMessage(brSlider, TBM_SETPOS, true, conf->shift);
        SendMessage(brSlider, TBM_SETTICFREQ, 32, 0);
    } break;
    case WM_COMMAND:
    {
        int lbItem = (int)SendMessage(light_list, LB_GETCURSEL, 0, 0);
        int lid = (int)SendMessage(light_list, LB_GETITEMDATA, lbItem, 0);
        lbItem = (int)SendMessage(dev_list, CB_GETCURSEL, 0, 0);
        int did = (int)SendMessage(dev_list, CB_GETITEMDATA, lbItem, 0);
        switch (LOWORD(wParam))
        {
        case IDOK: case IDCANCEL: case IDCLOSE: case IDM_EXIT:
        {
            cap->Stop();
            Shell_NotifyIcon(NIM_DELETE, &niData);
            DestroyWindow(hDlg); 
        } break;
        case IDM_ABOUT: // about dialogue here
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, About);
            break;
        case IDC_DEVICE: {
            switch (HIWORD(wParam))
            {
            case CBN_SELCHANGE: {
                    size_t lights = AlienFX_SDK::Functions::GetMappings()->size();
                    AlienFX_SDK::Functions::AlienFXChangeDevice(did);
                    SendMessage(light_list, CB_RESETCONTENT, 0, 0);
                    for (int i = 0; i < lights; i++) {
                        AlienFX_SDK::mapping lgh = AlienFX_SDK::Functions::GetMappings()->at(i);
                        if (lgh.devid == did && AlienFX_SDK::Functions::GetFlags(did, lgh.lightid) == 0) { // should be did
                            int pos = (int)SendMessage(light_list, LB_ADDSTRING, 0, (LPARAM)(TEXT(lgh.name.c_str())));
                            SendMessage(light_list, LB_SETITEMDATA, pos, (LPARAM)lgh.lightid);
                        }
                    }
                    RedrawWindow(light_list, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
            } break;
            }
        } break;// should reload dev list
        case IDC_LIGHTS: // should reload mappings
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE: {
                // check in config - do we have mappings?
                map = FindMapping(did, lid);
                if (map == NULL) {
                    mapping newmap;
                    newmap.devid = did;
                    newmap.lightid = lid;
                    conf->mappings.push_back(newmap);
                    std::sort(conf->mappings.begin(), conf->mappings.end(), ConfigHandler::sortMappings);
                    map = FindMapping(did, lid);
                }
                // load zones....
                UINT bid = IDC_CHECK1;
                // clear checks...
                for (int i = 0; i < 12; i++) {
                    CheckDlgButton(hDlg, bid + i, BST_UNCHECKED);
                }
                for (int j = 0; j < map->map.size(); j++) {
                    CheckDlgButton(hDlg, bid+map->map[j], BST_CHECKED);
                }
            } break;
        } break;
        case IDC_BUTTON1: case IDC_BUTTON2: case IDC_BUTTON3: case IDC_BUTTON4: case IDC_BUTTON5: case IDC_BUTTON6: case IDC_BUTTON7:
        case IDC_BUTTON8: case IDC_BUTTON9: case IDC_BUTTON10: case IDC_BUTTON11: case IDC_BUTTON12: {
            switch (HIWORD(wParam))
            {
            case BN_CLICKED: {
                UINT id = LOWORD(wParam) - IDC_BUTTON1;
                UINT bid = IDC_CHECK1 + id;
                map = FindMapping(did, lid);
                if (map != NULL) {
                    // add mapping
                    std::vector <unsigned char>::iterator Iter = map->map.begin();
                    int i = 0;
                    for (i = 0; i < map->map.size(); i++)
                        if (map->map[i] == id)
                            break;
                        else
                            Iter++;
                    HWND cBid = GetDlgItem(hDlg, bid);
                    if (i == map->map.size()) {
                        // new mapping, add and select
                        map->map.push_back(id);
                        CheckDlgButton(hDlg, bid, BST_CHECKED);
                    }
                    else {
                        map->map.erase(Iter);
                        CheckDlgButton(hDlg, bid, BST_UNCHECKED);
                    }
                }
            } break;
            }
        } break;
        case IDC_EDIT_DIVIDER:
            switch (HIWORD(wParam)) {
            case EN_UPDATE: {
                conf->divider = GetDlgItemInt(hDlg, IDC_EDIT_DIVIDER, NULL, false);
                if (conf->divider <= 0) conf->divider = 1;
            } break;
        } break;
        case IDC_RADIO_PRIMARY:
            switch (HIWORD(wParam)) {
            case BN_CLICKED:
                CheckDlgButton(hDlg, IDC_RADIO_PRIMARY, BST_CHECKED);
                CheckDlgButton(hDlg, IDC_RADIO_SECONDARY, BST_UNCHECKED);
                conf->mode = 0;
                //restart capture....
                cap->Restart();
                break;
            }
        break;
        case IDC_RADIO_SECONDARY:
            switch (HIWORD(wParam)) {
            case BN_CLICKED:
                CheckDlgButton(hDlg, IDC_RADIO_PRIMARY, BST_UNCHECKED);
                CheckDlgButton(hDlg, IDC_RADIO_SECONDARY, BST_CHECKED);
                conf->mode = 1;
                cap->Restart();
                break;
            }
            break;
        case IDC_CHECK_GAMMA:
            conf->gammaCorrection = (IsDlgButtonChecked(hDlg, LOWORD(wParam)) == BST_CHECKED);
            break;
        case IDC_BUTTON_MIN:
            // go to tray...
            ZeroMemory(&niData, sizeof(NOTIFYICONDATA));
            niData.cbSize = sizeof(NOTIFYICONDATA);
            niData.uID = IDI_ALIENFXAMBIENT;
            niData.uFlags = NIF_ICON | NIF_MESSAGE;
            niData.hIcon =
                (HICON)LoadImage(GetModuleHandle(NULL),
                    MAKEINTRESOURCE(IDI_ALIENFXAMBIENT),
                    IMAGE_ICON,
                    GetSystemMetrics(SM_CXSMICON),
                    GetSystemMetrics(SM_CYSMICON),
                    LR_DEFAULTCOLOR);
            niData.hWnd = hDlg;
            niData.uCallbackMessage = WM_APP + 1;
            Shell_NotifyIcon(NIM_ADD, &niData);
            ShowWindow(hDlg, SW_HIDE);
            break;
        case IDC_BUTTON_RESET:
            AlienFX_SDK::Functions::AlienFXChangeDevice(did);
            cap->Restart();
            break;
        default: return false;
        }
    } break;
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED) {
            // go to tray...
            ZeroMemory(&niData, sizeof(NOTIFYICONDATA));
            niData.cbSize = sizeof(NOTIFYICONDATA);
            niData.uID = IDI_ALIENFXAMBIENT;
            niData.uFlags = NIF_ICON | NIF_MESSAGE;
            niData.hIcon =
                (HICON)LoadImage(GetModuleHandle(NULL),
                    MAKEINTRESOURCE(IDI_ALIENFXAMBIENT),
                    IMAGE_ICON,
                    GetSystemMetrics(SM_CXSMICON),
                    GetSystemMetrics(SM_CYSMICON),
                    LR_DEFAULTCOLOR);
            niData.hWnd = hDlg;
            niData.uCallbackMessage = WM_APP + 1;
            Shell_NotifyIcon(NIM_ADD, &niData);
            ShowWindow(hDlg, SW_HIDE);
        } break;
    case WM_APP + 1: {
        switch (lParam)
        {
        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONUP:
            ShowWindow(hDlg, SW_RESTORE);
            SetWindowPos(hDlg,       // handle to window
                HWND_TOPMOST,  // placement-order handle
                0,     // horizontal position
                0,      // vertical position
                0,  // width
                0, // height
                SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE// window-positioning options
            );
            SetWindowPos(hDlg,       // handle to window
                HWND_NOTOPMOST,  // placement-order handle
                0,     // horizontal position
                0,      // vertical position
                0,  // width
                0, // height
                SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE// window-positioning options
            );
            Shell_NotifyIcon(NIM_DELETE, &niData);
            break;
            case WM_RBUTTONUP:
            case WM_CONTEXTMENU:
                cap->Stop();
                Shell_NotifyIcon(NIM_DELETE, &niData);
                DestroyWindow(hDlg);
                break;
        }
        break;
    } break;
    case WM_HSCROLL:
        switch (LOWORD(wParam)) {
        case TB_THUMBPOSITION: case TB_ENDTRACK:
            if ((HWND)lParam == brSlider) {
                conf->shift = (DWORD) SendMessage(brSlider, TBM_GETPOS, 0, 0);
            }
            break;
        }
        break;
    case WM_CLOSE: cap->Stop(); DestroyWindow(hDlg); break;
    case WM_DESTROY: PostQuitMessage(0); break;
    case WM_POWERBROADCAST:
        switch (wParam) {
        case PBT_APMRESUMEAUTOMATIC: case PBT_APMPOWERSTATUSCHANGE:
            if (wParam == PBT_APMRESUMEAUTOMATIC) {
                //resumed from sleep
                cap->Restart();
            }
            break;
        case PBT_POWERSETTINGCHANGE:
            POWERBROADCAST_SETTING* sParams = (POWERBROADCAST_SETTING*)lParam;
            if (sParams->PowerSetting == GUID_MONITOR_POWER_ON) {
                if (sParams->Data[0] == 0) {
                    cap->Stop();
                    fxhl->FadeToBlack();
                }
                else
                    cap->Start();
            }
            break;
        }
        break;
    case WM_DISPLAYCHANGE:
        // Monitor configuration changed
        cap->Restart();
        break;
    default: return false;
    }
    return true;
}