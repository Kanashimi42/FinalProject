#include "CensurDlg.h"
#include <windows.h>
#include <string>
#include <fstream>
#include <thread>
#include "EditDlg.h"
#include "ReportDlg.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
HWND hEditDlg = NULL;
#undef MAX_PATH
#define MAX_PATH 512
#include <Commctrl.h>
#define MAX_LOADSTRING 100
CCensurDlg* CCensurDlg::ptr = NULL;
bool CCensurDlg::stopScan = false;
std::vector<std::pair<std::string, int>> wordCount;

std::string FILES_DIRECTORY = "C:\\Users\\Kanashimi\\source\\repos\\Project29\\Project29\\Files\\";

wchar_t* newWordSuper;
std::string CENSOR_FILE = "C:\\Users\\Kanashimi\\source\\repos\\Project29\\Project29\\Censor list\\Censor.txt";


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
		case IDC_BUTTON2:
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG3), hWnd, (DLGPROC)EditDlg::DlgProc);
			hEditDlg = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG3), hWnd, (DLGPROC)EditDlg::DlgProc);
			ptr->EditSelectedCensorWord(hWnd, hEditDlg);
			return TRUE;
			break;
		case IDC_BUTTON3:
			ptr->DeleteSelectedCensorWord(hWnd);
			break;
		case IDC_BUTTON4:
			ptr->StartSearch(hWnd);
			break;
		case IDC_BUTTON5:
			ptr->ChangeSearchDirectory(hWnd);
			break;
		case IDC_BUTTON6:
			ptr->ChangeCensorFile(hWnd);
			break;
		case IDC_BUTTON7:
			ptr->StopSearch();
			break;
		case IDC_BUTTON8:
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG2), hWnd, (DLGPROC)ReportDlg::DlgProc);
			hEditDlg = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG2), hWnd, (DLGPROC)ReportDlg::DlgProc);
			ptr->WriteReportToFile();
			return TRUE;
			break;
		case IDC_BUTTON9:
			int result = MessageBox(hWnd, L"Вы точно хотите сканировать и изменить все файлы?", L"Подтверждение", MB_YESNO | MB_ICONQUESTION);
			if (result == IDYES) {
				ptr->ScanAllFilesAndReplace(hWnd);
			}
			break;
		}
	}
	break;

	case WM_APP: // Добавляем обработку сообщения WM_APP
	{
		LPCTSTR newWord = reinterpret_cast<LPCTSTR>(wParam);
		// Ваш код для обработки нового слова
		// Например, вы можете его отобразить в вашем диалоговом окне
		// Например, если у вас есть элемент управления для отображения текста:
		MessageBox(hWnd, newWord, L"Ошибка", MB_OK | MB_ICONERROR);
		newWordSuper = _tcsdup(newWord);
		return TRUE;
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
	LoadFilesList(FILES_DIRECTORY, hwnd);
	LoadCensorWordsList(CENSOR_FILE, hwnd);
	onProgressUpdate = [=](int progress) {
		HWND hProgressBar = GetDlgItem(hwnd, IDC_PROGRESS1);
		if (hProgressBar)
		{
			SendMessage(hProgressBar, PBM_SETPOS, progress, 0);
		}
		};

	return TRUE;
}


void CCensurDlg::StartSearch(HWND hWnd)
{
	stopScan = false;

	string filesDir = FILES_DIRECTORY;
	string censorDir = CENSOR_FILE;

	HWND hListBox = GetDlgItem(hWnd, IDC_LIST1); // Получить дескриптор ListBox
	int index = SendMessage(hListBox, LB_GETCURSEL, 0, 0); // Получить индекс выбранного элемента
	if (index != LB_ERR) // Если элемент выбран
	{
		TCHAR buffer[MAX_PATH];
		SendMessage(hListBox, LB_GETTEXT, index, (LPARAM)buffer); // Получить текст выбранного элемента

		TCHAR filename[MAX_PATH];
		wcsncpy(filename, buffer, MAX_PATH); // Копировать текст в filename
		filename[MAX_PATH - 1] = L'\0'; // Убедиться, что строка завершается нулевым символом
		wstring wFilesDir(filesDir.begin(), filesDir.end());

		MessageBox(NULL, filename, L"Сообщение", MB_OK); // Обратите внимание на использование L"..." для строк Unicode
		wstring wFilePath = wFilesDir + L"//" + wstring(filename);
		string filePath(wFilePath.begin(), wFilePath.end());

		ScanFilesAndReplace(filePath, censorDir, hWnd); // Запустить сканирование и замену для выбранного файла
	}
	else
	{
		MessageBox(NULL, L"Пожалуйста, выберите файл из списка.", L"Сообщение", MB_OK);
	}
}


