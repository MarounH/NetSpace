#include <iostream>
#include <chrono>
#include <thread>

#include "logger.hpp"

namespace hrd31
{
Logger::Logger(): m_stream("./log", std::ofstream::out | std::ofstream::app),
						   m_log_level(ERROR)
{}

Logger::~Logger()
{
	m_stream.close();
}

void Logger::SetLogLevel(Logger::log_level_t level_)
{
	m_log_level = level_;
}

void Logger::SetFilePath(const std::string& path_to_file_)
{
	if (m_stream.is_open())
	{
		m_stream.close();
	}
	m_stream.open(path_to_file_, std::ofstream::out | std::ofstream::app);
}

void Logger::Log(Logger::log_level_t level_,
            	 const std::string& msg_, 
            	 const std::string& file_, 
            	 int line_,  
            	 bool print_outstream_)
{
	if (level_ > m_log_level)
	{
		return;
	}
	
	LogToStream(m_stream, msg_, file_, line_);
	
 	if (print_outstream_)
 	{
		LogToStream(std::cout, msg_, file_, line_);
 	}
}

void Logger::LogToStream(std::ostream& stream_, const std::string& msg_, 
					 	 const std::string& file_, int line_)
{
	static std::mutex s_mutex;
	std::lock_guard<std::mutex> lock(s_mutex);
	stream_ << std::endl <<
	std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) 
	<< " In File:" << file_
	 		 << " Line:" << line_ << " thread#" << std::this_thread::get_id() 
	 		 << "\n" << msg_; 
}
} //namespace hrd31
