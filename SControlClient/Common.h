#pragma once
/*禁用4996号警告，通常用于忽略某些函数被认为不安全的警告。*/
#pragma warning(disable:4996)


#include <string>
/*设定结构体成员的内存对齐方式为1字节对齐*/
#pragma pack(push)
#pragma pack(1)

#define _IN_OUT_

#include <io.h>

/*
 *	CPacket => 构建和解析数据包
 *		1.公共成员变量：包头、包长度、命令、数据、校验和。
 *		2.私有成员变量：
 *		3.公共成员函数：
 *			3.1 构造函数：
 *				默认构造函数：初始化命令为-1.
 *				参数构造函数：参数 =》命令、数据指针、数据长度；根据传入的命令和数据构建数据包，并计算校验和
 *				参数构造函数：参数=》数据包，解析好的数据长度（传入参数、传出参数）；从给定的字节数组解析数据包，并进行校验和验证
 *			3.2 BYTE* Data()成员函数：封装了数据包操作方法
 *				将打包好的成员变量整合成一个string数据流sOut，返回数据流指针BYTE* pData
 *			3.3 DWORD Size()成员函数：打包好的包数据的大小
 *			3.4 重载等号运算符：实现数据包对象的赋值
 *			3.5 ToString、ToString2函数：返回数据包的字符串表示，方便调试和日志记录
 *				
 *	结构体用于：管理驱动器信息、文件信息、鼠标事件和用户信息
 *		1.DRIVEINFO 和 PDRIVEINFO：用于存储驱动器的信息
 *		2.FILEINFO 和 PFILEINFO：用于存储文件的信息，用于查找数据
 *      3.MOUSEINFO 和 PMOUSEINFO：用于存储鼠标的信息
 *      4.USERINFO 和 PUSERINFO：用于存储用户的信息
 *		5.MUserInfo：用于存储用户的详细信息
 *		6.ConnectIds：用于存储连接的ID
 * 
 *	枚举变量：
 *		1.MOUSEBTN：定义了鼠标按钮的类型
 *		2.MOUSEEVE：定义了鼠标事件的类型
 * 
 *	Dump函数：这是一个函数声明，用于将字节数据pData按指定长度len以每行col个字节的形式进行转储（dump）。
 * 
 *	GetErrInfo函数：用于根据错误代码 wsaErrCode 获取错误信息。
 *	
 *
*/

class CPacket
{
public:
	WORD			nHead;   //包头0xFEFE
	DWORD			nLength; //包长度（从控制命令开始，到和校验结束）
	WORD			nCmd;    //命令
	std::string		sData;   //数据
	WORD			nSum;    //校验和，只检验包数据的长度
private:
	std::string		sOut;

public:
	CPacket() { nCmd = -1; }
	
