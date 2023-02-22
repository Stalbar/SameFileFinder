#include <stdlib.h>
#include <windows.h>
#include <strsafe.h>

struct FileNameList {
	LPWSTR fileName;
	struct FileNameList* next;
};

typedef struct FileNameList FileNameListNode;

FileNameListNode* addFileNameListNode(FileNameListNode* head, LPWSTR value)
{
	FileNameListNode* tmp = (FileNameListNode*) malloc (sizeof (FileNameListNode));
	size_t maxLength = MAX_PATH;
	size_t valueLength = 0;
	StringCchLengthW(value, maxLength, &valueLength);
	tmp->fileName = (LPWSTR) malloc(sizeof(wchar_t) * (valueLength + 1));
	StringCchCopyW(tmp->fileName, sizeof(wchar_t) * (valueLength + 1), value);
	tmp->next = NULL;
	if (head == NULL)
	{
		head = tmp;
	}
	else
	{
		FileNameListNode* p = head;
		while (p->next != NULL)
			p = p->next;
		p->next = tmp;
	}
	return head;
}

uint32_t getFilesCount(FileNameListNode* head)
{
	uint32_t count = 0;
	while (head != NULL)
	{
		count++;
		head = head->next;
	}
	return count;
}

void printFileNameList(FileNameListNode* head)
{
	while (head != NULL)
	{
		printf("%ls\n", head->fileName);
		head = head->next;
	}
}

void freeListMemory(FileNameListNode** head)
{
	while (*head != NULL)
	{
		FileNameListNode* tmp = *head;
		*head = (*head)->next;
		free(tmp->fileName);
		free(tmp);
	}
}