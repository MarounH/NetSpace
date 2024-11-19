/* -----------------------------------------------------------------------------
Description: Request engine task
Author: HRD31
Reviewer: Itzik
Version:
    v0.1 - For mentor approval
    v0.2 - Mentor approved
----------------------------------------------------------------------------- */
#ifndef __HRD31_I_RE_TASK_HPP__
#define __HRD31_I_RE_TASK_HPP__

#include "i_task.hpp"

namespace hrd31
{
//note: for using this interface for creating tasks you have to provide
// std::shared_ptr<IRETask> CreateFunc(ARGS) - not member function
//Available priorities: LOW, MEDIUM, HIGH
class IRETask : public ITask
{
public:
    explicit IRETask(Priority priority_ = MEDIUM);
    virtual ~IRETask();
    IRETask(const IRETask&) = default;
    IRETask& operator=(const IRETask&) = default;

private:
    virtual void Execute() = 0;
};

}//namespace hrd31

#endif //__HRD31_I_RE_TASK_HPP__