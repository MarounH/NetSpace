#include <iostream>
#include <thread>
#include <unistd.h>

#include "thread_pool.hpp"

using namespace hrd31;

class PrintHello: public ITask
{
private:
	void Execute() override
	{
		std::cout << "hello\n";
	}
};

class SleepTask: public ITask
{
private:
	void Execute()
	{
		sleep(5);
	}
};

int main(void)
{
	ThreadPool tp(5);



	for (int i = 0; i < 10; ++i)
	{
		tp.AddTask(std::make_shared<PrintHello>());

	}
	tp.Pause();
	/*for (int i = 0; i < 10; ++i)
	{
		tp.AddTask(std::make_shared<PrintHello>());

	}*/
	sleep(2);
	tp.Resume();

	sleep(2);
	tp.Stop();
	return 0;
}



