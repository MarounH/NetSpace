

#include "i_task.hpp"

namespace hrd31
{

    //enum Priority
    //{   LOW, 
    //    MEDIUM, 
    //    HIGH, 
    //    ADMIN = __INT_MAX__  
    
ITask::ITask(Priority priority_): m_priority(priority_)
{}
    
ITask::~ITask() //virtual
{}

bool ITask::operator<(const ITask& other_) const
{
	// to do < operator for the content of shared pointer
	return m_priority < other_.m_priority;
}


}//namespace hrd31
