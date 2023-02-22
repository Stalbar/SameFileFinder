#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <strsafe.h>
#include "FileNameList.h"

#define TABLE_SIZE 8957

struct HashTableItemStruct {
	uint32_t hashValues[8];
	FileNameListNode* files;
	struct HashTableItemStruct* next;
};

typedef struct HashTableItemStruct HashTableItem;

HashTableItem** hashTable;

uint32_t hashFunction(uint32_t hashValues[8])
{
	uint32_t sum = 0;
	for (int i = 0; i < 8; i++)
	{
		sum += hashValues[i];
	}
	return sum % TABLE_SIZE;
}

void initializeHashTable()
{
	hashTable = (HashTableItem**) malloc(sizeof(HashTableItem*) * TABLE_SIZE);
	for (uint32_t i = 0; i < TABLE_SIZE; i++)
		hashTable[i] = NULL;
}

void insert(uint32_t hashValues[8], LPWSTR fileName)
{
	HashTableItem* hashTablePointer = NULL;
	HashTableItem* lastItem = NULL;
	uint32_t index = hashFunction(hashValues);
	if (hashTable[index] == NULL)
	{
		hashTable[index] = (HashTableItem*) malloc (sizeof(struct HashTableItemStruct));
		for (int i = 0; i < 8; i++)
			(hashTable[index])->hashValues[i] = hashValues[i];
		(hashTable[index])->next = NULL;
		(hashTable[index])->files = NULL;
		(hashTable[index])->files = addFileNameListNode((hashTable[index])->files, fileName);
	}
	else
	{
		hashTablePointer = hashTable[index];
		while (hashTablePointer != NULL)
		{
			int isEqual = 1;
			for (int i = 0; i < 8; i++)
				if (hashTablePointer->hashValues[i]  != hashValues[i])
				{
					isEqual = 0;
					break;
				}
			if (isEqual)
			{
				hashTablePointer->files = addFileNameListNode(hashTablePointer->files, fileName);
				return;
			}
			lastItem = hashTablePointer;
			hashTablePointer = hashTablePointer->next;
		}
		if (hashTablePointer == NULL)
		{
			HashTableItem* item = (HashTableItem*) malloc (sizeof(struct HashTableItemStruct));
			for (int i = 0; i < 8; i++)
				item->hashValues[i] = hashValues[i];
			item->next = NULL;
			item->files = NULL;
			item->files = addFileNameListNode(item->files, fileName);
			lastItem->next = item;
		}
	}
}

void outputHashTable()
{
	for (int i = 0; i < TABLE_SIZE; i++)
	{
		HashTableItem* hashTablePointer = hashTable[i];
		while (hashTablePointer != NULL)
		{
			printf("Hash Value: ");
			for (int i = 0; i < 8; i++)
				printf("%x", hashTablePointer->hashValues[i]);
			printf("\n");
			FileNameListNode* fileListHead = hashTablePointer->files;
			printFileNameList(fileListHead);
			hashTablePointer = hashTablePointer->next;
		}
	}
}

void saveResultsToFile(SYSTEMTIME executionTime, BOOL isWriteAll)
{
	HANDLE hFile = CreateFileW(L"D:\\User Files\\University\\Course Work\\resultFile.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, "Impossible to save results", NULL, MB_OK);
		return;
	}
	int totalCountOfDuplicates = 0;
	int totalCountOfScanedFiles = 0;
	wchar_t fileHeader[2048];
	wchar_t date[128];
	StringCbPrintfW(date, 128, L"%02i.%02i.%0000i %02i:%02i:%02i\r\n", executionTime.wDay, executionTime.wMonth, executionTime.wYear, executionTime.wHour, executionTime.wMinute, executionTime.wSecond);
	StringCchCopyW(fileHeader, 2048, L"Start scanning time: ");
	StringCchCatW(fileHeader, 2048, date);
	SYSTEMTIME currentTime;
	GetLocalTime(&currentTime);
	StringCbPrintfW(date, 128, L"%02i.%02i.%0000i %02i:%02i:%02i\r\n\r\n", currentTime.wDay, currentTime.wMonth, currentTime.wYear, currentTime.wHour, currentTime.wMinute, currentTime.wSecond);
	StringCchCatW(fileHeader, 2048, L"End scanning time: ");
	StringCchCatW(fileHeader, 2048, date);
	WriteFile(hFile, fileHeader, wcslen(fileHeader) * 2, NULL, NULL);
	for (int i = 0; i < TABLE_SIZE; i++)
	{
		HashTableItem* pointer = hashTable[i];
		while (pointer != NULL)
		{
			int count = getFilesCount(pointer->files);
			if (count > 1)
				totalCountOfDuplicates += count;
			totalCountOfScanedFiles += count;
			if (isWriteAll || count > 1)
			{
				WriteFile(hFile, L"Hash value: ", wcslen(L"Hash value: ") * 2, NULL, NULL);
				for (int j = 0; j < 8; j++)
				{
					wchar_t hexValue[8];
					StringCchPrintfW(hexValue, 8, L"%x", (pointer->hashValues)[j]);
					WriteFile(hFile, hexValue, wcslen(hexValue) * 2, NULL, NULL);
				}
				WriteFile(hFile, L"\r\n", wcslen(L"\r\n") * 2, NULL, NULL);
				FileNameListNode* files = pointer->files;
				while (files != NULL)
				{
					WriteFile(hFile, files->fileName, wcslen(files->fileName) * 2, NULL, NULL);
					WriteFile(hFile, L"\r\n", wcslen(L"\r\n") * 2, NULL, NULL);
					files = files->next;
				}
				WriteFile(hFile, L"\r\n", wcslen(L"\r\n") * 2, NULL, NULL);
			}
			pointer = pointer->next;
		}
	}
	wchar_t totalScanned[128];
	wchar_t totalDuplicates[128];
	StringCbPrintfW(totalScanned, 128, L"%d\r\n", totalCountOfScanedFiles);
	StringCbPrintfW(totalDuplicates, 128, L"%d\r\n", totalCountOfDuplicates);
	WriteFile(hFile, L"Total scanned files: ", wcslen(L"Total scanned files: ") * 2, NULL, NULL);
	WriteFile(hFile, totalScanned, wcslen(totalScanned) * 2, NULL, NULL);
	WriteFile(hFile, L"Total count of duplicates: ", wcslen(L"Total count of duplicates: ") * 2, NULL, NULL);
	WriteFile(hFile, totalDuplicates, wcslen(totalDuplicates) * 2, NULL, NULL);
	CloseHandle(hFile);
}

void freeHashTableMemory()
{
	for (int i = 0; i < TABLE_SIZE; i++)
	{
		HashTableItem* hashTablePointer = hashTable[i];
		if (hashTablePointer != NULL)
		{
			while (hashTablePointer != NULL)
			{
				freeListMemory(&(hashTablePointer->files));
				HashTableItem* tmp = hashTablePointer;
				hashTablePointer = hashTablePointer->next;
				free(tmp);
			}
		}
	}
	free(hashTable);
}