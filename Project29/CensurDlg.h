#pragma once
#include "header.h"

class CCensurDlg
{
	
public:
	CCensurDlg(void);
	~CCensurDlg(void);
	static bool stopScan;
	static BOOL CALLBACK DlgProc(HWND hWnd, UINT mes, WPARAM wp, LPARAM lp);
	static CCensurDlg* ptr;
	HWND hProgressBar;
	int maxProgressValue;
	int currentProgressValue;

	void Cls_OnClose(HWND hwnd);
	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void LoadCensorWordsList(const string& censorDir, HWND hWnd);
	void LoadFilesList(const string& filesDir, HWND hWnd);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void StartSearch(HWND hWnd);
	void StopSearch();
	void ScanFilesAndReplace(const string& filesDir, const string& censorDir, HWND hwnd);
	void ScanFileForCensorWords(const string& filePath, const string& censorDir, HWND hwnd);
};