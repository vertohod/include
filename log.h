#ifndef LO_H
#define LO_H

#include <exception>
#include <iostream>
#include <string>

class lo
{
public:

	enum log_level_t {
		TRASH	= 1,
		DB		= 2,
		DEBUG	= 3,
		INFO	= 4,
		IN		= 5,
		OUT		= 6,
		WARNING = 7,
		ERROR	= 8,
		FATAL	= 9
	};

	static log_level_t log_level;
	static std::string m_path;

	class l
	{
		private:
		log_level_t m_type;

		public:
		l(log_level_t arg_type = INFO);
		l(const l& obj);
		~l();

		template <typename T>
		l& operator<<(T val) {
			if (m_type >= log_level) {
				try {
					std::cout << val;
				} catch (const std::ostream::failure& e) {
					set_output(m_path);
					std::cout << val;
				}
			}
			return *this;
		}
	};

	static void set_output(const std::string&);
	static void set_log_level(log_level_t);
	static std::string get_time();
	static log_level_t trans_level(const std::string&);
	static std::string get_level();

private:
	static std::string get_level_str(log_level_t);

};

#endif
