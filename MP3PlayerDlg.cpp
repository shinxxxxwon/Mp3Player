
// MP3PlayerDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "MP3Player.h"
#include "MP3PlayerDlg.h"
#include "afxdialogex.h"
#include "lameHelper.h"
#include "vfw.h"
#include <cstdlib>
#include <Windows.h>
#include <mciapi.h>
#include "tagHelper.h"

#pragma comment( lib, "Winmm.lib" )


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WAV_FILE_PATH "C:\\MP3\\"


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMP3PlayerDlg 대화 상자



CMP3PlayerDlg::CMP3PlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMP3PlayerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMP3PlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_Player, m_ListBox);
	DDX_Control(pDX, IDC_BUTTON_PLAY, m_btnPlay);
	DDX_Control(pDX, IDC_BUTTON_PAUSE, m_btnPause);
	DDX_Control(pDX, IDC_EDIT_VOLUM, m_EditVol);
	DDX_Control(pDX, IDC_SLIDER_VOLUM, m_SliderVol);
	DDX_Control(pDX, IDC_SLIDER_TIMER, m_SliderTimer);
	DDX_Control(pDX, IDC_CHECK_LOOP, m_CheckLoop);
	DDX_Control(pDX, IDC_CHECK_RANDOM, m_CheckRandom);
	DDX_Control(pDX, IDC_STATIC_TIMER, m_staticTIMER);
}

BEGIN_MESSAGE_MAP(CMP3PlayerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_UPLOAD, &CMP3PlayerDlg::OnBnClickedButtonUpload)
	ON_BN_CLICKED(IDC_BUTTON_RMOVE, &CMP3PlayerDlg::OnBnClickedButtonRmove)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, &CMP3PlayerDlg::OnBnClickedButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CMP3PlayerDlg::OnBnClickedButtonPause)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CMP3PlayerDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, &CMP3PlayerDlg::OnBnClickedButtonNext)
	ON_BN_CLICKED(IDC_BUTTON_PREV, &CMP3PlayerDlg::OnBnClickedButtonPrev)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_EDIT_VOLUM, &CMP3PlayerDlg::OnEnChangeEditVolum)
	ON_WM_TIMER()
	ON_LBN_DBLCLK(IDC_LIST_Player, &CMP3PlayerDlg::OnLbnDblclkListPlayer)
END_MESSAGE_MAP()


// CMP3PlayerDlg 메시지 처리기

BOOL CMP3PlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	m_staticTIMER.SetWindowText("0:00");
	m_SliderVol.SetRange(0, 100);
	m_SliderVol.SetPos(50);
	m_SliderVol.SetLineSize(10);
	m_SliderVol.SetPageSize(20);
	CString strPos;
	int nPos = m_SliderVol.GetPos();
	strPos.Format("%d", nPos);
	m_Vol = 0;
	m_EditVol.SetWindowText(strPos);
	InitDlg();
	m_hWav = NULL;
	m_Playing = FALSE;
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CMP3PlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CMP3PlayerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CMP3PlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//=========================================================
//
// CMP3PlaterDlg Class
//
//=========================================================

void CMP3PlayerDlg::OnBnClickedButtonUpload()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if(!AfxBeginThread(ThreadDecode, this))
	{
		MessageBox("UpLoad 실패");
	}
}

void CMP3PlayerDlg::OnBnClickedButtonRmove()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	DeleteList();
}


void CMP3PlayerDlg::OnBnClickedCheckLoop()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

}


void CMP3PlayerDlg::OnBnClickedButtonPlay()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Play();
}

void CMP3PlayerDlg::OnBnClickedButtonPause()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//Pause();
	PauseWAVEFile(m_hWav);
}

void CMP3PlayerDlg::OnBnClickedButtonStop()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//Stop();
	StopWAVEFile(m_hWav);
}

void CMP3PlayerDlg::OnBnClickedButtonNext()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	StopWAVEFile(m_hWav);
	Next();
}

void CMP3PlayerDlg::OnBnClickedButtonPrev()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	StopWAVEFile(m_hWav);
	Prev();
}

//=========================================================
//
// CMP3PlaterDlg Method
//
//=========================================================

void CMP3PlayerDlg::InitDlg()
{
	CFileFind finder;
	//m_vec.clear();
	BOOL bChkDir = finder.FindFile(_T("C:\\MP3"));

	if(!bChkDir)
	{
		CreateDirectory(_T("C:\\MP3"), NULL);
	}

	BOOL bWorking = finder.FindFile(_T("C:\\MP3\\*.*"));


	while(bWorking)
	{
		bWorking = finder.FindNextFile();

		if(!finder.IsDots())
		{
			AddList(finder.GetFileName());
		}
	}
	StopWAVEFile(m_hWav);
	SetTimer(2, 300, NULL);
}

