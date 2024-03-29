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
    function<void(int)> onProgressUpdate;

    void ChangeCensorFile(HWND hWnd);
    void EditSelectedCensorWord(HWND hWnd, HWND hEditDlg);
    void WriteReportToFile();
    void UpdateCensorFile(const wchar_t* newWord);
    void Cls_OnClose(HWND hwnd);
    BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    void LoadCensorWordsList(const string& censorDir, HWND hWnd);
    void LoadFilesList(const string& filesDir, HWND hWnd);
    void ScanAllFilesAndReplace(HWND hWnd);
    void FindFilesInDirectory(const string& directory, vector<string>& fileList);
    void ChangeSearchDirectory(HWND hWnd);
    void DeleteSelectedCensorWord(HWND hWnd);
    void StartSearch(HWND hWnd);
    void StopSearch();
    void ScanFilesAndReplace(const string& filePath, const string& censorDir, HWND hwnd);
    void ScanFileForCensorWords(const string& filePath, const string& censorDir, HWND hwnd);
};
