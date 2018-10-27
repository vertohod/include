#ifndef CRYPTO_H
#define CRYPTO_H

#include <string>
#include "types.h"

namespace sp
{

sptr_str base64_encode(const std::string&);
sptr_str base64_decode(const std::string&);

} // end of namespace sp

#endif
