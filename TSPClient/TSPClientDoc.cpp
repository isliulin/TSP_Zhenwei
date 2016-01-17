// gh0stDoc.cpp : implementation of the CTSPClientDoc class
//

#include "stdafx.h"
#include "TSPClient.h"
#include "TSPClientDoc.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTSPClientDoc

IMPLEMENT_DYNCREATE(CTSPClientDoc, CDocument)

BEGIN_MESSAGE_MAP(CTSPClientDoc, CDocument)
	//{{AFX_MSG_MAP(CTSPClientDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTSPClientDoc construction/destruction

CTSPClientDoc::CTSPClientDoc()
{
	// TODO: add one-time construction code here

}

CTSPClientDoc::~CTSPClientDoc()
{
}

//DEL BOOL CTSPClientDoc::OnNewDocument()
//DEL {
//DEL 	if (!CDocument::OnNewDocument())
//DEL 		return FALSE;
//DEL 
//DEL 	// TODO: add reinitialization code here
//DEL 	// (SDI documents will reuse this document)
//DEL 	return TRUE;
//DEL }



/////////////////////////////////////////////////////////////////////////////
// CTSPClientDoc serialization

void CTSPClientDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTSPClientDoc diagnostics

#ifdef _DEBUG
void CTSPClientDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTSPClientDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTSPClientDoc commands

void CTSPClientDoc::DeleteContents() 
{
	// TODO: Add your specialized code here and/or call the base class
	if(AfxGetApp()->m_pMainWnd)
		((CMainFrame*)AfxGetApp()->m_pMainWnd)->DeleteContents();
	CDocument::DeleteContents();
}
