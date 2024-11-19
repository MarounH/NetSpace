/* -----------------------------------------------------------------------------
Description: Request engine input source
Author: HRD31
Reviewer: Itzik
Version:
    v0.1 - For mentor approval
    v0.2 - Mentor approved
----------------------------------------------------------------------------- */
#ifndef __HRD31_I_INPUT_SRC_HPP__
#define __HRD31_I_INPUT_SRC_HPP__

namespace hrd31
{

template <typename KEY, typename ARGS> 
class IInputSrc
{
public:
    explicit IInputSrc() = default;
    virtual ~IInputSrc() = default;
    IInputSrc(const IInputSrc&) = delete;
    IInputSrc& operator=(const IInputSrc&) = delete;

    virtual std::pair<KEY, ARGS> Read() = 0;
    virtual int GetFd() = 0;
};

}//namespace hrd31
#endif //__HRD31_I_INPUT_SRC_HPP__