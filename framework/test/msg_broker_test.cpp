#include <iostream>

#include "msg_broker.hpp"

using namespace hrd31;

class Fridge
{
public:
	explicit Fridge(Dispatcher<std::string> *dispatcher_):
					 m_call_back(dispatcher_, *this, &Fridge::ActionFunc,
					   			&Fridge::StopFunc)
					   
	{

	}

	~Fridge() = default;

	void ActionFunc(const std::string& s_)
	{
		std::cout << s_ << "\n";
	}
	
    void StopFunc()
	{
		std::cout << "Fridge temprature is not being monitored";
	}

	CallBack<std::string,Fridge> m_call_back;
	friend class Thermostat;
};

class Thermostat
{
public:
	explicit Thermostat() = default;
	~Thermostat()
	{}

	Dispatcher<std::string> m_dispatcher;
	friend class Fridge;
};

int main()
{
	Thermostat thermostat;
	Fridge fridge(&thermostat.m_dispatcher);
	
	thermostat.m_dispatcher.NotifyAll("Temprature is high");

	return 0;
}
