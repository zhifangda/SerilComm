打开VC++6.0，新建基于对话框的工程RS485Comm，在主对话框窗口 IDD_RS485COMM_DIALOG上添加两个按钮，ID分别为IDC_SEND和IDC_RECEIVE，标题分别为“发送”和“接收”；添加一个静态文本框IDC_DISP，用于显示串口接收到的内容。

在RS485CommDlg.cpp文件中添加全局变量：

HANDLE hCom;  //全局变量，串口句柄

在RS485CommDlg.cpp文件中的OnInitDialog()函数添加如下代码：

// TODO: Add extra initialization here
hCom=CreateFile("COM1",//COM1口
GENERIC_READ|GENERIC_WRITE, //允许读和写
0, //独占方式
NULL,
OPEN_EXISTING, //打开而不是创建
0, //同步方式
NULL);
if(hCom==(HANDLE)-1)
{
AfxMessageBox("打开COM失败!");
return FALSE;
}

SetupComm(hCom,100,100); //输入缓冲区和输出缓冲区的大小都是1024

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
// 在此需要简单介绍百特公司XMA5000的通讯协议：
//该仪表RS485通讯采用主机广播方式通讯。
//串行半双工，帧11位，1个起始位(0)，8个数据位，2个停止位(1)
//如：读仪表显示的瞬时值，主机发送：DC1 AAA BB ETX
//其中：DC1是标准ASCII码的一个控制符号，码值为11H(十进制的17)
//在XMA5000的通讯协议中，DC1表示读瞬时值
//AAA是从机地址码，也就是XMA5000显示仪表的通讯地址
//BB为通道号，读瞬时值时该值为01
//ETX也是标准ASCII码的一个控制符号，码值为03H
//在XMA5000的通讯协议中，ETX表示主机结束符

char lpOutBuffer[7];
memset(lpOutBuffer,''\0'',7); //前7个字节先清零
lpOutBuffer[0]=''\x11'';  //发送缓冲区的第1个字节为DC1
lpOutBuffer[1]=''0'';  //第2个字节为字符0(30H)
lpOutBuffer[2]=''0''; //第3个字节为字符0(30H)
lpOutBuffer[3]=''1''; // 第4个字节为字符1(31H)
lpOutBuffer[4]=''0''; //第5个字节为字符0(30H)
lpOutBuffer[5]=''1''; //第6个字节为字符1(31H)
lpOutBuffer[6]=''\x03''; //第7个字节为字符ETX
//从该段代码可以看出，仪表的通讯地址为001 
DWORD dwBytesWrite=7;
COMSTAT ComStat;
DWORD dwErrorFlags;
BOOL bWriteStat;
ClearCommError(hCom,&dwErrorFlags,&ComStat);
bWriteStat=WriteFile(hCom,lpOutBuffer,dwBytesWrite,& dwBytesWrite,NULL);
if(!bWriteStat)
{
AfxMessageBox("写串口失败!");
}

}
void CRS485CommDlg::OnReceive() 
{
// TODO: Add your control notification handler code here

char str[100];
memset(str,''\0'',100);
DWORD wCount=100;//读取的字节数
BOOL bReadStat;
bReadStat=ReadFile(hCom,str,wCount,&wCount,NULL);
if(!bReadStat)
AfxMessageBox("读串口失败!");
PurgeComm(hCom, PURGE_TXABORT|
PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
m_disp=str;
UpdateData(FALSE);

}

您可以观察返回的字符串，其中有和仪表显示值相同的部分，您可以进行相应的字符串操作取出仪表的显示值。
打开ClassWizard,为静态文本框IDC_DISP添加CString类型变量m_disp，同时添加WM_CLOSE的相应函数：

void CRS485CommDlg::OnClose() 
{
// TODO: Add your message handler code here and/or call default
    CloseHandle(hCom); //程序退出时关闭串口
CDialog::OnClose();
}
