#include <iostream>

#include "factory.hpp"
#include "i_task.hpp"
#include "i_re_task.hpp"
#include "re_fw.hpp"

using namespace hrd31;

enum key {CHANGE_LOG_LEVEL=0};

class IArgs
{
public:
	explicit IArgs() = default;
	virtual ~IArgs() = default;
	IArgs(const IArgs& o_) = delete;
	IArgs& operator=(const IArgs& o_) = delete;
};

struct ChangeLogLevelArgs: public IArgs
{
	explicit ChangeLogLevelArgs(Logger * logger_):
								m_logger(logger_)
	{}

	Logger *m_logger;
};

class ChangeLogLevel : public IRETask
{
public:
	explicit ChangeLogLevel(Logger * logger_):
							m_logger(logger_)
	{}
	~ChangeLogLevel() = default;

	ChangeLogLevel(const ChangeLogLevel& o_) = delete;
	ChangeLogLevel& operator=(const ChangeLogLevel& o_) = delete;

	static std::shared_ptr<IRETask> CreateFunc(std::shared_ptr<IArgs> args_)
	{
		std::shared_ptr<ChangeLogLevelArgs> arguments = 
		std::dynamic_pointer_cast<ChangeLogLevelArgs>(args_);

		return std::make_shared<ChangeLogLevel>(arguments->m_logger);
	}

	void Execute() override
	{
		m_logger->SetLogLevel(Logger::DEBUG);
		std::cout << "I'm shared object injected on run-time ye babyyyyyyyyy\n";
	}

private:
	Logger *m_logger;
};

__attribute__((constructor)) void Test()
{
	// Factory<IRETask,key,IArgs> task(); 	
	// get instance and factory->add()
}


// class InjectCode : public ITask
// {
// public:
// 	explicit InjectCode() = default;
// 	~InjectCode() = default;

// 	InjectCode(const InjectCode& o_) = delete;
// 	InjectCode& operator=(const InjectCode& o_) = delete;

// 	void Execute() override
// 	{
// 		std::cout << "I'm shared object injected on run-time ye babyyyyyyyyy\n";
// 	}
// };

// __attribute__((constructor)) void Test()
// {
// 	Factory<ITask,int,int> task(); 	
// }
