#include "CensurDlg.h"
#include <windows.h>
#include <string>
#include <fstream>
#include <thread>

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
			ptr->StopSearch();
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
	LoadCensorWordsList("Censor list", hwnd);

	HWND hProgressBar = GetDlgItem(hwnd, IDC_PROGRESS1);
	if (hProgressBar)
	{
		maxProgressValue = 100;
		currentProgressValue = 0;
		SendMessage(hProgressBar, PBM_SETRANGE32, 0, MAKELPARAM(0, maxProgressValue));
		SendMessage(hProgressBar, PBM_SETSTEP, 1, 0);
	}

	return TRUE;
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
	stopScan = false;

	string filesDir = "Files";
	string censorDir = "Censor list";

	SendMessage(GetDlgItem(hWnd, IDC_LIST1), LB_RESETCONTENT, 0, 0);
	SendMessage(GetDlgItem(hWnd, IDC_LIST2), LB_RESETCONTENT, 0, 0);
	LoadCensorWordsList(censorDir, hWnd);
	InvalidateRect(GetDlgItem(hWnd, IDC_LIST2), NULL, TRUE);
	UpdateWindow(GetDlgItem(hWnd, IDC_LIST2));
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
				string filename(FindFileData.cFileName, FindFileData.cFileName + wcslen(FindFileData.cFileName));
				ScanFilesAndReplace(filesDir + "\\" + filename, censorDir, hWnd);
			}
		} while (FindNextFileW(hFind, &FindFileData));
		FindClose(hFind);
	}
}


void CCensurDlg::LoadCensorWordsList(const string& censorDir, HWND hWnd)
{
	string censorFile = "C:\\Users\\Kanashimi\\source\\repos\\Project29\\Project29\\Censor list\\Censor.txt";
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

void CCensurDlg::LoadFilesList(const string& filesDir, HWND hWnd)
{
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;
	string searchPath = filesDir + "\\*.*";
	hFind = FindFirstFileA(searchPath.c_str(), &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				SendMessageA(GetDlgItem(hWnd, IDC_LIST1), LB_ADDSTRING, 0, (LPARAM)FindFileData.cFileName);
			}
		} while (FindNextFileA(hFind, &FindFileData));
		FindClose(hFind);
	}
}


void CCensurDlg::StopSearch()
{
	stopScan = true;
	MessageBox(NULL, L"Поиск остановлен", L"Сообщение", MB_OK);
}



void CCensurDlg::ScanFilesAndReplace(const string& filePath, const string& censorDir, HWND hwnd)
{
	HWND hProgressBar = GetDlgItem(hwnd, IDC_PROGRESS1);
	if (!hProgressBar)
	{
		return;
	}
	string censorDird = "C:\\Users\\Kanashimi\\source\\repos\\Project29\\Project29\\Censor list\\Censor.txt";
	ifstream file(filePath);
	if (!file.is_open())
	{
		MessageBox(NULL, L"Не удалось открыть файл", L"Ошибка", MB_OK);
		return;
	}

	string line;
	ofstream outFile(filePath + ".tmp");

	while (!stopScan && getline(file, line))
	{
		ifstream censorFile(censorDird);
		if (!censorFile.is_open())
		{
			MessageBox(NULL, L"Не удалось открыть файл цензуры", L"Ошибка", MB_OK);
			return;
		}

		string censorWord;
		while (censorFile >> censorWord)
		{
			size_t pos = line.find(censorWord);
			while (pos != string::npos)
			{
				line.replace(pos, censorWord.length(), "***");
				pos = line.find(censorWord, pos + 3);
			}
		}
		censorFile.close();

		outFile << line << endl;

		for (int i = 0; i < 3; ++i)
		{
			if (stopScan)
			{
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(333));
			SendMessage(hProgressBar, PBM_STEPIT, 0, 0);
		}
	}

	file.close();
	outFile.close();

	if (remove(filePath.c_str()) != 0)
	{
		MessageBox(NULL, L"Не удалось удалить исходный файл", L"Ошибка", MB_OK);
	}
	else if (rename((filePath + ".tmp").c_str(), filePath.c_str()) != 0)
	{
		MessageBox(NULL, L"Не удалось переименовать временный файл", L"Ошибка", MB_OK);
	}
}


void CCensurDlg::ScanFileForCensorWords(const string& filePath, const string& censorDir, HWND hwnd)
{
	HWND hProgressBar = GetDlgItem(hwnd, IDC_PROGRESS1);
	if (!hProgressBar)
	{
		return;
	}

	ifstream file(filePath);
	string line;

	if (file.is_open())
	{
		ofstream outFile(filePath + ".tmp");

		while (!stopScan && getline(file, line))
		{
			maxProgressValue = 100;
			currentProgressValue = 0;
			SendMessage(GetDlgItem(hwnd, IDC_PROGRESS1), PBM_SETPOS, 0, 0);

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

			outFile << line << endl;
		}

		file.close();
		outFile.close();

		if (remove(filePath.c_str()) != 0)
		{
		}
		else if (rename((filePath + ".tmp").c_str(), filePath.c_str()) != 0)
		{
		}
	}
	else
	{
		
	}
}


