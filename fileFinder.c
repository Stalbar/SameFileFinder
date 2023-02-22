#include <windows.h>
#include <strsafe.h>
#include <string.h>
#include "sha256.h"
#include "hashTable.h"

#define MAINWINDOWXPOS 300
#define MAINWINDOWYPOS 300
#define MAINWINDOWHEIGHT 170
#define MAINWINDOWWIDTH 230

#define ID_DIRECTORYINPUTEDIT 1
#define ID_STARTFINDERBUTTON 2
#define ID_RECURSIVESCANNINGCHECKBOX 3
#define ID_WRITEALLHASHESCHECKBOX 4

#define DIRECTORYINPUTEDITXPOS 10
#define DIRECTORYINPUTEDITYPOS 30
#define DIRECTORYINPUTEDITHEIGHT 25
#define DIRECTORYINPUTEDITWIDTH 200

#define DIRECTORYINPUTLABELXPOS 10
#define DIRECTORYINPUTLABELYPOS 10
#define DIRECTORYINPUTLABELHEIGHT 20
#define DIRECTORYINPUTLABELWIDTH 200

#define STARTFINDERBUTTONXPOS 10
#define STARTFINDERBUTTONYPOS 100
#define STARTFINDERBUTTONHEIGHT 30
#define STARTFINDERBUTTONWIDTH 200

#define RECURSIVESCANNINGCHECKBOXXPOS 10
#define RECURSIVESCANNINGCHECKBOXYPOS 55
#define RECURSIVESCANNINGCHECKBOXHEIGHT 25
#define RECURSIVESCANNINGCHECKBOXWIDTH 200

#define WRITEALLHASHESCHECKBOXXPOS 10
#define WRITEALLHASHESCHECKBOXYPOS 75
#define WRITEALLHASHESCHECKBOXHEIGHT 25
#define WRITEALLHASHESCHECKBOXWIDTH 200

#define MASK L"*"
#define MAX_PATH_LENGTH 260

HINSTANCE hInst;
HWND hDirectoryInputEdit, hStartFinderButton;

LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
BOOL DirectoryExists(LPWSTR path);
void TraverseDirectory(LPWSTR parentDirectory, BOOL isRecursive);
void AppendText(HWND hWnd, LPWSTR string);
void switchCheckBoxState(HWND hWnd, UINT checkBoxId);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	hInst = hInstance;
	WNDCLASSEX wcex;
	HWND hWnd;
	MSG msg;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 0);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "FileFinderClass";
	wcex.hIconSm = wcex.hIcon;
	
	RegisterClassEx(&wcex);
	
	hWnd = CreateWindow("FileFinderClass", "Duplicate Files Finder", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME, MAINWINDOWXPOS, MAINWINDOWYPOS, MAINWINDOWWIDTH, MAINWINDOWHEIGHT, NULL, NULL, hInstance, NULL);
	
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	wchar_t directoryPath[MAX_PATH];
	SYSTEMTIME st;
	switch(Message)
	{
		case WM_CREATE:
			CreateWindow("static", "Input directory", WS_CHILD | WS_VISIBLE | WS_TABSTOP, DIRECTORYINPUTLABELXPOS, DIRECTORYINPUTLABELYPOS, DIRECTORYINPUTLABELWIDTH, DIRECTORYINPUTLABELHEIGHT, hWnd, NULL, hInst, NULL);
			hDirectoryInputEdit = CreateWindow("edit", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL, DIRECTORYINPUTEDITXPOS, DIRECTORYINPUTEDITYPOS, DIRECTORYINPUTEDITWIDTH,  DIRECTORYINPUTEDITHEIGHT, hWnd, (HMENU) ID_DIRECTORYINPUTEDIT, hInst, NULL);
			hStartFinderButton = CreateWindow("button", "Start", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, STARTFINDERBUTTONXPOS, STARTFINDERBUTTONYPOS, STARTFINDERBUTTONWIDTH, STARTFINDERBUTTONHEIGHT, hWnd, (HMENU) ID_STARTFINDERBUTTON, hInst, NULL);
			CreateWindow("button", "Recursive scanning", WS_VISIBLE | WS_CHILD | BS_CHECKBOX, RECURSIVESCANNINGCHECKBOXXPOS, RECURSIVESCANNINGCHECKBOXYPOS, RECURSIVESCANNINGCHECKBOXWIDTH, RECURSIVESCANNINGCHECKBOXHEIGHT, hWnd, (HMENU) ID_RECURSIVESCANNINGCHECKBOX, hInst, NULL);
			CreateWindow("button", "Write all hashes", WS_VISIBLE | WS_CHILD | BS_CHECKBOX, WRITEALLHASHESCHECKBOXXPOS, WRITEALLHASHESCHECKBOXYPOS, WRITEALLHASHESCHECKBOXWIDTH, WRITEALLHASHESCHECKBOXHEIGHT, hWnd, (HMENU) ID_WRITEALLHASHESCHECKBOX, hInst, NULL);
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case ID_STARTFINDERBUTTON:
					GetLocalTime(&st);
					initializeHashTable();
					GetWindowTextW(hDirectoryInputEdit, directoryPath, MAX_PATH);
					if (!DirectoryExists(directoryPath))
					{
						MessageBoxW(NULL, L"Directory not exists", L"Error", MB_OK);
						break;
					}
					TraverseDirectory(directoryPath, IsDlgButtonChecked(hWnd, ID_RECURSIVESCANNINGCHECKBOX));
					saveResultsToFile(st, IsDlgButtonChecked(hWnd, ID_WRITEALLHASHESCHECKBOX));
					freeHashTableMemory();
					ShellExecuteW(0, 0, L"D:\\User Files\\University\\Course Work\\resultFile.txt", 0, 0, SW_SHOW);
					break;
				case ID_RECURSIVESCANNINGCHECKBOX:
					switchCheckBoxState(hWnd, ID_RECURSIVESCANNINGCHECKBOX);
					break;
				case ID_WRITEALLHASHESCHECKBOX:
					switchCheckBoxState(hWnd, ID_WRITEALLHASHESCHECKBOX);
					break;
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}
	return DefWindowProc(hWnd, Message, wParam, lParam);
}

BOOL DirectoryExists(LPWSTR path)
{
	DWORD dwAttrib = GetFileAttributesW(path);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void switchCheckBoxState(HWND hWnd, UINT checkBoxId)
{
	BOOL checked = IsDlgButtonChecked(hWnd, checkBoxId);
	UINT check = checked == TRUE ? BST_UNCHECKED : BST_CHECKED;
	CheckDlgButton(hWnd, checkBoxId, check);	
}

void TraverseDirectory(LPWSTR parentDirectory, BOOL isRecursive)
{
	WIN32_FIND_DATAW wfd = { 0 };
	SetCurrentDirectoryW(parentDirectory);
	HANDLE search = FindFirstFileW(MASK, &wfd);
	if (search == INVALID_HANDLE_VALUE)
	{
		return;
	}
	do
	{
		wchar_t tmpStr[MAX_PATH_LENGTH + 1];
		GetCurrentDirectoryW(MAX_PATH_LENGTH + 1, tmpStr);
		StringCchCatW(tmpStr, MAX_PATH_LENGTH, L"\\");
		StringCchCatW(tmpStr, MAX_PATH_LENGTH, wfd.cFileName);
		if (wcscmp(wfd.cFileName, L".") && wcscmp(wfd.cFileName, L".."))
		{
			if (((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) && (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)))
			{
				if (isRecursive)
				{
					TraverseDirectory(tmpStr, isRecursive);
					SetCurrentDirectoryW(parentDirectory);
				}
			}
			else
			{
				SHA256Digest digest = calculateHash(tmpStr);
				insert(digest.hashValues, digest.fileName);
				free(digest.fileName);
			}
		}			
	} while (FindNextFileW(search, &wfd));
	FindClose(search);
}