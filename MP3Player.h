
// MP3Player.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CMP3PlayerApp:
// �� Ŭ������ ������ ���ؼ��� MP3Player.cpp�� �����Ͻʽÿ�.
//

class CMP3PlayerApp : public CWinApp
{
public:
	CMP3PlayerApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CMP3PlayerApp theApp;