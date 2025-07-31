#include <windows.h>
#include <detours/detours.h>
#include <dinput.h>
#include <shlwapi.h>

#pragma comment(lib, "detours.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "shlwapi.lib")

decltype(&DirectInput8Create) TrueDirectInput8Create = DirectInput8Create;
decltype(&CreateWindowExA) TrueCreateWindowExA = CreateWindowExA;

double dAspectRatio;
WNDPROC oldWndProc;

LRESULT CALLBACK SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL bMaximizeFlag = FALSE;
    if (uMsg == WM_GETMINMAXINFO)
    {
        MINMAXINFO *mmi = (MINMAXINFO *)lParam;
        RECT rc;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
        LONG height = rc.bottom - rc.top;
        LONG width = static_cast<LONG>(height * dAspectRatio);
        mmi->ptMaxSize.x = width;
        mmi->ptMaxSize.y = height;
        mmi->ptMaxTrackSize.x = width;
        mmi->ptMaxTrackSize.y = height;

        mmi->ptMaxPosition.x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    }
    else if (uMsg > WM_APP && !bMaximizeFlag)
    {
        ShowWindow(hwnd, SW_MAXIMIZE);
        bMaximizeFlag = TRUE;
    }
    return CallWindowProc(oldWndProc, hwnd, uMsg, wParam, lParam);
}

class MyDirectInputDevice8Wrapper : public IDirectInputDevice8
{
public:
    MyDirectInputDevice8Wrapper(IDirectInputDevice8 *original) : original(original)
    {
        original->AddRef();
    }

    ~MyDirectInputDevice8Wrapper()
    {
        original->Release();
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override
    {
        return original->QueryInterface(riid, ppvObject);
    }

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        return original->AddRef();
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG refCount = original->Release();
        if (refCount == 0)
        {
            delete this;
        }
        return refCount;
    }

    HRESULT STDMETHODCALLTYPE GetCapabilities(LPDIDEVCAPS lpDIDevCaps) override
    {
        return original->GetCapabilities(lpDIDevCaps);
    }

