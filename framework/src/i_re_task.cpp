

#include "i_re_task.hpp"

namespace hrd31
{
//note: for using this interface for creating tasks you have to provide
// std::shared_ptr<IRETask> CreateFunc(ARGS) - not member function
//Available priorities: LOW, MEDIUM, HIGH

IRETask::IRETask(Priority priority_): ITask(priority_)
{}

IRETask::~IRETask()
{}

} //namespace hrd31
