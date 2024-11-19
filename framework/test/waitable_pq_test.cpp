#include <iostream>
#include <thread>

#include "waitable_pq.hpp"

void ThreadPushElement();

using namespace hrd31;
int main()
{
	WPQueue<std::string> wpq;
	
	std::cout << "size = " << wpq.Size() << std::endl;
	std::cout << "is_empty = " << wpq.IsEmpty() << std::endl;
	
	wpq.Push("hello");
	wpq.Push("gaby noob");
	wpq.Push("gaby noob1");
	wpq.Push("gaby noob2");
	wpq.Push("gaby noob3");
	wpq.Push("gaby noob4");
	wpq.Push("gaby noob5");
	
	//std::string& popped1 = wpq.Pop();
	std::cout << "popped element = " << wpq.Pop() << std::endl;
	std::cout << "popped element = " << wpq.Pop() << std::endl;
	std::cout << "popped element = " << wpq.Pop() << std::endl;
	std::cout << "popped element = " << wpq.Pop() << std::endl;
	std::cout << "popped element = " << wpq.Pop() << std::endl;
	std::cout << "popped element = " << wpq.Pop() << std::endl;
	std::cout << "popped element = " << wpq.Pop() << std::endl;
	//std::thread t(ThreadPushElement);
	return 0;
}

void ThreadPushElement(WPQueue<std::string> &wpq_)
{
	wpq_.Push("Thread push");
}
