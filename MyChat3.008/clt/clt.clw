; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=DlgMyChat
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "clt.h"
LastPage=0

ClassCount=2

ResourceCount=3
Resource1=IDD_CONNECTION_INFO
Class1=DlgConnect
Class2=DlgMyChat
Resource2=IDD_CHAT
Resource3=IDR_CHAT

[DLG:IDD_CONNECTION_INFO]
Type=1
Class=DlgConnect
ControlCount=10
Control1=IDC_STATIC,static,1342308352
Control2=IDC_TCP_IP,button,1342341129
Control3=IDC_SPX_IPX,button,1342341129
Control4=IDC_STATIC,static,1342308352
Control5=IDC_SERVER,edit,1350631424
Control6=IDC_STATIC,static,1342308352
Control7=IDC_PORT,edit,1350631426
Control8=IDC_STATIC,static,1342308352
Control9=IDC_NICKNAME,edit,1350631552
Control10=IDOK,button,1342275585

[DLG:IDD_CHAT]
Type=1
Class=DlgMyChat
ControlCount=4
Control1=IDOK,button,1342275585
Control2=IDC_CHAT_MSG,listbox,1353793793
Control3=IDC_MY_MSG,edit,1350631552
Control4=IDC_NICKS,listbox,1352728835

[CLS:DlgConnect]
Type=0
HeaderFile=DlgConnect.h
ImplementationFile=DlgConnect.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_NICKNAME

[CLS:DlgMyChat]
Type=0
HeaderFile=DlgMyChat.h
ImplementationFile=DlgMyChat.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=DlgMyChat

[MNU:IDR_CHAT]
Type=1
Class=?
Command1=ID_FILE_EXIT
Command2=ID_HELP_HELP
Command3=ID_HELP_ABOUT
CommandCount=3

