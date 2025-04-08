#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include "list.h"

int List_Init_Thread(struct List *pRootList)
{
	pRootList->p_Head = NULL;
	pRootList->p_Tail = NULL;
	pRootList->count = 0;
	
	pRootList->mutexOpt = 1;
	pthread_mutex_init(&pRootList->mutex, NULL);

	return 0;
}

int List_Init(struct List *pRootList)
{
	pRootList->p_Head = NULL;
	pRootList->p_Tail = NULL;
	pRootList->count = 0;
	pRootList->mutexOpt = 0;

	return 0;
}

int List_Insert(struct List *pRootList, void *arg)
{
	struct ListNode *new_node;
	
	new_node = (struct ListNode *)malloc(sizeof(struct ListNode));
	if(new_node == NULL) {
		printf("List_Insert:malloc ERROR!\n");
		return -1;
	}

	new_node->next = NULL;
    new_node->arg = arg;
	
	if (pRootList->mutexOpt) {
		pthread_mutex_lock(&pRootList->mutex);
	}
	
	if(pRootList->p_Head == NULL) {
		pRootList->p_Head = new_node;
		pRootList->p_Tail = new_node;
	} else {
		pRootList->p_Tail->next = new_node;
		pRootList->p_Tail = new_node;
	}
	  
	pRootList->count++;
	
	if (pRootList->mutexOpt) {
		pthread_mutex_unlock(&pRootList->mutex);
	}
	
	return 0;
}

struct ListNode *List_GetHead(struct List *pRootList)
{
	if(pRootList == NULL || pRootList->count <= 0 || pRootList->p_Head == NULL) {
		return NULL;
	}

	return(pRootList->p_Head);
}


int List_DelHead(struct List *pRootList)
{
	struct ListNode *pTmpNode;
	
	if (pRootList->mutexOpt)
	{
		pthread_mutex_lock(&pRootList->mutex);
	}
	
	if(pRootList->count <= 0 || pRootList->p_Head == NULL)
	{
		if (pRootList->mutexOpt)
		{
			pthread_mutex_unlock(&pRootList->mutex);
		}
		return -1;
	}
	pTmpNode = pRootList->p_Head;
	
	pRootList->p_Head = pRootList->p_Head->next;
	pRootList->count--;
	if(pRootList->count == 0)
	{
		pRootList->p_Head = NULL;
		pRootList->p_Tail = NULL;
	}
	free(pTmpNode);
	
	if (pRootList->mutexOpt)
	{
		pthread_mutex_unlock(&pRootList->mutex);
	}
	
	return 0;
}