void CCensurDlg::EditSelectedCensorWord(HWND hWnd, HWND hEditDlg)
{
	// Получить дескриптор списка IDC_LIST2
	HWND hListBox2 = GetDlgItem(hWnd, IDC_LIST2);

	// Получить индекс выбранного элемента в списке
	int selectedIndex = SendMessage(hListBox2, LB_GETCURSEL, 0, 0);
	if (selectedIndex != LB_ERR)
	{
		// Получить длину текста выбранного элемента
		int textLength = SendMessage(hListBox2, LB_GETTEXTLEN, selectedIndex, 0);

		// Выделить память для текста выбранного элемента
		wchar_t* buffer = new wchar_t[textLength + 1];

		// Получить текст выбранного элемента
		SendMessage(hListBox2, LB_GETTEXT, selectedIndex, (LPARAM)buffer);
		buffer[textLength] = L'\0'; // Добавить нулевой символ в конец строки

		// Передать выбранное слово в IDC_EDIT1 в окне EditDlg
		HWND hEdit1 = GetDlgItem(hEditDlg, IDC_EDIT1);
		SetWindowText(hEdit1, buffer);
		int newWordLength = GetWindowTextLength(hEdit1);
		wchar_t* newWord = new wchar_t[newWordLength + 1];
		GetWindowText(hEdit1, newWord, newWordLength + 1);

		// Получить путь к временному файлу
		std::string tempFilePath = CENSOR_FILE + ".tmp";

		// Открыть оригинальный файл с запрещенными словами для чтения
		std::wifstream inFile(CENSOR_FILE);
		if (!inFile.is_open())
		{
			MessageBox(hWnd, L"Ошибка открытия оригинального файла.", L"Ошибка", MB_OK | MB_ICONERROR);
			return;
		}

		// Открыть временный файл для записи
		std::wofstream outFile(tempFilePath);
		if (!outFile.is_open())
		{
			MessageBox(hWnd, L"Ошибка открытия временного файла.", L"Ошибка", MB_OK | MB_ICONERROR);
			inFile.close();
			return;
		}

		// Переменная для хранения текущего слова из файла
		std::wstring word;

		// Копировать все слова из оригинального файла в временный файл, заменяя выбранное слово на новое
		while (inFile >> word)
		{
			if (word == buffer)
			{
				word = newWordSuper;
			}
			outFile << word << std::endl;
		}

		// Закрыть файлы
		inFile.close();
		outFile.close();

		// Удалить оригинальный файл
		if (remove(CENSOR_FILE.c_str()) != 0)
		{
			MessageBox(hWnd, L"Ошибка удаления оригинального файла.", L"Ошибка", MB_OK | MB_ICONERROR);
			return;
		}

		// Переименовать временный файл в оригинальный
		if (rename(tempFilePath.c_str(), CENSOR_FILE.c_str()) != 0)
		{
			MessageBox(hWnd, L"Ошибка переименования временного файла.", L"Ошибка", MB_OK | MB_ICONERROR);
			return;
		}

		// Освободить память для текста выбранного элемента
		delete[] buffer;
		delete[] newWord;

		// Обновить список слов в листбоксе
		LoadCensorWordsList(CENSOR_FILE, hWnd);
	}
}


void CCensurDlg::DeleteSelectedCensorWord(HWND hWnd)
{
	// Получить дескриптор списка IDC_LIST2
	HWND hListBox2 = GetDlgItem(hWnd, IDC_LIST2);

	// Получить индекс выбранного элемента в списке
	int selectedIndex = SendMessage(hListBox2, LB_GETCURSEL, 0, 0);
	if (selectedIndex != LB_ERR)
	{
		// Получить длину текста выбранного элемента
		int textLength = SendMessage(hListBox2, LB_GETTEXTLEN, selectedIndex, 0);

		// Выделить память для текста выбранного элемента
		wchar_t* buffer = new wchar_t[textLength + 1];

		// Получить текст выбранного элемента
		SendMessage(hListBox2, LB_GETTEXT, selectedIndex, (LPARAM)buffer);
		buffer[textLength] = '\0'; // Добавить нулевой символ в конец строки
		MessageBoxW(hWnd, buffer, L"Выбранный элемент", MB_OK | MB_ICONINFORMATION);

		// Получить путь к временному файлу
		std::string tempFilePath = CENSOR_FILE + ".tmp";

		// Открыть оригинальный файл с запрещенными словами для чтения
		std::wifstream inFile(CENSOR_FILE);

		// Открыть временный файл для записи
		std::wofstream outFile(tempFilePath);

		// Переменная для хранения текущего слова из файла
		std::wstring word;

		// Копировать все слова из оригинального файла в временный файл, кроме выбранного
		while (inFile >> word)
		{
			// Если текущее слово не совпадает с выбранным, записать его во временный файл
			if (word != buffer)
			{
				outFile << word << std::endl;
			}
		}

		// Закрыть файлы
		inFile.close();
		outFile.close();

		// Удалить оригинальный файл
		if (remove(CENSOR_FILE.c_str()) != 0)
		{
			MessageBox(hWnd, L"Ошибка удаления оригинального файла.", L"Ошибка", MB_OK | MB_ICONERROR);
			return;
		}

		// Переименовать временный файл в оригинальный
		if (rename(tempFilePath.c_str(), CENSOR_FILE.c_str()) != 0)
		{
			MessageBox(hWnd, L"Ошибка переименования временного файла.", L"Ошибка", MB_OK | MB_ICONERROR);
			return;
		}

		// Освободить память для текста выбранного элемента
		delete[] buffer;

		// Обновить список слов в листбоксе
		LoadCensorWordsList(CENSOR_FILE, hWnd);
	}
}


