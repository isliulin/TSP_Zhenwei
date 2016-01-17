#if!defined (__VIRUS_SINGLETON_H__)
#define __VIRUS_SINGLETON_H__

template<typename T>
class SingletonHolder
{
public:
	typedef T ObjectType;
	static T& Instance()
	{
		if (!pInstance_)
			pInstance_ = new ObjectType();		
		return *pInstance_;
	}
	static T* instance()
	{
		if (!pInstance_)
			pInstance_ = new ObjectType();		
		return pInstance_;
	}
	~SingletonHolder()
	{
		if( pInstance_ )
			delete pInstance_;
		pInstance_ = NULL;
	}
private:
	SingletonHolder();
	static ObjectType *pInstance_;
};

template<typename T>
T *SingletonHolder<T>::pInstance_ = NULL;


#endif	//__VIRUS_SINGLETON_H__