void CMP3PlayerDlg::AddList(CString strFileName)
{
	std::list<CString>::iterator iter;

	for(iter = m_list.begin(); iter != m_list.end();)
	{
		if(strFileName == *iter)
		{
			return;
		}
		else
		{
			iter++;
		}
	}
	m_list.push_back(strFileName);
	ShowList();
}

void CMP3PlayerDlg::DeleteList()
{
	CFileFind finder;
	CString strTemp;
	CString strRemoveFile;
	//POSITION pos;

	int nSelCount = LB_ERR;

	nSelCount = m_ListBox.GetSelCount();
	int *nItems = new int[nSelCount];
	m_ListBox.GetSelItems(nSelCount, nItems);

	
	if(nSelCount == 0)
	{
		MessageBox("하나 이상 선택하세요.");
	}
	for(int i=nSelCount-1; i>=0; i--)
	{
		m_ListBox.GetText(nItems[i], strTemp);

		strRemoveFile.Format(_T("C:\\MP3\\%s"), strTemp);

		BOOL bRet = finder.FindFile(strRemoveFile);

		if(bRet)
		{
			if(DeleteFile(strRemoveFile))
			{
				std::list<CString>::iterator iter;
				for(iter = m_list.begin(); iter != m_list.end();)
				{
					if(strTemp == *iter)
					{
						iter = m_list.erase(iter);
						ShowList();
					}
					else
					{
						iter++;
					}
				}
			}
		}
	}
}

void CMP3PlayerDlg::UpLoad()
{
	static TCHAR BASED_CODE szFilter[] = _T("모든파일(*.*)|*.*||");
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, szFilter);

	if(IDOK == dlg.DoModal())
	{
		CString strOrigin, strDest, strWavFile;
		POSITION pos = dlg.GetStartPosition();
		BOOL bResult;
		while(pos)
		{
			m_strFilePath = dlg.GetNextPathName(pos);
			m_strFileName =  m_strFilePath;
			m_strFileName.Delete(0, m_strFilePath.ReverseFind('\\')+1);

			strOrigin.Format(_T("%s"), m_strFilePath);
			CString strWav = m_strFileName;
			strWav.Replace(".mp3", ".wav");
			strDest.Format(_T("C:\\MP3\\%s"), strWav);
			strWavFile = strDest;
			strWavFile.Replace(".mp3", ".wav");
			if(strOrigin.Find(".mp3", strOrigin.GetLength()-3))
			{

				CLameHelper* lamehelper = new CLameHelper;
				//bResult = CopyFile(strOrigin, strDest, TRUE);
				tagHelper th((LPSTR)(LPCSTR)strOrigin);
				th.removeAllTags();
				lamehelper->Decode((LPSTR)(LPCSTR)strOrigin, (LPSTR)(LPCSTR)strWavFile);
				
			}
			else
			{
				bResult = CopyFile(strOrigin, strDest, TRUE);
			}

			
				AddList(strWav);
			
		}
	}
}


void CMP3PlayerDlg::ShowList()
{
	m_ListBox.ResetContent();
	std::list<CString>::iterator iter;

	for(iter = m_list.begin(); iter != m_list.end();)
	{
		m_ListBox.InsertString(-1, *iter);
		iter++; 
	}
}


void CMP3PlayerDlg::Play()
{

	CFileFind finder;
	CString strPlayerPath;
	CString strFileName;
	int nIndex = -1;

	nIndex = m_ListBox.GetCurSel();
	strPlayerPath.Format(WAV_FILE_PATH);

	if(nIndex == -1)
	{
		nIndex = 0;
	}

	m_ListBox.GetText(nIndex, strFileName);
	std::list<CString>::iterator iter;
	for(iter = m_list.begin(); iter != m_list.end();)
	{
		if(strFileName != *iter)
		{
			iter++; 
		}
		else
			break;
	}
	playWAVEFile(m_hWav, *iter);
}




void CMP3PlayerDlg::Next()
{
	std::list<CString>::iterator iter;
	for(iter = m_list.begin(); iter != m_list.end();)
	{
		if(*iter == m_nowWav)
			break;
		else
			iter++;
	}
	if(*iter == m_nowWav)
	{
		StopWAVEFile(m_hWav);
		if(*iter != m_list.back())
		{
			iter++;
			playWAVEFile(m_hWav, *iter);
			return;

		}
		else
		{
			iter = m_list.begin();
			playWAVEFile(m_hWav, *iter);
			return;
		}
	}
}

