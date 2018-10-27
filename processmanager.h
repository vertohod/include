#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <unordered_map>
#include <sys/types.h>
#include <signal.h>
#include <string>
#include <vector>
#include <memory>
#include <mutex>

#include "types.h"

struct proc_info
{
	pid_t			m_pid;
	sptr_cstr		m_line;
	time_t			m_start;

	proc_info();
	proc_info(pid_t pid, sptr_cstr line);
};

class proc_manager
{
	typedef std::unordered_map<pid_t, sptr<proc_info>> proc_map_t;

	proc_map_t m_proc_map;

	std::mutex m_mutex;

public:
	pid_t start(sptr_cstr comm);
	bool stop(pid_t);
	void stop_all();
	sptr<proc_info> get_info(pid_t);

private:
	bool stop_without_lock(pid_t);
};

#endif

