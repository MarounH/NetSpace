/* -----------------------------------------------------------------------------
Description: Message broker (Publisher-subscriber)
Author: HRD31
Reviewer: Itzik
Version:
    v0.1 - For mentor approval
----------------------------------------------------------------------------- */
#ifndef __HRD31_MSG_BROKER_HPP__
#define __HRD31_MSG_BROKER_HPP__

#include <vector>
#include <algorithm>

namespace hrd31
{

template<typename MSG>
class ICallBack;

template<typename MSG>
class Dispatcher
{
public:
    explicit Dispatcher() = default;
    ~Dispatcher();
    Dispatcher(const Dispatcher& other_) = delete;
    Dispatcher& operator=(const Dispatcher& other_) = delete;

    void NotifyAll(const MSG& msg_);

private:
    std::vector<ICallBack<MSG>*> m_subscribers;
    void Register(ICallBack<MSG>* call_back_);
    void Unregister(ICallBack<MSG>* call_back_);
    
    friend class ICallBack<MSG>;
};

template<typename MSG>
class ICallBack
{
public:
    explicit ICallBack(Dispatcher<MSG>* dispatcher_);
    virtual ~ICallBack();
    ICallBack(const ICallBack& other_) = delete;
    ICallBack& operator=(const ICallBack& other_) = delete;

private:
    Dispatcher<MSG>* const m_dispatcher;

    virtual void Notify(const MSG& msg_) = 0;
    virtual void Disconnect() = 0;

    friend class Dispatcher<MSG>;
};

template<typename MSG, typename Observer>
class CallBack : public ICallBack<MSG>
{
public:
    using DoAction = void(Observer::*)(const MSG&);
    using DoStop = void(Observer::*)();

    explicit CallBack(Dispatcher<MSG>* dispatcher_, Observer& observer_,
        DoAction action_func_, DoStop stop_func_ = nullptr);
    virtual ~CallBack() = default;

    CallBack(const CallBack& other_) = delete;
    CallBack& operator=(const CallBack& other_) = delete;

    void Notify(const MSG& msg_) override;
    void Disconnect() override;

private:
    Observer& m_observer;
    DoAction m_action_func;
    DoStop m_stop_func;
};

}//namespace hrd31

namespace hrd31
{

template<typename MSG>
Dispatcher<MSG>::~Dispatcher()
{
    for(auto i : m_subscribers)
    {
        i->Disconnect();
    }
    // should call unregister ?? gaby says no
}

template<typename MSG>
void Dispatcher<MSG>::NotifyAll(const MSG& msg_)
{
    // for (auto i = m_subscribers.begin(); i != m_subscribers.end(); ++i)
    // {
    //     i->Notify(msg_);
    // }
    for (auto i : m_subscribers)
    {
        i->Notify(msg_);
    }
}

template<typename MSG>
void Dispatcher<MSG>::Register(ICallBack<MSG>* call_back_)
{
    m_subscribers.push_back(call_back_);
}

template<typename MSG>
void Dispatcher<MSG>::Unregister(ICallBack<MSG>* call_back_)
{
    auto iter = std::find(m_subscribers.begin(), m_subscribers.end(),
                                   call_back_);
    if (iter != m_subscribers.end())
    {
        m_subscribers.erase(iter);
    }
    // what if call_back_ not found
}

template<typename MSG>
ICallBack<MSG>::ICallBack(Dispatcher<MSG>* dispatcher_): m_dispatcher(dispatcher_)
{
    m_dispatcher->Register(this);
}

template<typename MSG>
ICallBack<MSG>::~ICallBack()
{
    m_dispatcher->Unregister(this);
    //Disconnect();
}

template<typename MSG, typename Observer>
CallBack<MSG,Observer>::CallBack(Dispatcher<MSG>* dispatcher_, Observer& observer_,
                                 DoAction action_func_, DoStop stop_func_): ICallBack<MSG>(dispatcher_),
                                                                            m_observer(observer_),
                                                                            m_action_func(action_func_),
                                                                            m_stop_func(stop_func_)
{

}

template<typename MSG, typename Observer>
void CallBack<MSG,Observer>::Notify(const MSG& msg_)
{
   (m_observer.*m_action_func)(msg_);
}

template<typename MSG, typename Observer>
void CallBack<MSG,Observer>::Disconnect()
{
    if (m_stop_func)
    {
        (m_observer.*m_stop_func)();
    }
}

}//namespace hrd31

#endif //__HRD31_MSG_BROKER_HPP__NotifyAll