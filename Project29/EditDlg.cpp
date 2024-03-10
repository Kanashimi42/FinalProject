#include "EditDlg.h"
HWND hEdit1 = NULL; int newWordLength = NULL; wchar_t* newWord = NULL; 
INT_PTR CALLBACK EditDlg::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		// Инициализация диалога...
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_SEND: // IDC_BUTTON_SEND - идентификатор вашей кнопки "Отправить"
			// Получить новое слово из IDC_EDIT1
			hEdit1 = GetDlgItem(hWnd, IDC_EDIT1);
			newWordLength = GetWindowTextLength(hEdit1);
			newWord = new wchar_t[newWordLength + 1];
			GetWindowText(hEdit1, newWord, newWordLength + 1);

			// Отправить сообщение в CensurDlg с новым словом
			SendMessage(GetParent(hWnd), WM_APP, (WPARAM)newWord, 0);

			MessageBox(hWnd, newWord, L"Ошибка", MB_OK | MB_ICONERROR);
			// Закрыть диалоговое окно
			EndDialog(hWnd, 0);
			return TRUE;
		case IDOK:

		case IDCANCEL:
			EndDialog(hWnd, 0);
			return TRUE;
		}
		break;
	}

	return FALSE;
}