void CMP3PlayerDlg::Prev()
{
	std::list<CString>::iterator iter;
	for(iter = m_list.begin(); iter != m_list.end();)
	{
		if(*iter == m_nowWav)
			break;
		else
			iter++;
	}
	if(*iter == m_nowWav)
	{
		StopWAVEFile(m_hWav);
		if(*iter != m_list.front())
		{
			iter--;
			playWAVEFile(m_hWav ,*iter);
			return;

		}
		else
		{
			iter = m_list.end();
			iter--;
			playWAVEFile(m_hWav ,*iter);
			return;
		}
	}
}

void CMP3PlayerDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if(IDC_SLIDER_VOLUM == pScrollBar->GetDlgCtrlID())
	{
		int nPos = m_SliderVol.GetPos();
		CString strPos;
		strPos.Format("%d", nPos);
		m_EditVol.SetWindowText(strPos);
		waveOutSetVolume((HWAVEOUT)m_hWav, nPos*500);
	}

	if(IDC_SLIDER_TIMER == pScrollBar->GetDlgCtrlID())
	{
		m_SliderTimer.GetPos();
	}
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}





void CMP3PlayerDlg::OnEnChangeEditVolum()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString strPos;
	CString strFile;
	m_EditVol.GetWindowText(strPos);
	DWORD nPos = _ttoi(strPos);
	if(nPos > 100)
	{
		strPos = "100";
		nPos = 100;
		m_EditVol.SetWindowText("100");
	}
	m_SliderVol.SetPos(nPos);
}


DWORD CMP3PlayerDlg::playWAVEFile(HWND hWndNotify, CString lpszWAVEFileName)
{
	if(m_Playing)
	{
		return (0L);
	}
		m_Playing = TRUE;

		m_nowWav = lpszWAVEFileName;
		SetSel(TRUE);

		CString strWav;
		strWav.Format(WAV_FILE_PATH + lpszWAVEFileName);
		GetWavSize(strWav);
		int nLen = strWav.GetLength();
		LPSTR lpszBuf = strWav.GetBuffer(nLen);
		//strWav.Format("C:\\MP3\\19th Floor - Bobby Richards.wav");
		m_mciOpenParms.lpstrDeviceType = "waveaudio";
		m_mciOpenParms.lpstrElementName = lpszBuf;
		if (m_dwReturn = mciSendCommand(m_wDeviceID, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT ,(DWORD)(LPVOID) &m_mciOpenParms))
		{
			return (m_dwReturn);
		}
		m_wDeviceID = m_mciOpenParms.wDeviceID;
		m_totlaTime = m_WavSize / 176400;
		m_Min = m_totlaTime / 60;
		m_Sec = m_totlaTime % 60;
		CString str1, str2, str3;
	/*	str1.Format("time : %d", m_totlaTime);
		str2.Format("min : %d", m_Min);
		str3.Format("sec : %d", m_Sec);
		MessageBox(str1);
		MessageBox(str2);
		MessageBox(str3);*/
		m_SliderTimer.SetRange(0, m_totlaTime);
		m_SliderTimer.SetPos(0);
		m_nTimerPos = 0;
		m_mciPlayParms.dwCallback = (DWORD) hWndNotify;
		m_dwReturn = mciSendCommand(m_wDeviceID, MCI_PLAY , MCI_NOTIFY, (DWORD)(LPVOID) &m_mciPlayParms);
		CString strTimer;
		strTimer.Format("%d:%d",m_Min ,m_Sec);
		SetTimer(1, 1000, NULL);
		m_staticTIMER.SetWindowText(strTimer);
		m_btnPlay.EnableWindow(FALSE);		
    return (0L);
}

DWORD CMP3PlayerDlg::PauseWAVEFile(HWND hWndNotify)
{
	m_mciGenericParam.dwCallback = (DWORD) hWndNotify;
	if(m_bPause)
	{
		m_btnPause.SetWindowText("Resume");
		m_dwReturn = mciSendCommand(m_wDeviceID, MCI_PAUSE, MCI_NOTIFY | MCI_WAIT, (DWORD)(LPVOID) &m_mciGenericParam);
		m_bPause = FALSE;
	}
	else
	{
		m_btnPause.SetWindowText("Pause");
		m_dwReturn = mciSendCommand(m_wDeviceID, MCI_RESUME, MCI_NOTIFY, (DWORD)(LPVOID) &m_mciGenericParam);
		m_bPause = TRUE;
	}
    return (0L);
}

DWORD CMP3PlayerDlg::StopWAVEFile(HWND hWndNotify)
{
	SetSel(FALSE);
	m_staticTIMER.SetWindowText("0:00");
	m_mciGenericParam.dwCallback = (DWORD) hWndNotify;
	m_Playing = FALSE;
	m_btnPlay.EnableWindow(TRUE);
	m_dwReturn = mciSendCommand(m_wDeviceID, MCI_CLOSE, MCI_NOTIFY, (DWORD)(LPVOID) &m_mciGenericParam);
	m_bPause = TRUE;
	KillTimer(1);
	m_SliderTimer.SetPos(0);
    return (0L);
}

