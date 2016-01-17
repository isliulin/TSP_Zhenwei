#pragma once

class CTspList;

class node
{
    friend CTspList;
public:
    node(LPCSTR lpStr = NULL)
    {
        if (lpStr == NULL)
        {
            lpData = NULL;
            return;
        }

        lpData = _strdup(lpStr);
    }
    ~node(void)
    {
        if (lpData != NULL)
        {
            free(lpData);
            lpData = NULL;
        }
    }
private:
    node* pPrevNode;
    node* pNextNode;
    char* lpData;
};

class CTspList
{
public:
    CTspList(void);
    ~CTspList(void);
private:
    node* pFirstNode;
    node* pLastNode;
public:
    void push_back(char* szStr);

    void clear(void);
    bool empty(void);
    void erase(int nIndex);
    LPCSTR operator[](int nIndex);
};
