#include <iostream>
#include <unistd.h> // sleep
#include "re_fw.hpp"

using namespace hrd31;

class PrintTask: public IRETask
{
public:
	explicit PrintTask(Priority priority_ = MEDIUM): IRETask(priority_)
	{

	}
	static std::shared_ptr<IRETask> CreateFunc(int a_)
	{
		return std::make_shared<PrintTask>();
	}
	void Execute() override
	{
		std::cout << "hello from PrintTask\n";
	}
private:
};

class Input: public IInputSrc<std::string,int>
{
public:
	std::pair<std::string,int> Read() override
	{
		std::cout << "In read shel Input\n";
		return std::make_pair("input", 2);
	}

	int GetFd() override
	{
		//return fileno(stdin);
		return STDIN_FILENO;
	}

};

int main()
{
	RequestEngine<std::string,int> re;
	std::shared_ptr<IInputSrc<std::string,int>> input = std::make_shared<Input>();
	PrintTask task1;
	
	re.ConfigInputSrc(input);
	re.ConfigTask(std::string("input") , PrintTask::CreateFunc);

	re.Run();
	sleep(2);

	re.Stop();
	sleep(2);
	return 0;
}
