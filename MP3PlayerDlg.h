
// MP3PlayerDlg.h : 헤더 파일
//

#pragma once
#ifndef _MP3_PLAYER_DLG_H_
#define _MP3_PLAYER_DLG_H_

#include "lame.h"
#include <list>
#include <fstream>
#include <iostream>
#include "afxwin.h"
#include "afxcmn.h"
#include <mmsystem.h>
#include <Vfw.h>
#include <digitalv.h>

// CMP3PlayerDlg 대화 상자
class CMP3PlayerDlg : public CDialogEx
{
	// 생성입니다.
public:
	CMP3PlayerDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

	// 대화 상자 데이터입니다.
	enum { IDD = IDD_MP3PLAYER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


	// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()


public:
	afx_msg void OnBnClickedButtonUpload();
	afx_msg void OnBnClickedButtonRmove();
	afx_msg void OnBnClickedCheckLoop();
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnBnClickedButtonPause();
	afx_msg void OnBnClickedButtonNext();
	afx_msg void OnBnClickedButtonPrev();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnEnChangeEditVolum();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	CString m_strFilePath;
	CString m_strFileName;
	CString m_nowWav;
	CString m_strCommand;
	std::list<CString> m_list;
	HWND m_hWav;
	BOOL m_bPause;
	BOOL m_Playing;
	MCIDEVICEID m_wDeviceID;
    DWORD m_dwReturn;
    MCI_DGV_OPEN_PARMSA m_mciOpenParms;
    MCI_DGV_PLAY_PARMS m_mciPlayParms;
	MCI_DGV_SET_PARMS  m_mciSetParams;
	MCI_GENERIC_PARMS m_mciGenericParam;
	MCI_DGV_STATUS_PARMS m_mciStatusParam;
	MCI_DGV_SETAUDIO_PARMSW m_mciSetAudioParam;

	int m_WavSize;
	int m_totlaTime;
	int m_Min;
	int m_Sec;
	int m_nTimerPos;
	DWORD m_Vol;

	CListBox m_ListBox;
	CButton m_btnPlay;
	CButton m_btnPause;
	CSliderCtrl m_SliderVol;
	CEdit m_EditVol;
	CSliderCtrl m_SliderTimer;
	CButton m_CheckLoop;
	CButton m_CheckRandom;

	void AddList(CString strFileName);
	void InitDlg();
	void ShowList();
	void DeleteList();
	void UpLoad();
	void SetVolume(int nVol);

	void Play();
	void Next();
	void Prev();
	void SetSel(BOOL bStatus);


	DWORD playWAVEFile(HWND hWndNotify, CString lpszWAVEFileName);
	DWORD PauseWAVEFile(HWND hWndNotify);
	DWORD StopWAVEFile(HWND hWndNotify);
	LONG GetWavSize(CString strFileName);

	CStatic m_staticTIMER;
	afx_msg void OnLbnDblclkListPlayer();
};

UINT ThreadDecode(LPVOID pParam);

#endif