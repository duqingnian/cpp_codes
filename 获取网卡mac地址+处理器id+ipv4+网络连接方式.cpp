#include <iostream>
#include <string>
#include <Windows.h>
#include <Iphlpapi.h>
#include <WbemIdl.h>
#include <comdef.h>
#include <sstream>
#include <iomanip>

// 链接所需的库
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "comsuppw.lib")

// 获取 MAC 地址、IPv4 地址和连接类型
void getNetworkInfo(std::string& macAddress, std::string& ipAddress, std::string& connectionType) {
    ULONG outBufLen = 0;
    GetAdaptersInfo(NULL, &outBufLen); // 获取缓冲区大小
    PIP_ADAPTER_INFO pAdapterInfo = (PIP_ADAPTER_INFO)malloc(outBufLen);
    if (pAdapterInfo == NULL)
    {
        std::cerr << "内存分配失败。" << std::endl;
        return;
    }

    if (GetAdaptersInfo(pAdapterInfo, &outBufLen) == ERROR_SUCCESS)
    {
        PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
        while (pAdapter)
        {
            if (pAdapter->AddressLength > 0)
            {
                // 获取 MAC 地址
                std::stringstream ss;
                for (unsigned int i = 0; i < pAdapter->AddressLength; i++)
                {
                    if (i > 0) ss << ":";
                    ss << std::hex << std::setw(2) << std::setfill('0') << (int)pAdapter->Address[i];
                }
                macAddress = ss.str();

                // 获取 IPv4 地址
                ipAddress = pAdapter->IpAddressList.IpAddress.String;

                // 判断连接类型
                if (pAdapter->Type == MIB_IF_TYPE_ETHERNET)
                {
                    connectionType = "有线连接";
                }
                else if (pAdapter->Type == IF_TYPE_IEEE80211)
                {
                    connectionType = "无线连接";
                }
                else
                {
                    connectionType = "其他类型连接";
                }

                free(pAdapterInfo);
                return; // 返回第一个有效的适配器信息
            }
            pAdapter = pAdapter->Next;
        }
    }
    else
    {
        std::cerr << "获取适配器信息失败。" << std::endl;
    }
    free(pAdapterInfo);
}

// 获取处理器 ID
std::string getProcessorId() {
    std::string processorId = "";
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        std::cerr << "初始化 COM 失败。错误代码: " << hr << std::endl;
        return processorId;
    }

    hr = CoInitializeSecurity(
        NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL
    );
    if (FAILED(hr))
    {
        std::cerr << "设置 COM 安全级别失败。错误代码: " << hr << std::endl;
        CoUninitialize();
        return processorId;
    }

    IWbemLocator* pLoc = NULL;
    IWbemServices* pSvc = NULL;
    IEnumWbemClassObject* pEnum = NULL;
    IWbemClassObject* pClass = NULL;
    ULONG uReturn = 0;

    hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
    if (FAILED(hr))
    {
        std::cerr << "创建 IWbemLocator 失败。错误代码: " << hr << std::endl;
        goto cleanup;
    }

    hr = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
    if (FAILED(hr))
    {
        std::cerr << "连接 WMI 服务失败。错误代码: " << hr << std::endl;
        goto cleanup;
    }

    hr = pSvc->ExecQuery(_bstr_t(L"WQL"), _bstr_t(L"SELECT ProcessorId FROM Win32_Processor"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnum);
    if (FAILED(hr))
    {
        std::cerr << "查询失败。错误代码: " << hr << std::endl;
        goto cleanup;
    }

    hr = pEnum->Next(WBEM_INFINITE, 1, &pClass, &uReturn);
    if (SUCCEEDED(hr) && uReturn == 1)
    {
        VARIANT vtProp;
        hr = pClass->Get(L"ProcessorId", 0, &vtProp, NULL, NULL);
        if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR)
        {
            processorId = _bstr_t(vtProp.bstrVal);
            VariantClear(&vtProp);
        }
        else
        {
            std::cerr << "获取处理器 ID 失败。错误代码: " << hr << std::endl;
        }
    }
    else
    {
        std::cerr << "未找到处理器或查询失败。错误代码: " << hr << std::endl;
    }

cleanup:
    if (pClass) pClass->Release();
    if (pEnum) pEnum->Release();
    if (pSvc) pSvc->Release();
    if (pLoc) pLoc->Release();
    CoUninitialize();
    return processorId;
}

/*
输出：
MAC 地址: 18:3f:32:f3:d6:a7
IPv4 地址: 172.20.10.4
连接类型: 无线连接
处理器 ID: BFFBFBFB100903BA
按任意键退出...
*/
int main() {
    std::string macAddress, ipAddress, connectionType;
    getNetworkInfo(macAddress, ipAddress, connectionType);
    std::string processorId = getProcessorId();

    std::cout << "MAC 地址: " << (macAddress.empty() ? "未找到" : macAddress) << std::endl;
    std::cout << "IPv4 地址: " << (ipAddress.empty() ? "未找到" : ipAddress) << std::endl;
    std::cout << "连接类型: " << (connectionType.empty() ? "未找到" : connectionType) << std::endl;
    std::cout << "处理器 ID: " << (processorId.empty() ? "未找到" : processorId) << std::endl;

    std::cout << "按任意键退出..." << std::endl;
    std::cin.get();
    return 0;
}
