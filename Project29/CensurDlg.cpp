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
	onProgressUpdate = [=](int progress) {
		HWND hProgressBar = GetDlgItem(hwnd, IDC_PROGRESS1);
		if (hProgressBar)
		{
			SendMessage(hProgressBar, PBM_SETPOS, progress, 0);
		}
		};

	return TRUE;
}




void CCensurDlg::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
	case IDC_BUTTON4: // Кнопка "Start Search"
	{
		HWND hListBox = GetDlgItem(hwnd, IDC_LIST1); // Получить дескриптор ListBox
		int index = SendMessage(hListBox, LB_GETCURSEL, 0, 0); // Получить индекс выбранного элемента
		if (index != LB_ERR) // Если элемент выбран
		{
			char filename[MAX_PATH];
			SendMessage(hListBox, LB_GETTEXT, index, (LPARAM)filename); // Получить текст выбранного элемента
			string filePath = "Files\\" + string(filename); // Сформировать путь к файлу
			string censorDir = "C:\\Users\\Kanashimi\\source\\repos\\Project29\\Project29\\Censor list\\Censor.txt";

			ScanFilesAndReplace(filePath, censorDir, hwnd); // Запустить сканирование и замену для выбранного файла
		}
		else
		{
			MessageBox(NULL, L"Пожалуйста, выберите файл из списка.", L"Сообщение", MB_OK);
		}
		break;
	}
	case IDC_BUTTON7:
		StopSearch();
		break;
	}
}



void CCensurDlg::StartSearch(HWND hWnd)
{
	stopScan = false;

	string filesDir = "Files\\";
	string censorDir = "C:\\Users\\Kanashimi\\source\\repos\\Project29\\Project29\\Censor list\\Censor.txt";

	HWND hListBox = GetDlgItem(hWnd, IDC_LIST1); // Получить дескриптор ListBox
	int index = SendMessage(hListBox, LB_GETCURSEL, 0, 0); // Получить индекс выбранного элемента
	if (index != LB_ERR) // Если элемент выбран
	{
		char filename[MAX_PATH];
		SendMessage(hListBox, LB_GETTEXT, index, (LPARAM)filename); // Получить текст выбранного элемента
		string filePath = filesDir + "\\" + string(filename); // Сформировать путь к файлу

		ScanFilesAndReplace(filePath, censorDir, hWnd); // Запустить сканирование и замену для выбранного файла
	}
	else
	{
		MessageBox(NULL, L"Пожалуйста, выберите файл из списка.", L"Сообщение", MB_OK);
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

	thread scanThread([this, filePath, censorDir, hwnd] {
		HWND hProgressBar = GetDlgItem(hwnd, IDC_PROGRESS1);
		if (!hProgressBar)
		{
			return;
		}

		ifstream file(filePath);
		if (!file.is_open())
		{
			MessageBox(NULL, L"Не удалось открыть файл", L"Ошибка", MB_OK);
			return;
		}

		string line;
		ofstream outFile(filePath + ".tmp");

		ifstream censorFile(censorDir);
		if (!censorFile.is_open())
		{
			MessageBox(NULL, L"Не удалось открыть файл цензуры", L"Ошибка", MB_OK);
			return;
		}

		vector<string> censorWords;
		string censorWord;
		while (censorFile >> censorWord)
		{
			censorWords.push_back(censorWord);
		}
		censorFile.close();

		while (!stopScan && getline(file, line))
		{
			for (const auto& censorWord : censorWords)
			{
				size_t pos = line.find(censorWord);
				while (pos != string::npos)
				{
					line.replace(pos, censorWord.length(), "***");
					pos = line.find(censorWord, pos + 3);
				}
			}

			outFile << line << endl;

			for (int i = 0; i < 3; ++i)
			{
				if (stopScan)
				{
					break;
				}
				this_thread::sleep_for(chrono::milliseconds(333));
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
		HWND hwnd = FindWindow(NULL, L"Сообщение");
		if (hwnd != NULL)
		{
			SendMessage(hwnd, WM_CLOSE, 0, 0);
		}
		SendMessage(hProgressBar, PBM_SETPOS, 0, 0);
		MessageBox(NULL, L"Файл просканирован и изменен!", L"Сообщение", MB_OK);
		});
	scanThread.detach();
}


void CCensurDlg::ScanFileForCensorWords(const string& filePath, const string& censorDir, HWND hwnd)
{
	thread scanThread([this, filePath, censorDir, hwnd] {
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

			ifstream censorFile(censorDir);
			string censorWord;
			vector<string> censorWords;

			if (censorFile.is_open())
			{
				while (censorFile >> censorWord)
				{
					censorWords.push_back(censorWord);
				}
				censorFile.close();
			}

			int totalLines = count(istreambuf_iterator<char>(file), istreambuf_iterator<char>(), '\n');
			file.clear(); 
			file.seekg(0, ios::beg);

			int currentLine = 0;

			while (!stopScan && getline(file, line))
			{
				for (const auto& censorWord : censorWords)
				{
					size_t pos = line.find(censorWord);
					while (pos != string::npos)
					{
						line.replace(pos, censorWord.length(), "***");
						pos = line.find(censorWord, pos + 3);
					}
				}

				outFile << line << endl;

				++currentLine;
				int progress = (currentLine * 100) / totalLines;
				onProgressUpdate(progress);

				this_thread::sleep_for(chrono::milliseconds(100));
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
		else
		{
			MessageBox(NULL, L"Не удалось открыть файл", L"Ошибка", MB_OK);
		}
		
		});
	scanThread.detach();
}





