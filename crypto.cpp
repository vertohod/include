#include <base64.h>

#include "crypto.h"

namespace sp
{

sptr_str base64_encode(const std::string& str)
{
	using CryptoPP::Name::Pad;
	using CryptoPP::Name::InsertLineBreaks;

	sptr_str res;

	CryptoPP::Base64Encoder encoder;
	CryptoPP::AlgorithmParameters params = CryptoPP::MakeParameters(Pad(), false)(InsertLineBreaks(), false);
	encoder.IsolatedInitialize(params);

	encoder.Put(reinterpret_cast<const byte*>(str.c_str()), str.size());
	encoder.MessageEnd();
	res->resize(encoder.MaxRetrievable());
	encoder.Get(reinterpret_cast<byte*>(&(*res)[0]), res->size());

	return res;
}

sptr_str base64_decode(const std::string& str)
{
	sptr_str res;

	CryptoPP::Base64Decoder decoder;
	decoder.Put(reinterpret_cast<const byte*>(str.c_str()), str.size());
	decoder.MessageEnd();
	res->resize(decoder.MaxRetrievable());
	decoder.Get(reinterpret_cast<byte*>(&(*res)[0]), res->size());

	return res;
}

} // end of namespace sp