	/*根据传入的命令和数据构建数据包，并计算校验和*/
	CPacket(WORD _nCmd, BYTE* _bData = NULL, DWORD _nLength = 0/*数据_bData的长度*/)
	{
		nHead = 0xFEFF;
		nLength = sizeof(nCmd) + _nLength + sizeof(nSum);
		nCmd = _nCmd;
		sData.resize(nLength - sizeof(nCmd) - sizeof(nSum)); memcpy((void*)sData.c_str(), _bData, _nLength);
		nSum = 0; for (size_t i = 0; i < _nLength; i++) nSum += ((BYTE)sData[i]) & 0xFF;
	}
	/*从给定的字节数组解析数据包，并进行校验和验证*/
	CPacket(BYTE* bParserAddr, _IN_OUT_ int& len_/*传入参数，数据包的长度；传出参数，解析好的数据长度*/)
	{
		int& len = len_;
		//长度小于最小整个包长，说明包不全，无法解析数据包；包数据可能不全，或者包头未能全部接收到
		if (int(sizeof(nHead) + sizeof(nLength) + sizeof(nCmd) + 0 + sizeof(nSum)) > len) { len = 0; return; };
		//找包头
		size_t i = 0;
		/* &bParserAddr[i]获取数组 bParserAddr 中第 i 个元素的地址，将该地址转换为 WORD* 类型的指针 */
		for (; i < len - 1; i++) if (*(WORD*)&bParserAddr[i] == 0xFEFF) { nHead = 0xFEFF; break; };
		//找到了包头，按照最小的整个包长来算，也超出了nLength
		if ((i + (sizeof(nHead) + sizeof(nLength) + sizeof(nCmd) + 0 + sizeof(nSum) - 1) > len)) { len = 0; return; };
		//指向包长
		i += sizeof(nHead);
		//解析包长
		nLength = *(DWORD*)&bParserAddr[i];
		//找到了包长，按照整个包长来算，也超出了nLength
		if ((i + (sizeof(nLength) + nLength - 1) > len)) { len = 0; return; };
		//指向命令
		i += sizeof(nLength);
		//解析命令
		nCmd = *(WORD*)&bParserAddr[i];
		//指向数据
		i += sizeof(nCmd);
		//解析数据
		sData.resize(nLength - sizeof(nCmd) - sizeof(nSum));
		memcpy((void*)sData.c_str(), &bParserAddr[i], sData.size());
		//指向和校验
		i += sData.size();
		//解析和校验
		nSum = *(WORD*)&bParserAddr[i];
		//指向和校验的下一位
		i += sizeof(nSum);
		//和校验
		if (nCmd == 5)
		{
			len = i;
		}
		else
		{
			WORD tSum = 0;
			for (size_t j = 0; j < sData.size(); j++) tSum += ((BYTE)sData[j]) & 0xFF;
			if (tSum == nSum) len = i;//不要忘记包头前面可能残余数据,一起销毁掉。
		}
	}
	/* 将打包好的成员变量整合成一个string数据流sOut，返回数据流指针BYTE* pData */
	BYTE* Data()
	{
		/* pData类型是unsigned char类型，用来指向string数据流sOut */
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
	/* 打包好的包数据的大小 */
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
/*这个结构体用于存储驱动器的信息*/
typedef struct drive_info
{
	/*用于存储驱动器名，最多可以存储26个驱动器*/
	char drive[26];
	/*记录驱动器的数量*/
	char drive_count;
	drive_info()
	{
		memset(drive, 0, 26);
		drive_count = 0;
	}
}DRIVEINFO,*PDRIVEINFO;
/*这个结构体用于存储文件的信息*/
typedef struct file_info
{
	/*data 是 _finddata64i32_t 类型的数据结构，用于存储文件查找数据*/
	_finddata64i32_t data;
	/*表示文件信息是否为空*/
	BOOL isNull;
}FILEINFO, * PFILEINFO;

enum MOUSEBTN
{
	LEFT = 0x1,     //左键
	MID = 0x2,      //中键
	RIGHT = 0x4,	//右键
	NOTHING = 0x8,  //没有按键
};

enum MOUSEEVE
{
	CLICK = 0x10,   //单击
	DBCLICK = 0x20, //双击
	DOWN = 0x40,    //按下
	UP = 0x80,		//松开
	MOVE = 0x100,	//移动
};

typedef struct mouse_info
{
	CPoint		ptXY;    //存储鼠标的坐标
	MOUSEBTN	nButton; //表示鼠标按键
	MOUSEEVE	nEvent;  //表示鼠标事件
}MOUSEINFO, PMOUSEINFO;
/*这个结构体用于存储用户的信息*/
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
/*这个结构体用于存储用户的详细信息*/
struct MUserInfo
{
	int					tcpSock;  //TCP套接字
	unsigned long long  id;       //存储用户的ID
	char				ip[16];	  //用于存储IP地址
	short				port;     //用于存储端口号
	long long			last;     //用于记录最后一次活动时间

	MUserInfo()//用GetTickCount64()初始化id。
	{
		id = GetTickCount64();
	}

	MUserInfo(const char* _ip, const short _port)//初始化ip和port
	{
		memcpy(ip, _ip, sizeof(ip));
		port = _port;
	}
};
/*存储连接的ID*/
struct ConnectIds
{
	unsigned long long  id0;
	unsigned long long  id1;
};

#pragma pack(pop)
/*用于将字节数据pData按指定长度len以每行col个字节的形式进行转储（dump）*/
void Dump(BYTE* pData, DWORD len, DWORD col = 16);
/*根据错误代码 wsaErrCode 获取错误信息*/
std::string GetErrInfo(int wsaErrCode);