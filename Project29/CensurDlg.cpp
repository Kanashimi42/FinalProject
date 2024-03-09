#include "CensurDlg.h"
#include <windows.h>
#include <string>
#include <fstream>
#include <Commctrl.h>
#define MAX_LOADSTRING 100

CCensurDlg* CCensurDlg::ptr = NULL;
bool CCensurDlg::stopScan = false;

CCensurDlg::CCensurDlg(void)
{
	ptr = this;
}

CCensurDlg::~CCensurDlg(void)
{

}

BOOL CCensurDlg::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return ptr->Cls_OnInitDialog(hWnd, (HWND)wParam, lParam);
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case IDC_BUTTON4: 
			ptr->StartSearch(hWnd);
			break;
		case IDC_BUTTON7:
			ptr->StartSearch(hWnd);
			break;
		}
	}
	break;
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}


void CCensurDlg::Cls_OnClose(HWND hwnd)
{
	EndDialog(hwnd, 0);
}

BOOL CCensurDlg::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	LoadFilesList("Files", hwnd);
	LoadCensorWordsList("Censur list", hwnd);
	HWND hProgressBar = GetDlgItem(hwnd, IDC_PROGRESS1);
	SendMessage(hProgressBar, PBM_SETRANGE32, 0, MAKELPARAM(0, maxProgressValue));
	SendMessage(hProgressBar, PBM_SETSTEP, 1, 0);

	return 0;
}

void CCensurDlg::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
	case IDC_BUTTON4: 
		StartSearch(hwnd);
		break;
	case IDC_BUTTON7:
		StopSearch();
		break;
	}
}

void CCensurDlg::StartSearch(HWND hWnd)
{
	string filesDir = "Files"; 
	string censorDir = "Censor list"; 

	SendMessage(GetDlgItem(hWnd, IDC_LIST1), LB_RESETCONTENT, 0, 0);
	SendMessage(GetDlgItem(hWnd, IDC_LIST2), LB_RESETCONTENT, 0, 0);
	LoadFilesList(filesDir, hWnd);
	LoadCensorWordsList(censorDir, hWnd);
	ScanFilesAndReplace(filesDir, censorDir);
}
void CCensurDlg::LoadFilesList(const string& filesDir, HWND hWnd)
{
	WIN32_FIND_DATAW FindFileData;
	HANDLE hFind;

	wstring searchPath = wstring(filesDir.begin(), filesDir.end()) + L"\\*.*";

	hFind = FindFirstFileW(searchPath.c_str(), &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				SendMessageW(GetDlgItem(hWnd, IDC_LIST1), LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(FindFileData.cFileName));
			}
		} while (FindNextFileW(hFind, &FindFileData));
		FindClose(hFind);
	}
}

void CCensurDlg::LoadCensorWordsList(const string& censorDir, HWND hWnd)
{
	string censorFile = censorDir + "\\Censur.txt";
	ifstream file(censorFile);
	string word;

	if (file.is_open())
	{
		while (file >> word)
		{
			SendMessageA(GetDlgItem(hWnd, IDC_LIST2), LB_ADDSTRING, 0, (LPARAM)word.c_str());
		}
		file.close();
	}
}

void CCensurDlg::StopSearch()
{
	stopScan = true;
}


void CCensurDlg::ScanFilesAndReplace(const string& filesDir, const string& censorDir)
{
	stopScan = false;

	WIN32_FIND_DATAW FindFileData;
	HANDLE hFind;

	wstring searchPath(filesDir.begin(), filesDir.end());
	searchPath += L"\\*.*";

	hFind = FindFirstFileW(searchPath.c_str(), &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				int requiredSize = WideCharToMultiByte(CP_UTF8, 0, FindFileData.cFileName, -1, NULL, 0, NULL, NULL);
				if (requiredSize > 0)
				{
					string filePath(requiredSize - 1, 0);
					WideCharToMultiByte(CP_UTF8, 0, FindFileData.cFileName, -1, &filePath[0], requiredSize, NULL, NULL);
					filePath = filesDir + "\\" + filePath;
					if (!stopScan)
						ScanFileForCensorWords(filePath, censorDir);
				}
			}
		} while (FindNextFileW(hFind, &FindFileData));
		FindClose(hFind);
	}
}


void CCensurDlg::ScanFileForCensorWords(const string& filePath, const string& censorDir, HWND hwnd)
{
	ifstream file(filePath);
	string line;

	if (file.is_open())
	{
		while (!stopScan && getline(file, line))
		{
			maxProgressValue = ;
			currentProgressValue = 0;
			SendMessage(GetDlgItem(hWnd, IDC_PROGRESS1), PBM_SETPOS, 0, 0); 

			ifstream censorFile(censorDir);
			string censorWord;

			if (censorFile.is_open())
			{
				while (censorFile >> censorWord)
				{
					size_t pos = line.find(censorWord);
					while (pos != string::npos)
					{
						line.replace(pos, censorWord.length(), "***");
						pos = line.find(censorWord, pos + 3); 
						currentProgressValue++;
						SendMessage(hProgressBar, PBM_SETPOS, currentProgressValue, 0);

					}
				}
				censorFile.close();
			}
			else
			{
			
			}
			
		}
		file.close();
	}
	else
	{
		
	}
}