    HRESULT STDMETHODCALLTYPE EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags) override
    {
        return original->EnumObjects(lpCallback, pvRef, dwFlags);
    }

    HRESULT STDMETHODCALLTYPE GetProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph) override
    {
        return original->GetProperty(rguidProp, pdiph);
    }

    HRESULT STDMETHODCALLTYPE SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph) override
    {
        return original->SetProperty(rguidProp, pdiph);
    }

    HRESULT STDMETHODCALLTYPE Acquire() override
    {
        return original->Acquire();
    }

    HRESULT STDMETHODCALLTYPE Unacquire() override
    {
        return original->Unacquire();
    }

    HRESULT STDMETHODCALLTYPE GetDeviceState(DWORD cbData, LPVOID lpvData) override
    {
        HRESULT hr = original->GetDeviceState(cbData, lpvData);
        if (SUCCEEDED(hr))
        {
            DIJOYSTATE *pState = (DIJOYSTATE *)lpvData;
            switch (pState->rgdwPOV[0])
            {
            case 0:
                pState->lY = -1000; // push stick up
                break;
            case 4500:
                pState->lY = -1000; // push stick up
                pState->lX = 1000;  // push stick right
                break;
            case 9000:
                pState->lX = 1000; // push stick right
                break;
            case 13500:
                pState->lY = 1000; // push stick down
                pState->lX = 1000; // push stick right
                break;
            case 18000:
                pState->lY = 1000; // push stick down
                break;
            case 22500:
                pState->lY = 1000;  // push stick down
                pState->lX = -1000; // push stick left
                break;
            case 27000:
                pState->lX = -1000; // push stick left
                break;
            case 31500:
                pState->lY = -1000; // push stick up
                pState->lX = -1000; // push stick left
                break;
            default:
                break;
            }
        }

        return hr;
    }

    HRESULT STDMETHODCALLTYPE GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) override
    {
        return original->GetDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags);
    }

    HRESULT STDMETHODCALLTYPE SetDataFormat(LPCDIDATAFORMAT lpdf) override
    {
        return original->SetDataFormat(lpdf);
    }

    HRESULT STDMETHODCALLTYPE SetEventNotification(HANDLE hEvent) override
    {
        return original->SetEventNotification(hEvent);
    }

    HRESULT STDMETHODCALLTYPE SetCooperativeLevel(HWND hwnd, DWORD dwFlags) override
    {
        return original->SetCooperativeLevel(hwnd, dwFlags);
    }

    HRESULT STDMETHODCALLTYPE GetObjectInfo(LPDIDEVICEOBJECTINSTANCE pdidoi, DWORD dwObj, DWORD dwHow) override
    {
        return original->GetObjectInfo(pdidoi, dwObj, dwHow);
    }

    HRESULT STDMETHODCALLTYPE GetDeviceInfo(LPDIDEVICEINSTANCE pdidi) override
    {
        return original->GetDeviceInfo(pdidi);
    }

    HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags) override
    {
        return original->RunControlPanel(hwndOwner, dwFlags);
    }

    HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion, const GUID &guid) override
    {
        return original->Initialize(hinst, dwVersion, guid);
    }

    HRESULT STDMETHODCALLTYPE CreateEffect(REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT *ppdeff, IUnknown *pUnkOuter) override
    {
        return original->CreateEffect(rguid, lpeff, ppdeff, pUnkOuter);
    }

    HRESULT STDMETHODCALLTYPE EnumEffects(LPDIENUMEFFECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwEffType) override
    {
        return original->EnumEffects(lpCallback, pvRef, dwEffType);
    }

    HRESULT STDMETHODCALLTYPE GetEffectInfo(LPDIEFFECTINFO pdei, REFGUID rguid) override
    {
        return original->GetEffectInfo(pdei, rguid);
    }

    HRESULT STDMETHODCALLTYPE GetForceFeedbackState(LPDWORD pdwOut) override
    {
        return original->GetForceFeedbackState(pdwOut);
    }

    HRESULT STDMETHODCALLTYPE SendForceFeedbackCommand(DWORD dwFlags) override
    {
        return original->SendForceFeedbackCommand(dwFlags);
    }

    HRESULT STDMETHODCALLTYPE EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags) override
    {
        return original->EnumCreatedEffectObjects(lpCallback, pvRef, dwFlags);
    }

    HRESULT STDMETHODCALLTYPE Escape(LPDIEFFESCAPE pDieff) override
    {
        return original->Escape(pDieff);
    }

    HRESULT STDMETHODCALLTYPE Poll() override
    {
        return original->Poll();
    }

    HRESULT STDMETHODCALLTYPE SendDeviceData(DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) override
    {
        return original->SendDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags);
    }

    HRESULT STDMETHODCALLTYPE EnumEffectsInFile(LPCSTR lpszFileName, LPDIENUMEFFECTSINFILECALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags) override
    {
        return original->EnumEffectsInFile(lpszFileName, lpCallback, pvRef, dwFlags);
    }

    HRESULT STDMETHODCALLTYPE WriteEffectToFile(LPCSTR lpszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags) override
    {
        return original->WriteEffectToFile(lpszFileName, dwEntries, rgDiFileEft, dwFlags);
    }

    HRESULT STDMETHODCALLTYPE BuildActionMap(LPDIACTIONFORMAT lpdiActionFormat, LPCSTR ptszUserName, DWORD dwFlags) override
    {
        return original->BuildActionMap(lpdiActionFormat, ptszUserName, dwFlags);
    }

    HRESULT STDMETHODCALLTYPE SetActionMap(LPDIACTIONFORMAT lpdiActionFormat, LPCSTR ptszUserName, DWORD dwFlags) override
    {
        return original->SetActionMap(lpdiActionFormat, ptszUserName, dwFlags);
    }

    HRESULT STDMETHODCALLTYPE GetImageInfo(LPDIDEVICEIMAGEINFOHEADER pdidiih) override
    {
        return original->GetImageInfo(pdidiih);
    }

private:
    IDirectInputDevice8 *original;
};

