/*
Description:
Author: HRD31
Reviewer: Itzik
Version:
    v0.1 - For mentor approval
*/


//Use instruction:
//Must define default Ctor
//Default Ctor must be declared in private
//Using class must declare Singleton as a friend
#ifndef __HRD31__SINGLETON__HPP__
#define __HRD31__SINGLETON__HPP__

#include <atomic>
#include <mutex>
#include <memory>

template <typename T>
class Singleton
{
public:
    static T* GetInstance();

    Singleton() = delete;
    ~Singleton() = delete; //Automatic destruction
    Singleton(const Singleton<T>& other_) = delete;
    Singleton<T>& operator=(const Singleton<T>& other_) = delete;

private:
    static std::atomic<T*> s_instance;
    static std::mutex s_mutex;
};

template <typename T>
std::atomic<T*> Singleton<T>::s_instance;
template <typename T>
std::mutex Singleton<T>::s_mutex;

template <typename T>
T* Singleton<T>::GetInstance()
{
	T* tmp = s_instance.load(std::memory_order_relaxed);
	std::atomic_thread_fence(std::memory_order_acquire); //barrier
	if (nullptr == tmp)
	{
		std::lock_guard<std::mutex> lock(s_mutex);
		tmp = s_instance.load(std::memory_order_relaxed);
		if (nullptr == tmp) //double check 
		{
			tmp = new T;
			static std::unique_ptr<T> static_local_instance(tmp);
			
			std::atomic_thread_fence(std::memory_order_release); //release barrier
			s_instance.store(static_local_instance.get(),
							 std::memory_order_relaxed);
		}
	}
	return tmp;
}

#endif // __HRD31__SINGLETON__HPP__