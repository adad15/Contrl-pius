#pragma once

/*
 *   CTool�ࣺ������Windowsϵͳ��ʵ���Զ�����Ӧ�ó���Ĺ����Լ���ȡ��ǰʱ�� 
 *      1.��̬��Ա���� -> int AutoInvokeS(CStringW strExeName)��ʹ��ע���ķ�ʽ��Windows��ʵ����������
 *      ��ʹ��ע���ķ�ʽ��Windows��ʵ�֡�
 * 
 *      2.��̬��Ա���� -> int AutoInvokeX(CStringW strExeName):ʹ�ý�����Ŀ�ݷ�ʽ����ϵͳ�������ļ�����������
 *  
 *      3.��̬��Ա���� -> CStringW GetCurrentTime():��ȡ��ǰ��ϵͳʱ��
 * 
 */

class CTool
{
public:
    /*���ó����ڼ��������ʱ�Զ����С���ʹ��ע���ķ�ʽ��Windows��ʵ�֡�*/
    static int AutoInvokeS(CStringW strExeName)
    {
        int ret = 0;
        BOOL isWow64;
        TCHAR tPath[MAX_PATH]{}; //�����������������ʱ��·����Ϣ
        CString strExePath;      //��ǰ�������ڵľ���·��
        CString strSysPath;      //ϵͳĿ¼·��
        CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");  //ע���ָ�������µ��Ӽ�����

        //0.����ϵͳ
        //ʹ�� IsWow64Process ��������鵱ǰ������������64λWindows�ϻ���32λWindows��
        ret = IsWow64Process(GetCurrentProcess()/*��õ�ǰ���̵ľ��*/, &isWow64);
        if (ret == 0)
        {
            //MessageBox(NULL, _T("����ϵͳʧ��"), _T("����"), MB_ICONERROR);
            return -1;
        }

        //1.��õ�ǰEXE�ľ���·����ʹ�� GetModuleFileName ������ȡ����ִ�еĳ����·����
        ret = GetModuleFileName(NULL, tPath, MAX_PATH);
        if (ret == 0)
        {
            //MessageBox(NULL, _T("��õ�ǰEXE�ľ���·��ʧ��"), _T("����"), MB_ICONERROR);
            return -1;
        }
        strExePath = tPath;
        memset(tPath, 0, MAX_PATH * sizeof(TCHAR));

        //2.��ע���
        HKEY hKey;
        /*
         * KEY_ALL_ACCESS ��ʾ����Լ���ȫ������Ȩ�ޣ�������ȡ��д��ȡ�
         * EY_WOW64_64KEY ����64λϵͳ������ʱ�������32λӦ�÷���64λע�����ͼ��ȷ����ʹ��32λӦ����Ҳ����ȷ����ע����
         * hKey һ��ָ�룬ָ��򿪺�ļ��ľ��
        */
        ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
        if (ret != ERROR_SUCCESS)
        {
            //MessageBox(NULL, _T("��ע���ʧ��"), _T("����"), MB_ICONERROR);
            return -1;
        }
        //3.��ѯע�������Ƿ��Ѿ��������Զ���������������ã���������1��
        /*
         * ��ʱ����λ��ע����е�ָ�����Ӽ�λ�ã�run��������أ������Ҹ��Ӽ����Ƿ��иó�����
         * hKey�����Ǵ�ע�����ľ��
         * strExeName��������
         * tPath����Ų�ѯ�����������ֵ
         * &nKeyValueLen��ָ��һ��������ָ�룬�ں�������֮���Ŵ洢��tPath�е����ݵ�ʵ�ʴ�С��
         * 
         */
        long nKeyValueLen = MAX_PATH * sizeof(TCHAR);
        ret = RegQueryValue(hKey, strExeName, tPath, &nKeyValueLen);
        if (ret == ERROR_SUCCESS)
        {
            //�ҵ��ˣ���û��Ҫִ����
            return 1;
        }
        //4.��ȡϵͳĿ¼������ϵͳλ����ʹ�� SHGetSpecialFolderPath ��ȡ��ȷ��ϵͳĿ¼·����
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
            //MessageBox(NULL, _T("��û�ȡϵͳĿ¼ʧ��"), _T("����"), MB_ICONERROR);
            return -1;
        }
        strSysPath = tPath;
        memset(tPath, 0, MAX_PATH * sizeof(TCHAR));
        strSysPath += _T("\\");
        strSysPath += strExeName;

