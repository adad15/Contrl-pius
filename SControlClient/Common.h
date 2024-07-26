#pragma once
/*����4996�ž��棬ͨ�����ں���ĳЩ��������Ϊ����ȫ�ľ��档*/
#pragma warning(disable:4996)


#include <string>
/*�趨�ṹ���Ա���ڴ���뷽ʽΪ1�ֽڶ���*/
#pragma pack(push)
#pragma pack(1)

#define _IN_OUT_

#include <io.h>

/*
 *	CPacket => �����ͽ������ݰ�
 *		1.������Ա��������ͷ�������ȡ�������ݡ�У��͡�
 *		2.˽�г�Ա������
 *		3.������Ա������
 *			3.1 ���캯����
 *				Ĭ�Ϲ��캯������ʼ������Ϊ-1.
 *				�������캯�������� =���������ָ�롢���ݳ��ȣ����ݴ������������ݹ������ݰ���������У���
 *				�������캯��������=�����ݰ��������õ����ݳ��ȣ�����������������������Ӹ������ֽ�����������ݰ���������У�����֤
 *			3.2 BYTE* Data()��Ա��������װ�����ݰ���������
 *				������õĳ�Ա�������ϳ�һ��string������sOut������������ָ��BYTE* pData
 *			3.3 DWORD Size()��Ա����������õİ����ݵĴ�С
 *			3.4 ���صȺ��������ʵ�����ݰ�����ĸ�ֵ
 *			3.5 ToString��ToString2�������������ݰ����ַ�����ʾ��������Ժ���־��¼
 *				
 *	�ṹ�����ڣ�������������Ϣ���ļ���Ϣ������¼����û���Ϣ
 *		1.DRIVEINFO �� PDRIVEINFO�����ڴ洢����������Ϣ
 *		2.FILEINFO �� PFILEINFO�����ڴ洢�ļ�����Ϣ�����ڲ�������
 *      3.MOUSEINFO �� PMOUSEINFO�����ڴ洢������Ϣ
 *      4.USERINFO �� PUSERINFO�����ڴ洢�û�����Ϣ
 *		5.MUserInfo�����ڴ洢�û�����ϸ��Ϣ
 *		6.ConnectIds�����ڴ洢���ӵ�ID
 * 
 *	ö�ٱ�����
 *		1.MOUSEBTN����������갴ť������
 *		2.MOUSEEVE������������¼�������
 * 
 *	Dump����������һ���������������ڽ��ֽ�����pData��ָ������len��ÿ��col���ֽڵ���ʽ����ת����dump����
 * 
 *	GetErrInfo���������ڸ��ݴ������ wsaErrCode ��ȡ������Ϣ��
 *	
 *
*/

class CPacket
{
public:
	WORD			nHead;   //��ͷ0xFEFE
	DWORD			nLength; //�����ȣ��ӿ������ʼ������У�������
	WORD			nCmd;    //����
	std::string		sData;   //����
	WORD			nSum;    //У��ͣ�ֻ��������ݵĳ���
private:
	std::string		sOut;

public:
	CPacket() { nCmd = -1; }
	
