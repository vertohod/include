#include "types.h"

bool operator<(const sptr_cstr& ref1, const sptr_cstr& ref2)
{
	return *ref1 < *ref2;
}

bool comparer_sptr_cstr::operator()(const sptr_cstr& ref1, const sptr_cstr& ref2) const
{
	return ref1 < ref2;
}

bool equal_sptr_cstr::operator()(const sptr_cstr& ref1, const sptr_cstr& ref2) const
{
	return *ref1 == *ref2;
}

bool operator==(const sptr_cstr& ref1, const sptr_cstr& ref2)
{
	return *ref1 == *ref2;
}

size_t std::hash<sptr_cstr>::operator()(const sptr_cstr& obj) const
{
    return std::hash<std::string>()(*obj);
}
