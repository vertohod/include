#ifndef TYPES_H
#define TYPES_H

#include <boost/uuid/uuid_generators.hpp>
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <cstddef>
#include <list>
#include <map>

typedef boost::uuids::uuid  UUID;
typedef uint64_t            OID;

#if defined(__APPLE__)
    typedef int64_t time_tt;
    typedef uint64_t size_tt;
#elif defined(__LINUX__)
    typedef time_t time_tt;
    typedef size_t size_tt;
#else
    typedef time_t time_tt;
    typedef size_t size_tt;
#endif

// Типы строк
template <typename STR>
class uptr_string;

typedef uptr_string<std::string> uptr_str;
typedef uptr_string<const std::string> uptr_cstr;

template <typename STR>
class wptr_string;

typedef wptr_string<std::string> wptr_str;
typedef wptr_string<const std::string> wptr_cstr;

template <typename STR>
class sptr_string : public std::shared_ptr<STR>
{
private:
	void reset() noexcept;
	template <class U> void reset (U* p);
	template <class U, class D> void reset (U* p, D del);
	template <class U, class D, class Alloc> void reset (U* p, D del, Alloc alloc);

public:
	sptr_string() : std::shared_ptr<STR>(new STR()) {}
	template <typename U> explicit sptr_string(U* p) : std::shared_ptr<STR>(p) {}
	template <typename U, typename D> sptr_string(U* p, D del) : std::shared_ptr<STR>(p, del) {}
	template <typename U, typename D, typename Alloc> sptr_string(U* p, D del, Alloc alloc) : std::shared_ptr<STR>(p, del, alloc) {}
	sptr_string(const sptr_string& x) noexcept : std::shared_ptr<STR>(x) {}
	template <typename U> sptr_string(const sptr_string<U>& x) noexcept : std::shared_ptr<STR>(x) {}
	template <typename U> explicit sptr_string(const wptr_string<U>& x) : std::shared_ptr<STR>(x) {}
	sptr_string(sptr_string&& x) noexcept : std::shared_ptr<STR>(x) {}
	template <typename U> sptr_string(sptr_string<U>&& x) noexcept : std::shared_ptr<STR>(x) {}
	template <typename U> sptr_string(uptr_string<U>&& x) noexcept : std::shared_ptr<STR>(static_cast<std::unique_ptr<U>&&>(x)) {}

	sptr_string& operator=(const sptr_string& x) noexcept
	{
		std::shared_ptr<STR>::operator=(x);
		return *this;
	}

	template <typename U>
	sptr_string& operator=(const sptr_string<U>& x) noexcept
	{
		std::shared_ptr<STR>::operator=(x);
		return *this;
	}

	sptr_string& operator=(sptr_string&& x) noexcept
	{
		std::shared_ptr<STR>::operator=(x);
		return *this;
	}

	template <typename U>
	sptr_string& operator=(sptr_string<U>&& x) noexcept
	{
		std::shared_ptr<STR>::operator=(x);
		return *this;
	}

	template <typename U> sptr_string& operator=(uptr_string<U>&& x)
	{
		std::shared_ptr<STR>::operator=(x);
		return *this;
	}

	sptr_string(const char* c_ptr) : std::shared_ptr<STR>(new STR(c_ptr)) {}
	sptr_string(const std::string& s) : std::shared_ptr<STR>(new STR(s)) {}

	// ----
	sptr_string(const wptr_string<STR>& x) : std::shared_ptr<STR>(x.lock()) {}

	sptr_string& operator=(const wptr_string<STR>& x) {
		std::shared_ptr<STR>::operator=(x.lock());
		return *this;
	}

    bool operator==(const sptr_string<STR>& x) noexcept
    {
        return *(*this) == *x;
    }
};

typedef sptr_string<std::string> sptr_str;
typedef sptr_string<const std::string> sptr_cstr;

template <typename STR>
class wptr_string : public std::weak_ptr<STR>
{
private:
	constexpr wptr_string() noexcept;
	void reset() noexcept;

public:
	wptr_string(const wptr_string& x) noexcept : std::weak_ptr<STR>(x) {}
	template <class U> wptr_string (const wptr_string<U>& x) noexcept : std::weak_ptr<STR>(x) {}
	template <class U> wptr_string (const sptr_string<U>& x) noexcept : std::weak_ptr<STR>(x) {}

	wptr_string& operator=(const wptr_string& x) noexcept
	{
		std::weak_ptr<STR>::operator=(x);
		return *this;
	}

	template <class U> wptr_string& operator=(const wptr_string<U>& x) noexcept
	{
		std::weak_ptr<STR>::operator=(x);
		return *this;
	}

	template <class U> wptr_string& operator=(const sptr_string<U>& x) noexcept
	{
		std::weak_ptr<STR>::operator=(x);
		return *this;
	}
};

template <typename STR>
class uptr_string : public std::unique_ptr<STR>
{
private:
	template <typename U> void reset(U* p) noexcept;

public:
	constexpr uptr_string() noexcept : std::unique_ptr<STR>(new STR()) {}
	explicit uptr_string(STR* p) noexcept : std::unique_ptr<STR>(p) {}
	uptr_string(const uptr_string& x) noexcept
	{
		std::unique_ptr<STR>::swap(*const_cast<uptr_string*>(&x));
	}

	template <typename U>
	uptr_string(const uptr_string<U>& x) noexcept : std::unique_ptr<STR>(const_cast<uptr_string<U>*>(&x)->release()) {}

	uptr_string& operator=(uptr_string&& x) noexcept
	{
		std::unique_ptr<STR>::operator=(x);
	}

	template <typename U>
	uptr_string& operator=(uptr_string<U>&& x) noexcept
	{
		std::unique_ptr<STR>::operator=(x);
	}

	uptr_string& operator=(const uptr_string& x) noexcept
	{
		std::unique_ptr<STR>::swap(x);
	}

	uptr_string(const char* c_ptr) : std::unique_ptr<STR>(new STR(c_ptr)) {}
	uptr_string(const std::string& s) : std::unique_ptr<STR>(new STR(s)) {}
};

bool operator<(const sptr_cstr& ref1, const sptr_cstr& ref2);
struct comparer_sptr_cstr
{
    bool operator()(const sptr_cstr& ref1, const sptr_cstr& ref2) const;
};
bool operator==(const sptr_cstr& ref1, const sptr_cstr& ref2);
struct equal_sptr_cstr
{
	bool operator()(const sptr_cstr& ref1, const sptr_cstr& ref2) const;
};

namespace std {
    template<>
    class hash<sptr_cstr>
    {
    public:
        size_t operator()(const sptr_cstr&) const;
    };
}

#endif
