#pragma once

/*
 *   CTool类：用于在Windows系统中实现自动启动应用程序的功能以及获取当前时间 
 *      1.静态成员函数 -> int AutoInvokeS(CStringW strExeName)：使用注册表的方式在Windows中实现自启动。
 *      它使用注册表的方式在Windows中实现。
 * 
 *      2.静态成员函数 -> int AutoInvokeX(CStringW strExeName):使用将程序的快捷方式放在系统的启动文件夹中自启动
 *  
 *      3.静态成员函数 -> CStringW GetCurrentTime():获取当前的系统时间
 * 
 */

class CTool
{
public:
    /*设置程序在计算机启动时自动运行。它使用注册表的方式在Windows中实现。*/
    static int AutoInvokeS(CStringW strExeName)
    {
        int ret = 0;
        BOOL isWow64;
        TCHAR tPath[MAX_PATH]{}; //缓存区，用来存放临时的路径信息
        CString strExePath;      //当前程序所在的绝对路径
        CString strSysPath;      //系统目录路径
        CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");  //注册表指定根键下的子键名称

        //0.区分系统
        //使用 IsWow64Process 函数来检查当前进程是运行在64位Windows上还是32位Windows上
        ret = IsWow64Process(GetCurrentProcess()/*获得当前进程的句柄*/, &isWow64);
        if (ret == 0)
        {
            //MessageBox(NULL, _T("区分系统失败"), _T("错误"), MB_ICONERROR);
            return -1;
        }

        //1.获得当前EXE的绝对路径，使用 GetModuleFileName 函数获取正在执行的程序的路径。
        ret = GetModuleFileName(NULL, tPath, MAX_PATH);
        if (ret == 0)
        {
            //MessageBox(NULL, _T("获得当前EXE的绝对路径失败"), _T("错误"), MB_ICONERROR);
            return -1;
        }
        strExePath = tPath;
        memset(tPath, 0, MAX_PATH * sizeof(TCHAR));

        //2.打开注册表
        HKEY hKey;
        /*
         * KEY_ALL_ACCESS 表示请求对键的全部访问权限，包括读取、写入等。
         * EY_WOW64_64KEY 当在64位系统上运行时，这会让32位应用访问64位注册表视图，确保即使在32位应用中也能正确处理注册表项。
         * hKey 一个指针，指向打开后的键的句柄
        */
        ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
        if (ret != ERROR_SUCCESS)
        {
            //MessageBox(NULL, _T("打开注册表失败"), _T("错误"), MB_ICONERROR);
            return -1;
        }
        //3.查询注册表，检查是否已经设置了自动启动，如果已设置，函数返回1。
        /*
         * 此时程序定位到注册表中的指定的子键位置（run和启动相关），查找该子键下是否有该程序名
         * hKey：这是打开注册表键的句柄
         * strExeName：程序名
         * tPath：存放查询结果，即键的值
         * &nKeyValueLen：指向一个变量的指针，在函数调用之后存放存储在tPath中的数据的实际大小。
         * 
         */
        long nKeyValueLen = MAX_PATH * sizeof(TCHAR);
        ret = RegQueryValue(hKey, strExeName, tPath, &nKeyValueLen);
        if (ret == ERROR_SUCCESS)
        {
            //找到了，就没必要执行了
            return 1;
        }
        //4.获取系统目录，根据系统位数，使用 SHGetSpecialFolderPath 获取正确的系统目录路径。
        if (isWow64 != 0)
        {
            ret = SHGetSpecialFolderPath(NULL, tPath, CSIDL_SYSTEMX86, 0);
        }
        else
        {
            ret = SHGetSpecialFolderPath(NULL, tPath, CSIDL_SYSTEM, 0);
        }
        if (ret == 0)
        {
            //MessageBox(NULL, _T("获得获取系统目录失败"), _T("错误"), MB_ICONERROR);
            return -1;
        }
        strSysPath = tPath;
        memset(tPath, 0, MAX_PATH * sizeof(TCHAR));
        strSysPath += _T("\\");
        strSysPath += strExeName;

        //5.拷贝到系统目录下
        //将可执行文件从其原始位置复制到系统目录
        ret = CopyFile(strExePath, strSysPath, FALSE);
        if (ret == 0)
        {
            //MessageBox(NULL, _T("拷贝到系统目录下失败"), _T("错误"), MB_ICONERROR);
            return -1;
        }

        //6.写入注册表
        ret = RegSetValueEx(hKey, strExeName, 0, REG_SZ, (BYTE*)(LPCTSTR)strSysPath, strSysPath.GetLength() * sizeof(TCHAR));
        if (ret != ERROR_SUCCESS)
        {
            //MessageBox(NULL, _T("写入注册表失败"), _T("错误"), MB_ICONERROR);
            return -1;
        }
        return 1;
    }
    
    /*这个函数也是用来设置程序在计算机启动时自动运行，但它使用的是将程序的快捷方式放在系统的启动文件夹中*/
    static int AutoInvokeX(CStringW strExeName)
    {
        TCHAR tPath[MAX_PATH]{};
        CString strExePath;
        CString strStartupPath;
        int ret = 0;

        //1.获得当前EXE的绝对路径
        ret = GetModuleFileName(NULL, tPath, MAX_PATH);
        if (ret == 0)
        {
            //MessageBox(NULL, _T("获得当前EXE的绝对路径失败"), _T("错误"), MB_ICONERROR);
            return -1;
        }
        strExePath = tPath;
        memset(tPath, 0, MAX_PATH * sizeof(TCHAR));

        //2.获取启动目录
        ret = SHGetSpecialFolderPath(NULL, tPath, CSIDL_COMMON_STARTUP, 0);
        if (ret == FALSE)
        {
            //MessageBox(NULL, _T("获取启动目录失败"), _T("错误"), MB_ICONERROR);
            return -1;
        }
        strStartupPath = tPath;
        strStartupPath += _T("\\");
        strStartupPath += strExeName;
        memset(tPath, 0, MAX_PATH * sizeof(TCHAR));

        //3.是否复制过
        if (PathFileExists(strStartupPath))
        {
            //复制过了，就没必要执行了
            return 1;
        }

        //4.复制到启动目录
        ret = CopyFile(strExePath, strStartupPath, FALSE);
        if (ret == 0)
        {
            //MessageBox(NULL, _T("拷贝到启动目录下失败"), _T("错误"), MB_ICONERROR);
            return -1;
        }
        return 1;
    }
    /*这个函数用来获取当前的系统时间，并以 CStringW 的格式返回。*/
    static CStringW GetCurrentTime()
    {
        time_t tt = time(NULL);//这句返回的只是一个时间戳，从1970年1月1日开始计算
        tm* t = localtime(&tt);//转换成详细的时间信息，如年、月、日、小时、分钟和秒。
        WCHAR wTime[64]{};
        /*使用 wsprintf 将时间格式化为 "年-月-日 时:分:秒" 的格式。*/
        wsprintf(wTime,L"%d-%02d-%02d %02d:%02d:%02d\n",
            t->tm_year + 1900,
            t->tm_mon + 1,
            t->tm_mday,
            t->tm_hour,
            t->tm_min,
            t->tm_sec);
        return CStringW(wTime);
    }
};