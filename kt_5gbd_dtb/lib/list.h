#ifndef __LIST_H
#define __LIST_H


struct ListNode
{
	struct ListNode *next;
	void *arg;
} ;

struct List
{
	struct ListNode *p_Head;
	struct ListNode *p_Tail;
	uint16_t count;
	
	int mutexOpt;
	pthread_mutex_t mutex;  
} ;

int List_Init(struct List *pRootList);
int List_Init_Thread(struct List *pRootList);
int List_Insert(struct List *pRootList, void *arg);
int List_DelHead(struct List *pRootList);
struct ListNode *List_GetHead(struct List *pRootList);

#endif