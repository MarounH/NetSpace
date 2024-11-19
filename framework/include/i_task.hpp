/* -----------------------------------------------------------------------------
Description: 
Author: HRD31
Reviewer: Itzik
Version:
    v0.1 - For mentor approval
    v0.2 - Mentor approved
    v0.2.1 - removed #include "thread_pool.hpp"
----------------------------------------------------------------------------- */
#ifndef __HRD31_ITASK_HPP__
#define __HRD31_ITASK_HPP__


namespace hrd31
{

class ITask
{
public:
    enum Priority
    {   LOW, 
        MEDIUM, 
        HIGH, 
        ADMIN = __INT_MAX__ // a new priority can be added before ADMIN. 
        //  ADMIN should always be the last priority for internal use
    };
    
    explicit ITask(Priority priority_ = MEDIUM);
    ITask(const ITask& other_) = delete;
    ITask& operator=(const ITask& other_) = delete;
    virtual ~ITask();

    bool operator<(const ITask& other_) const;

private:
    virtual void Execute() = 0;
    friend class ThreadPool;

    Priority m_priority;
};



}//namespace hrd31

#endif //__HRD31_ITASK_HPP__
