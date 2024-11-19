/* -----------------------------------------------------------------------------
Description: Generic factory for runtime object creation
Author: HRD31
Reviewer: Itzik
Version:
    v0.1 - For mentor approval
----------------------------------------------------------------------------- */
#ifndef __HRD31_FACTORY_HPP__
#define __HRD31_FACTORY_HPP__

#include <functional> //std::function
#include <unordered_map> //std::unordered_map

#include "singleton.hpp"

namespace hrd31
{
//Use instructions:
//KEY must support comparison operators
template <typename BASE, typename KEY, typename ARGS>
class Factory
{
public:
    using CreateFunc = std::function<std::shared_ptr<BASE>(ARGS)>;
    ~Factory() = default;
    Factory(const Factory& other_) = delete;
    Factory& operator=(const Factory& other_) = delete;

    //If KEY exists, create_func_ is updated
    void Add(const KEY& key_, CreateFunc create_func_);
    std::shared_ptr<BASE> Create(const KEY& key_, ARGS args_) const;

private:
    explicit Factory() = default;

    friend class Singleton<Factory>; 

    std::unordered_map<KEY, CreateFunc> m_operations;
};


}//namespace hrd31

namespace hrd31
{
template <typename BASE, typename KEY, typename ARGS>
void Factory<BASE, KEY, ARGS>::Add(const KEY& key_, 
								   Factory<BASE, KEY, ARGS>::
								   CreateFunc create_func_)
{
	//m_operations.insert_or_assign(key_, create_func_);
    m_operations[key_] = create_func_;
    //m_operations.insert(std::make_pair(key_, create_func_));
}

template <typename BASE, typename KEY, typename ARGS>
std::shared_ptr<BASE> Factory<BASE, KEY, ARGS>::
	Create(const KEY& key_, ARGS args_) const
{
	return m_operations.at(key_)(args_);
}
} //namespace hrd31
#endif //__HRD31_FACTORY_HPP__
