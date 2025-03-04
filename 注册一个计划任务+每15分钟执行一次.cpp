#include <windows.h>
#include <taskschd.h>
#include <comdef.h>
#include <iostream>

#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsupp.lib")

bool CreateScheduledTask(const wchar_t* taskName, const wchar_t* exePath, const wchar_t* triggerType) {
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        std::wcout << L"Failed to initialize COM: " << hr << std::endl;
        return false;
    }

    // 创建任务服务对象
    ITaskService* pService = nullptr;
    hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService);
    if (FAILED(hr))
    {
        std::wcout << L"Failed to create TaskService: " << hr << std::endl;
        CoUninitialize();
        return false;
    }

    hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr))
    {
        std::wcout << L"Failed to connect to TaskService: " << hr << std::endl;
        pService->Release();
        CoUninitialize();
        return false;
    }

    // 获取根任务文件夹
    ITaskFolder* pRootFolder = nullptr;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr))
    {
        std::wcout << L"Failed to get root folder: " << hr << std::endl;
        pService->Release();
        CoUninitialize();
        return false;
    }

    // 创建任务定义
    ITaskDefinition* pTask = nullptr;
    hr = pService->NewTask(0, &pTask);
    if (FAILED(hr))
    {
        std::wcout << L"Failed to create task: " << hr << std::endl;
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return false;
    }

    // 创建触发器
    ITriggerCollection* pTriggerCollection = nullptr;
    hr = pTask->get_Triggers(&pTriggerCollection);
    if (FAILED(hr))
    {
        std::wcout << L"Failed to get trigger collection: " << hr << std::endl;
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return false;
    }

    ITrigger* pTrigger = nullptr;
    hr = pTriggerCollection->Create(TASK_TRIGGER_TIME, &pTrigger);
    if (FAILED(hr))
    {
        std::wcout << L"Failed to create trigger: " << hr << std::endl;
        pTriggerCollection->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return false;
    }

    ITimeTrigger* pTimeTrigger = nullptr;
    hr = pTrigger->QueryInterface(IID_ITimeTrigger, (void**)&pTimeTrigger);
    if (FAILED(hr))
    {
        std::wcout << L"Failed to query ITimeTrigger: " << hr << std::endl;
        pTrigger->Release();
        pTriggerCollection->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        CoUninitialize();
        return false;
    }

    // 设置触发器属性
    pTimeTrigger->put_Id(_bstr_t(L"Trigger1"));
    pTimeTrigger->put_StartBoundary(_bstr_t(L"2025-03-03T00:00:00")); // 开始时间

    // 设置重复间隔（每15分钟）
    IRepetitionPattern* pRepetition = nullptr;
    hr = pTimeTrigger->get_Repetition(&pRepetition);
    if (SUCCEEDED(hr))
    {
        hr = pRepetition->put_Interval(_bstr_t(L"PT15M")); // 每15分钟
        if (FAILED(hr))
        {
            std::wcout << L"Failed to set repetition interval: " << hr << std::endl;
        }
        pRepetition->Release();
    }
    else
    {
        std::wcout << L"Failed to get repetition pattern: " << hr << std::endl;
    }

    pTimeTrigger->Release();
    pTrigger->Release();
    pTriggerCollection->Release();

    // 创建动作
    IActionCollection* pActionCollection = nullptr;
    hr = pTask->get_Actions(&pActionCollection);
    if (SUCCEEDED(hr))
    {
        IAction* pAction = nullptr;
        hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
        if (SUCCEEDED(hr))
        {
            IExecAction* pExecAction = nullptr;
            hr = pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
            if (SUCCEEDED(hr))
            {
                pExecAction->put_Path(_bstr_t(exePath));
                pExecAction->Release();
            }
            pAction->Release();
        }
        pActionCollection->Release();
    }

    // 注册任务
    IRegisteredTask* pRegisteredTask = nullptr;
    hr = pRootFolder->RegisterTaskDefinition(
        _bstr_t(taskName),
        pTask,
        TASK_CREATE_OR_UPDATE,
        _variant_t(),
        _variant_t(),
        TASK_LOGON_INTERACTIVE_TOKEN,
        _variant_t(L""),
        &pRegisteredTask
    );

    if (SUCCEEDED(hr))
    {
        std::wcout << L"Task registered successfully: " << taskName << std::endl;
        if (pRegisteredTask) pRegisteredTask->Release();
    }
    else
    {
        std::wcout << L"Failed to register task: " << hr << std::endl;
    }

    // 清理
    pTask->Release();
    pRootFolder->Release();
    pService->Release();
    CoUninitialize();

    return SUCCEEDED(hr);
}

int wmain() {
    CreateScheduledTask(
        L"RunEvery15Minutes",
        L"D:\\project\\cpp\\TEST\\x64\\Release\\PCHardInfo.exe",
        L"interval"
    );

    return 0;
}
