#include "ReportDlg.h"

INT_PTR CALLBACK ReportDlg::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		return TRUE;
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
