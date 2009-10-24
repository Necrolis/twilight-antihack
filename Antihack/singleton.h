#ifndef INC_SINGLETON
#define INC_SINGLETON

#include <windows.h>

template <typename T>
class CSingleton {
	public:
		CSingleton();
		~CSingleton();

		static T* Get();
		int ThreadRequestResources();
		int ThreadReleaseResources();

	private:
		CRITICAL_SECTION critSection;
};

template <typename T>
CSingleton<T>::CSingleton()
{
	InitializeCriticalSection(&critSection);
}

template <typename T>
CSingleton<T>::~CSingleton()
{
	DeleteCriticalSection(&critSection);
}

template <typename T>
T* CSingleton<T>::Get()
{
	static T instance;
	return &instance;
}

template <typename T>
int CSingleton<T>::ThreadRequestResources()
{
	EnterCriticalSection(&critSection);

	return 1;
}

template <typename T>
int CSingleton<T>::ThreadReleaseResources()
{
	LeaveCriticalSection(&critSection);

	return 1;
}

#endif