LONG CMP3PlayerDlg::GetWavSize(CString strFileName)
{
	CString strFile;
	FILE* wavesize = fopen(strFileName, "rb");
	fseek(wavesize, 0, SEEK_END);
	m_WavSize = ftell(wavesize); 
	fseek(wavesize, 0, SEEK_SET);
	fclose(wavesize);
	return 0L;
}



void CMP3PlayerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if(!m_bPause)
	{
		return;
	}
	CString strTimer;
	strTimer.Format("%d:%d",m_Min ,m_Sec);
	switch(nIDEvent)
	{
	case 1 :
		m_nTimerPos += 1;
		m_SliderTimer.SetPos(m_nTimerPos);
		m_Sec--;
		m_staticTIMER.SetWindowText(strTimer);
		//MessageBox(strTimer);
		if(m_Sec < 0)
		{
			m_Sec = 59;
			m_Min--;
		}
		break;

	case 2:
		if(m_CheckLoop.GetCheck())
		{
			m_CheckRandom.SetCheck(FALSE);
			m_CheckRandom.EnableWindow(FALSE);
		}
		else if(!m_CheckLoop.GetCheck())
		{
			m_CheckRandom.EnableWindow(TRUE);
		}
		if(m_CheckRandom.GetCheck())
		{
			m_CheckLoop.SetCheck(FALSE);
			m_CheckLoop.EnableWindow(FALSE);
		}
		else if(!m_CheckRandom.GetCheck())
		{
			m_CheckLoop.EnableWindow(TRUE);
		}
		break;
	}
	if(m_Min == 0 && m_Sec == 0)
	{
		::Sleep(1000);
		if(m_CheckLoop.GetCheck())
		{
			m_CheckRandom.SetCheck(FALSE);
			m_CheckRandom.EnableWindow(FALSE);
			KillTimer(1);
			StopWAVEFile(m_hWav);
			playWAVEFile(m_hWav, m_nowWav);
			return;
		}

		else if(m_CheckRandom.GetCheck())
		{
			m_CheckLoop.SetCheck(FALSE);
			m_CheckLoop.EnableWindow(FALSE);
			int nCount = m_list.size();
			srand(time(0));
			int nrand = rand() % (nCount - 1);
			std::list<CString>::iterator iter;
			iter = m_list.begin();
			for(int i = 0; i< nrand; i++)
			{
				if(iter != m_list.end())
				{
					iter++;
				}
				else if(*iter == m_list.back())
				{
					iter = m_list.begin();
				}
			}
			KillTimer(1);
			StopWAVEFile(m_hWav);
			playWAVEFile(m_hWav, *iter);
		}
		else
		{
			KillTimer(1);
			Next();
			return;
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CMP3PlayerDlg::SetVolume(int nVol)
{
	//m_Vol = 0;
	//nVol *= 10;

	m_mciSetAudioParam.dwItem     = MCI_DGV_SETAUDIO_VOLUME;
	m_mciSetAudioParam.dwValue    = nVol;
	m_mciSetAudioParam.dwCallback = 0;
	m_mciSetAudioParam.dwOver     = 0;
	m_mciSetAudioParam.lpstrAlgorithm = NULL;
	m_mciSetAudioParam.lpstrQuality   = NULL;

	DWORD dwReturn = mciSendCommandW(m_wDeviceID, MCI_SET_AUDIO, MCI_DGV_SETAUDIO_VALUE | MCI_DGV_SETAUDIO_ITEM, (DWORD)(LPVOID)&m_mciSetAudioParam );
	
	if(dwReturn == 0)
	{
		MessageBox("TRUE");
	}
	else
	{
		CString str;
		str.Format("%d", dwReturn);
		MessageBox(str);
	}
}

void CMP3PlayerDlg::SetSel(BOOL bStatus)
{
	CString strList;
	int nCount = m_ListBox.GetCount();
	BOOL b;
	b=FALSE;
	for(int i=0; i<nCount; i++)
	{
		m_ListBox.GetText(i, strList);
		if(strList == m_nowWav)
		{
			m_ListBox.SetSel(i, bStatus);
			b = TRUE;
		}
		if(b)
			break;

	}
}

UINT ThreadDecode(LPVOID pParam)
{
	CMP3PlayerDlg* pDlg = (CMP3PlayerDlg*)pParam;

	pDlg->UpLoad();

	return 0;
}


void CMP3PlayerDlg::OnLbnDblclkListPlayer()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if(m_Playing)
	{
		StopWAVEFile(m_hWnd);
	}
	Play();

}
