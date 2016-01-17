#if !defined(__TSP_PLUGINRECYCLE_H__)
#define __TSP_PLUGINRECYCLE_H__

#include "windows.h"
#include <list>
#include <string>
using namespace std;

/**
* @class CTspRecycle
*
* @brief 回收站类，负责删除临时文件和目录
*/
class CTspRecycle
{
public:
	/// constrator
	CTspRecycle(){};

	/// destrator
	~CTspRecycle() {PopAll();}

	enum{ FLAG_FILE,FLAG_DIR };		// 文件或目录
	enum{ FLAG_DEL,FLAG_NODEL };	// 删除或不删除

	/**
	* 进回收站
	* @param strName 文件或目录名
	* @param dwFlag 标志位:FLAG_FILE,FLAG_DIR 
	* @retval 0 成功
	*/
	int Push(const string &strName,DWORD dwFlag = CTspRecycle::FLAG_FILE );

	/**
	* 出回收站
	* @param strName 文件或目录名
	* @param dwFlag 标志位:FLAG_DEL,FLAG_NODEL 
	* @retval 0 成功
	*/
	int Pop(const string &strName,DWORD dwFlag = CTspRecycle::FLAG_DEL );

	/**
	* 清空回收站	
	* @param dwFlag 标志位:FLAG_DEL,FLAG_NODEL 
	* @retval 0 成功
	*/
	int PopAll(DWORD dwFlag = CTspRecycle::FLAG_DEL );
private:

	// 私有数据结构,保存文件名和类型
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
