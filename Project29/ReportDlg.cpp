#include "ReportDlg.h"

INT_PTR CALLBACK ReportDlg::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		HWND hList = GetDlgItem(hWnd, IDC_LIST4);
		if (hList)
		{
			ifstream reportFile("report.txt");
			if (reportFile.is_open())
			{
				string line;
				while (getline(reportFile, line))
				{
					SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)line.c_str());
				}
				reportFile.close();
			}
			else
			{
				MessageBox(hWnd, L"Не удалось открыть файл отчета", L"Ошибка", MB_OK | MB_ICONERROR);
				EndDialog(hWnd, 0);
			}
		}
		return TRUE;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hWnd, 0);
			return TRUE;
		}
		break;
	}

	return FALSE;
}