	/*���ݴ������������ݹ������ݰ���������У���*/
	CPacket(WORD _nCmd, BYTE* _bData = NULL, DWORD _nLength = 0/*����_bData�ĳ���*/)
	{
		nHead = 0xFEFF;
		nLength = sizeof(nCmd) + _nLength + sizeof(nSum);
		nCmd = _nCmd;
		sData.resize(nLength - sizeof(nCmd) - sizeof(nSum)); memcpy((void*)sData.c_str(), _bData, _nLength);
		nSum = 0; for (size_t i = 0; i < _nLength; i++) nSum += ((BYTE)sData[i]) & 0xFF;
	}
	/*�Ӹ������ֽ�����������ݰ���������У�����֤*/
	CPacket(BYTE* bParserAddr, _IN_OUT_ int& len_/*������������ݰ��ĳ��ȣ����������������õ����ݳ���*/)
	{
		int& len = len_;
		//����С����С����������˵������ȫ���޷��������ݰ��������ݿ��ܲ�ȫ�����߰�ͷδ��ȫ�����յ�
		if (int(sizeof(nHead) + sizeof(nLength) + sizeof(nCmd) + 0 + sizeof(nSum)) > len) { len = 0; return; };
		//�Ұ�ͷ
		size_t i = 0;
		/* &bParserAddr[i]��ȡ���� bParserAddr �е� i ��Ԫ�صĵ�ַ�����õ�ַת��Ϊ WORD* ���͵�ָ�� */
		for (; i < len - 1; i++) if (*(WORD*)&bParserAddr[i] == 0xFEFF) { nHead = 0xFEFF; break; };
		//�ҵ��˰�ͷ��������С�������������㣬Ҳ������nLength
		if ((i + (sizeof(nHead) + sizeof(nLength) + sizeof(nCmd) + 0 + sizeof(nSum) - 1) > len)) { len = 0; return; };
		//ָ�����
		i += sizeof(nHead);
		//��������
		nLength = *(DWORD*)&bParserAddr[i];
		//�ҵ��˰��������������������㣬Ҳ������nLength
		if ((i + (sizeof(nLength) + nLength - 1) > len)) { len = 0; return; };
		//ָ������
		i += sizeof(nLength);
		//��������
		nCmd = *(WORD*)&bParserAddr[i];
		//ָ������
		i += sizeof(nCmd);
		//��������
		sData.resize(nLength - sizeof(nCmd) - sizeof(nSum));
		memcpy((void*)sData.c_str(), &bParserAddr[i], sData.size());
		//ָ���У��
		i += sData.size();
		//������У��
		nSum = *(WORD*)&bParserAddr[i];
		//ָ���У�����һλ
		i += sizeof(nSum);
		//��У��
		if (nCmd == 5)
		{
			len = i;
		}
		else
		{
			WORD tSum = 0;
			for (size_t j = 0; j < sData.size(); j++) tSum += ((BYTE)sData[j]) & 0xFF;
			if (tSum == nSum) len = i;//��Ҫ���ǰ�ͷǰ����ܲ�������,һ�����ٵ���
		}
	}
	/* ������õĳ�Ա�������ϳ�һ��string������sOut������������ָ��BYTE* pData */
	BYTE* Data()
	{
		/* pData������unsigned char���ͣ�����ָ��string������sOut */
		sOut.resize(sizeof(nHead) + sizeof(nLength) + nLength);
		BYTE* pData = (BYTE*) sOut.c_str();
		int i = 0;
		*(WORD*)&pData[i] = nHead; i += sizeof(nHead);
		*(DWORD*)&pData[i] = nLength; i += sizeof(nLength);
		*(WORD*)&pData[i] = nCmd; i += sizeof(nCmd);
		memcpy(&pData[i], sData.c_str(), sData.size()); i += sData.size();
		*(WORD*)&pData[i] = nSum; i += sizeof(nSum);
		return pData;
	}
	/* ����õİ����ݵĴ�С */
	DWORD Size()
	{
		return sizeof(nHead) + sizeof(nLength) + nLength;
	}
	CPacket& operator=(const CPacket& _pack)
	{
		nHead		= _pack.nHead;
		nLength		= _pack.nLength;
		nCmd		= _pack.nCmd;
		sData.resize(_pack.sData.size());
		memcpy((void*)sData.c_str(), _pack.sData.c_str(), _pack.sData.length());
		nSum		= _pack.nSum;
		return *this;
	}
	std::string ToString()
	{
		std::string info;
		char chInfo[0xff]{};
		sprintf(chInfo, "%snHead    : %d\r\n", chInfo, nHead		);
		sprintf(chInfo, "%snLength  : %d\r\n", chInfo, nLength		);
		sprintf(chInfo, "%ssDataLen : %lld\r\n", chInfo, sData.size());
		sprintf(chInfo, "%snCmd     : %d\r\n", chInfo, nCmd			);
		sprintf(chInfo, "%snSum     : %d\r\n", chInfo, nSum			);
		info = chInfo;
		return info;
	}
	std::string ToString2()
	{
		std::string info;
		char chInfo[0xff]{};
		sprintf(chInfo, "%snHead : %d ", chInfo, nHead);
		sprintf(chInfo, "%snLength : %d ", chInfo, nLength);
		sprintf(chInfo, "%ssDataLen : %lld ", chInfo, sData.size());
		sprintf(chInfo, "%snCmd : %d ", chInfo, nCmd);
		sprintf(chInfo, "%snSum : %d ", chInfo, nSum);
		info = chInfo;
		return info;
	}
};
/*����ṹ�����ڴ洢����������Ϣ*/
typedef struct drive_info
{
	/*���ڴ洢���������������Դ洢26��������*/
	char drive[26];
	/*��¼������������*/
	char drive_count;
	drive_info()
	{
		memset(drive, 0, 26);
		drive_count = 0;
	}
}DRIVEINFO,*PDRIVEINFO;
/*����ṹ�����ڴ洢�ļ�����Ϣ*/
typedef struct file_info
{
	/*data �� _finddata64i32_t ���͵����ݽṹ�����ڴ洢�ļ���������*/
	_finddata64i32_t data;
	/*��ʾ�ļ���Ϣ�Ƿ�Ϊ��*/
	BOOL isNull;
}FILEINFO, * PFILEINFO;

enum MOUSEBTN
{
	LEFT = 0x1,     //���
	MID = 0x2,      //�м�
	RIGHT = 0x4,	//�Ҽ�
	NOTHING = 0x8,  //û�а���
};

enum MOUSEEVE
{
	CLICK = 0x10,   //����
	DBCLICK = 0x20, //˫��
	DOWN = 0x40,    //����
	UP = 0x80,		//�ɿ�
	MOVE = 0x100,	//�ƶ�
};

typedef struct mouse_info
{
	CPoint		ptXY;    //�洢��������
	MOUSEBTN	nButton; //��ʾ��갴��
	MOUSEEVE	nEvent;  //��ʾ����¼�
}MOUSEINFO, PMOUSEINFO;
/*����ṹ�����ڴ洢�û�����Ϣ*/
typedef struct user_info
{
	CString ip;
	CString port;
	user_info(CString _ip, CString _port)
	{
		ip = _ip;
		port = _port;
	}
}USERINFO,PUSERINFO;
/*����ṹ�����ڴ洢�û�����ϸ��Ϣ*/
struct MUserInfo
{
	int					tcpSock;  //TCP�׽���
	unsigned long long  id;       //�洢�û���ID
	char				ip[16];	  //���ڴ洢IP��ַ
	short				port;     //���ڴ洢�˿ں�
	long long			last;     //���ڼ�¼���һ�λʱ��

	MUserInfo()//��GetTickCount64()��ʼ��id��
	{
		id = GetTickCount64();
	}

	MUserInfo(const char* _ip, const short _port)//��ʼ��ip��port
	{
		memcpy(ip, _ip, sizeof(ip));
		port = _port;
	}
};
/*�洢���ӵ�ID*/
struct ConnectIds
{
	unsigned long long  id0;
	unsigned long long  id1;
};

#pragma pack(pop)
/*���ڽ��ֽ�����pData��ָ������len��ÿ��col���ֽڵ���ʽ����ת����dump��*/
void Dump(BYTE* pData, DWORD len, DWORD col = 16);
/*���ݴ������ wsaErrCode ��ȡ������Ϣ*/
std::string GetErrInfo(int wsaErrCode);