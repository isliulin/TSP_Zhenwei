#include "StdAfx.h"
#include "Tsplist.h"

CTspList::CTspList(void)
{
    pFirstNode = new node;
    pLastNode = new node;

    pFirstNode->pPrevNode = NULL;
    pFirstNode->pNextNode = pLastNode;
    pLastNode->pPrevNode = pFirstNode;
    pLastNode->pNextNode = NULL;
}

CTspList::~CTspList(void)
{
    node* pCur = pFirstNode;
    node* pNext = NULL;

    while (pCur != NULL)
    {
        pNext = pCur->pNextNode;
        delete pCur;
        pCur = pNext;
    }

    pFirstNode = NULL;
    pLastNode = NULL;
}

void CTspList::push_back(char* szStr)
{
    node* pCur = NULL;

    if (szStr == NULL)
    {
        return;
    }

    pCur = new node(szStr);
    if (pCur == NULL)
    {
        return;
    }
    pCur->pPrevNode = pLastNode->pPrevNode;
    pCur->pNextNode = pLastNode;
    pLastNode->pPrevNode->pNextNode = pCur;
    pLastNode->pPrevNode = pCur;
}

void CTspList::clear(void)
{
    node* pCur = pFirstNode->pNextNode;
    node* pNext = NULL;

    while (pCur != pLastNode)
    {
        pNext = pCur->pNextNode;
        delete pCur;
        pCur = pNext;
    }

    pFirstNode->pPrevNode = NULL;
    pFirstNode->pNextNode = pLastNode;
    pLastNode->pPrevNode = pFirstNode;
    pLastNode->pNextNode = NULL;
}

bool CTspList::empty(void)
{
    if (pFirstNode->pNextNode == pLastNode)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void CTspList::erase(int nIndex)
{
    node* pCur = pFirstNode->pNextNode;

    while (nIndex-- != 0)
    {
        pCur = pCur->pNextNode;

        if (pCur == NULL)
        {
            return;
        }
    }
//	if (pCur->pNextNode&&pCur->pPrevNode)
//	{
	pCur->pNextNode->pPrevNode = pCur->pPrevNode;
	pCur->pPrevNode->pNextNode = pCur->pNextNode;

	//}
	delete pCur;



}

LPCSTR CTspList::operator[](int nIndex)
{
    node* pCur = pFirstNode->pNextNode;

    while (nIndex-- != 0)
    {
        pCur = pCur->pNextNode;

        if (pCur == NULL)
        {
            return NULL;
        }
    }

    return pCur->lpData;
}