void CCensurDlg::WriteReportToFile()
{
	// Открываем файл "report.txt" для записи
	std::ofstream reportFile("report.txt");
	if (!reportFile.is_open()) {
		MessageBox(NULL, L"Не удалось создать файл отчета", L"Ошибка", MB_OK);
		return;
	}

	// Проходим по вектору wordCount и записываем каждую пару в файл
	for (const auto& pair : wordCount) {
		reportFile << pair.first << " (" << pair.second << ")" << std::endl;
	}

	// Закрываем файл
	reportFile.close();

	MessageBox(NULL, L"Информация успешно записана в файл отчета", L"Успех", MB_OK);
}
void CCensurDlg::ScanAllFilesAndReplace(HWND hWnd)
{
	string filesDir = FILES_DIRECTORY;
	string censorDir = CENSOR_FILE;

	// Получить список всех файлов в папке
	vector<string> fileList;
	FindFilesInDirectory(filesDir, fileList);

	// Очистить счетчик слов перед началом сканирования всех файлов
	wordCount.clear();

	// Пройти по всем файлам и выполнить сканирование и изменение
	for (const auto& file : fileList) {
		// Сканировать и заменять в файле, а также обновить счетчик слов
		ScanFilesAndReplace(file, censorDir, hWnd);
	}
}


void CCensurDlg::ChangeCensorFile(HWND hWnd)
{
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH] = { 0 };

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = _T("Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0");
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileName(&ofn) == TRUE)
	{

		// Очистить список IDC_LIST2
		HWND hListBox2 = GetDlgItem(hWnd, IDC_LIST2);
		if (hListBox2)
		{
			SendMessage(hListBox2, LB_RESETCONTENT, 0, 0);
		}
		// Преобразование TCHAR в std::wstring
		std::wstring wFilePath(szFile);

		// Преобразование std::wstring в std::string
		std::string filePath(wFilePath.begin(), wFilePath.end());



		// Обновить переменную CENSOR_FILE
		CENSOR_FILE = filePath;

		// Загрузить слова из нового файла
		LoadCensorWordsList(CENSOR_FILE, hWnd);
	}
}

void CCensurDlg::FindFilesInDirectory(const string& directory, vector<string>& fileList)
{
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;
	string searchPath = directory + "\\*.*";
	hFind = FindFirstFileA(searchPath.c_str(), &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				fileList.push_back(directory + "\\" + FindFileData.cFileName);
			}
		} while (FindNextFileA(hFind, &FindFileData));
		FindClose(hFind);
	}
}

void CCensurDlg::ChangeSearchDirectory(HWND hWnd)
{
	BROWSEINFO browseInfo = { 0 };
	browseInfo.lpszTitle = L"Выберите папку для поиска файлов";
	LPITEMIDLIST pidl = SHBrowseForFolder(&browseInfo);
	wchar_t selectedDir[MAX_PATH]; // Изменение типа на wchar_t
	if (pidl != NULL) {
		SHGetPathFromIDList(pidl, selectedDir);

		// Преобразование wchar_t в std::wstring
		std::wstring wSelectedDir(selectedDir);

		// Преобразование std::wstring в std::string
		std::string selectedDirStr(wSelectedDir.begin(), wSelectedDir.end());
		// Очистить список IDC_List1
		HWND hListBox = GetDlgItem(hWnd, IDC_LIST1);
		if (hListBox)
		{
			SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
		}
		// Обновить список файлов в ListBox и очистить прогресс-бар
		LoadFilesList(selectedDirStr, hWnd);
		HWND hProgressBar = GetDlgItem(hWnd, IDC_PROGRESS1);
		if (hProgressBar)
		{
			SendMessage(hProgressBar, PBM_SETPOS, 0, 0);
		}



		// Обновить значение переменной FILES_DIRECTORY
		FILES_DIRECTORY = selectedDirStr;
	}
}



void CCensurDlg::LoadCensorWordsList(const string& censorDir, HWND hWnd)
{
	string censorFile = CENSOR_FILE;
	ifstream file(censorFile);
	string word;

	// Получить дескриптор списка IDC_LIST2
	HWND hListBox2 = GetDlgItem(hWnd, IDC_LIST2);

	// Очистить список
	SendMessage(hListBox2, LB_RESETCONTENT, 0, 0);

	if (file.is_open())
	{
		while (file >> word)
		{
			SendMessageA(hListBox2, LB_ADDSTRING, 0, (LPARAM)word.c_str());
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
					// При добавлении слова в список слов также добавляем его в счетчик
					wordCount.push_back({ censorWord, 0 });
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
						// Увеличить счетчик для слова
						for (auto& pair : wordCount) {
							if (pair.first == censorWord) {
								pair.second++;
								break;
							}
						}
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
