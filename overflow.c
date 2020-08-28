异步方式
打开VC++6.0，新建基于对话框的工程RS485Comm，在主对话框窗口IDD_RS485COMM_DIALOG上添加两个按钮，ID分别为 IDC_SEND和IDC_RECEIVE，标题分别为“发送”和“接收”；添加一个静态文本框IDC_DISP，用于显示串口接收到的内容。在RS485CommDlg.cpp文件中添加全局变量：

HANDLE hCom; //全局变量，

串口句柄在RS485CommDlg.cpp文件中的OnInitDialog()函数添加如下代码：

hCom=CreateFile("COM1",//COM1口
GENERIC_READ|GENERIC_WRITE, //允许读和写
0, //独占方式
NULL,
OPEN_EXISTING, //打开而不是创建
FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, //重叠方式
NULL);
if(hCom==(HANDLE)-1)
{
AfxMessageBox("打开COM失败!");
return FALSE;
}

SetupComm(hCom,100,100); //输入缓冲区和输出缓冲区的大小都是100

COMMTIMEOUTS TimeOuts;
//设定读超时
TimeOuts.ReadIntervalTimeout=MAXDWORD;
TimeOuts.ReadTotalTimeoutMultiplier=0;
TimeOuts.ReadTotalTimeoutConstant=0;
//在读一次输入缓冲区的内容后读操作就立即返回，
//而不管是否读入了要求的字符。


//设定写超时
TimeOuts.WriteTotalTimeoutMultiplier=100;
TimeOuts.WriteTotalTimeoutConstant=500;
SetCommTimeouts(hCom,&TimeOuts); //设置超时

DCB dcb;
GetCommState(hCom,&dcb);
dcb.BaudRate=9600; //波特率为9600
dcb.ByteSize=8; //每个字节有8位
dcb.Parity=NOPARITY; //无奇偶校验位
dcb.StopBits=TWOSTOPBITS; //两个停止位
SetCommState(hCom,&dcb);

PurgeComm(hCom,PURGE_TXCLEAR|PURGE_RXCLEAR);

分别双击IDC_SEND按钮和IDC_RECEIVE按钮，添加两个按钮的响应函数：

void CRS485CommDlg::OnSend() 
{
// TODO: Add your control notification handler code here
OVERLAPPED m_osWrite;
memset(&m_osWrite,0,sizeof(OVERLAPPED));
m_osWrite.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);


char lpOutBuffer[7];
memset(lpOutBuffer,''\0'',7);
lpOutBuffer[0]=''\x11'';
lpOutBuffer[1]=''0'';
lpOutBuffer[2]=''0'';
lpOutBuffer[3]=''1'';
lpOutBuffer[4]=''0'';
lpOutBuffer[5]=''1'';
lpOutBuffer[6]=''\x03'';

DWORD dwBytesWrite=7;
COMSTAT ComStat;
DWORD dwErrorFlags;
BOOL bWriteStat;
ClearCommError(hCom,&dwErrorFlags,&ComStat);
bWriteStat=WriteFile(hCom,lpOutBuffer,
dwBytesWrite,& dwBytesWrite,&m_osWrite);

if(!bWriteStat)
{
if(GetLastError()==ERROR_IO_PENDING)
{
WaitForSingleObject(m_osWrite.hEvent,1000);
}
}

}

void CRS485CommDlg::OnReceive() 
{
// TODO: Add your control notification handler code here
OVERLAPPED m_osRead;
memset(&m_osRead,0,sizeof(OVERLAPPED));
m_osRead.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);

COMSTAT ComStat;
DWORD dwErrorFlags;

char str[100];
memset(str,''\0'',100);
DWORD dwBytesRead=100;//读取的字节数
BOOL bReadStat;

ClearCommError(hCom,&dwErrorFlags,&ComStat);
dwBytesRead=min(dwBytesRead, (DWORD)ComStat.cbInQue);
bReadStat=ReadFile(hCom,str,
dwBytesRead,&dwBytesRead,&m_osRead);
if(!bReadStat)
{
if(GetLastError()==ERROR_IO_PENDING)
    //GetLastError()函数返回ERROR_IO_PENDING,表明串口正在进行读操作
{
WaitForSingleObject(m_osRead.hEvent,2000);
    //使用WaitForSingleObject函数等待，直到读操作完成或延时已达到2秒钟
    //当串口读操作进行完毕后，m_osRead的hEvent事件会变为有信号
}
}

PurgeComm(hCom, PURGE_TXABORT|
PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
m_disp=str;
UpdateData(FALSE);
}

打开ClassWizard,为静态文本框IDC_DISP添加CString类型变量m_disp，同时添加WM_CLOSE的相应函数：

void CRS485CommDlg::OnClose() 
{
// TODO: Add your message handler code here and/or call default
    CloseHandle(hCom); //程序退出时关闭串口
CDialog::OnClose();
}