class MyDirectInput8Wrapper : public IDirectInput8
{
public:
    MyDirectInput8Wrapper(IDirectInput8 *original) : original(original)
    {
        original->AddRef();
    }

    ~MyDirectInput8Wrapper()
    {
        original->Release();
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override
    {
        return original->QueryInterface(riid, ppvObject);
    }

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        return original->AddRef();
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG refCount = original->Release();
        if (refCount == 0)
        {
            delete this;
        }
        return refCount;
    }

    HRESULT STDMETHODCALLTYPE CreateDevice(REFGUID rguid, IDirectInputDevice8 **ppDevice, IUnknown *pUnkOuter) override
    {
        HRESULT hr = original->CreateDevice(rguid, ppDevice, pUnkOuter);
        if (SUCCEEDED(hr) && ppDevice && *ppDevice)
        {
            IDirectInputDevice8 *device = *ppDevice;
            DIDEVICEINSTANCE info = {0};
            info.dwSize = sizeof(info);
            if (SUCCEEDED(device->GetDeviceInfo(&info)))
            {
                if (StrStrI(info.tszProductName, TEXT("Xbox")))
                {
                    *ppDevice = new MyDirectInputDevice8Wrapper(*ppDevice);
                }
            }
        }
        return hr;
    }

    HRESULT STDMETHODCALLTYPE EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags) override
    {
        return original->EnumDevices(dwDevType, lpCallback, pvRef, dwFlags);
    }

    HRESULT STDMETHODCALLTYPE GetDeviceStatus(REFGUID rguidInstance) override
    {
        return original->GetDeviceStatus(rguidInstance);
    }

    HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags) override
    {
        return original->RunControlPanel(hwndOwner, dwFlags);
    }

    HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion) override
    {
        return original->Initialize(hinst, dwVersion);
    }

    HRESULT STDMETHODCALLTYPE FindDevice(REFGUID rguidClass, LPCSTR pwszName, LPGUID pguidInstance) override
    {
        return original->FindDevice(rguidClass, pwszName, pguidInstance);
    }

    HRESULT STDMETHODCALLTYPE EnumDevicesBySemantics(LPCSTR ptszUserName, LPDIACTIONFORMAT lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCB lpCallback, LPVOID pvRef, DWORD dwFlags) override
    {
        return original->EnumDevicesBySemantics(ptszUserName, lpdiActionFormat, lpCallback, pvRef, dwFlags);
    }

    HRESULT STDMETHODCALLTYPE ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMS lpdiCDParams, DWORD dwFlags, LPVOID pvRefData) override
    {
        return original->ConfigureDevices(lpdiCallback, lpdiCDParams, dwFlags, pvRefData);
    }

private:
    IDirectInput8 *original;
};

HWND WINAPI MyCreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
    dAspectRatio = (double)nWidth / (double)nHeight;
    dwStyle = WS_SYSMENU | WS_MAXIMIZEBOX;
    HWND result = TrueCreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    oldWndProc = (WNDPROC)SetWindowLongPtr(result, GWLP_WNDPROC, (LONG_PTR)SubclassProc);
    return result;
}

HRESULT WINAPI MyDirectInput8Create(
    HINSTANCE hinst,
    DWORD dwVersion,
    REFIID riid,
    LPVOID *ppvOut,
    LPUNKNOWN punkOuter)
{
    HRESULT hr = TrueDirectInput8Create(hinst, dwVersion, riid, ppvOut, punkOuter);
    if (SUCCEEDED(hr) && IsEqualIID(riid, IID_IDirectInput8))
    {
        IDirectInput8 *original = (IDirectInput8 *)(*ppvOut);
        *ppvOut = new MyDirectInput8Wrapper(original);
    }
    return hr;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DetourRestoreAfterWith();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID &)TrueDirectInput8Create, MyDirectInput8Create);
        DetourAttach(&(PVOID &)TrueCreateWindowExA, MyCreateWindowExA);
        DetourTransactionCommit();
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID &)TrueDirectInput8Create, MyDirectInput8Create);
        DetourDetach(&(PVOID &)TrueCreateWindowExA, MyCreateWindowExA);
        DetourTransactionCommit();
    }
    return TRUE;
}