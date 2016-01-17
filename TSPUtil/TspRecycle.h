#if !defined(__TSP_PLUGINRECYCLE_H__)
#define __TSP_PLUGINRECYCLE_H__

#include "windows.h"
#include <list>
#include <string>
using namespace std;

/**
* @class CTspRecycle
*
* @brief ����վ�࣬����ɾ����ʱ�ļ���Ŀ¼
*/
class CTspRecycle
{
public:
	/// constrator
	CTspRecycle(){};

	/// destrator
	~CTspRecycle() {PopAll();}

	enum{ FLAG_FILE,FLAG_DIR };		// �ļ���Ŀ¼
	enum{ FLAG_DEL,FLAG_NODEL };	// ɾ����ɾ��

	/**
	* ������վ
	* @param strName �ļ���Ŀ¼��
	* @param dwFlag ��־λ:FLAG_FILE,FLAG_DIR 
	* @retval 0 �ɹ�
	*/
	int Push(const string &strName,DWORD dwFlag = CTspRecycle::FLAG_FILE );

	/**
	* ������վ
	* @param strName �ļ���Ŀ¼��
	* @param dwFlag ��־λ:FLAG_DEL,FLAG_NODEL 
	* @retval 0 �ɹ�
	*/
	int Pop(const string &strName,DWORD dwFlag = CTspRecycle::FLAG_DEL );

	/**
	* ��ջ���վ	
	* @param dwFlag ��־λ:FLAG_DEL,FLAG_NODEL 
	* @retval 0 �ɹ�
	*/
	int PopAll(DWORD dwFlag = CTspRecycle::FLAG_DEL );
private:

	// ˽�����ݽṹ,�����ļ���������
	struct PrivateData
	{
		PrivateData(const string &strName,DWORD dwFlag)
			:name(strName),flag(dwFlag){};

		inline bool operator==(const string &strName)
		{
			return name==strName;
		}

		string name;
		DWORD flag;
	};

	std::list<PrivateData> m_list;	
};


#endif	//__TSP_PLUGINRECYCLE_H__