        //5.������ϵͳĿ¼��
        //����ִ���ļ�����ԭʼλ�ø��Ƶ�ϵͳĿ¼
        ret = CopyFile(strExePath, strSysPath, FALSE);
        if (ret == 0)
        {
            //MessageBox(NULL, _T("������ϵͳĿ¼��ʧ��"), _T("����"), MB_ICONERROR);
            return -1;
        }

        //6.д��ע���
        ret = RegSetValueEx(hKey, strExeName, 0, REG_SZ, (BYTE*)(LPCTSTR)strSysPath, strSysPath.GetLength() * sizeof(TCHAR));
        if (ret != ERROR_SUCCESS)
        {
            //MessageBox(NULL, _T("д��ע���ʧ��"), _T("����"), MB_ICONERROR);
            return -1;
        }
        return 1;
    }
    
    /*�������Ҳ���������ó����ڼ��������ʱ�Զ����У�����ʹ�õ��ǽ�����Ŀ�ݷ�ʽ����ϵͳ�������ļ�����*/
    static int AutoInvokeX(CStringW strExeName)
    {
        TCHAR tPath[MAX_PATH]{};
        CString strExePath;
        CString strStartupPath;
        int ret = 0;

        //1.��õ�ǰEXE�ľ���·��
        ret = GetModuleFileName(NULL, tPath, MAX_PATH);
        if (ret == 0)
        {
            //MessageBox(NULL, _T("��õ�ǰEXE�ľ���·��ʧ��"), _T("����"), MB_ICONERROR);
            return -1;
        }
        strExePath = tPath;
        memset(tPath, 0, MAX_PATH * sizeof(TCHAR));

        //2.��ȡ����Ŀ¼
        ret = SHGetSpecialFolderPath(NULL, tPath, CSIDL_COMMON_STARTUP, 0);
        if (ret == FALSE)
        {
            //MessageBox(NULL, _T("��ȡ����Ŀ¼ʧ��"), _T("����"), MB_ICONERROR);
            return -1;
        }
        strStartupPath = tPath;
        strStartupPath += _T("\\");
        strStartupPath += strExeName;
        memset(tPath, 0, MAX_PATH * sizeof(TCHAR));

        //3.�Ƿ��ƹ�
        if (PathFileExists(strStartupPath))
        {
            //���ƹ��ˣ���û��Ҫִ����
            return 1;
        }

        //4.���Ƶ�����Ŀ¼
        ret = CopyFile(strExePath, strStartupPath, FALSE);
        if (ret == 0)
        {
            //MessageBox(NULL, _T("����������Ŀ¼��ʧ��"), _T("����"), MB_ICONERROR);
            return -1;
        }
        return 1;
    }
    /*�������������ȡ��ǰ��ϵͳʱ�䣬���� CStringW �ĸ�ʽ���ء�*/
    static CStringW GetCurrentTime()
    {
        time_t tt = time(NULL);//��䷵�ص�ֻ��һ��ʱ�������1970��1��1�տ�ʼ����
        tm* t = localtime(&tt);//ת������ϸ��ʱ����Ϣ�����ꡢ�¡��ա�Сʱ�����Ӻ��롣
        WCHAR wTime[64]{};
        /*ʹ�� wsprintf ��ʱ���ʽ��Ϊ "��-��-�� ʱ:��:��" �ĸ�ʽ��*/
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