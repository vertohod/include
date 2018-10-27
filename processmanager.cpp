#include <ctime>

#include "processmanager.h"
#include "util.h"

proc_info::proc_info() : m_pid(0), m_start(0) {}

proc_info::proc_info(pid_t pid, sptr_cstr line) : 
	m_pid(pid), m_line(line), m_start(time(NULL)) {}


pid_t proc_manager::start(sptr_cstr comm)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	pid_t pid = util::exec_command(*comm);

	m_proc_map.insert(std::make_pair(pid, sptr<proc_info>(new proc_info(pid, comm))));

	return pid;
}	

bool proc_manager::stop(pid_t pid)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	return stop_without_lock(pid);
}

bool proc_manager::stop_without_lock(pid_t pid)
{
	return (0 == kill(pid, SIGKILL));
}

void proc_manager::stop_all()
{
	for (auto& proc_pr : m_proc_map) stop(proc_pr.first);
}

sptr<proc_info> proc_manager::get_info(pid_t pid)
{
	return m_proc_map[pid];
}
