#ifndef SERIALIZER_H
#define SERIALIZER_H

#include "tools/type_table.hpp"
#include "tools/super.hpp"
#include "tools/bytes.hpp"
#include "tools/context.hpp"
#include "tools/dynamic_array.hpp"
#include "serializer/serialize.hpp"
#include "serializer/serializer.hpp"
#include "serialize.hpp"

/// Useful alias:

/// @brief namespace serialzer
namespace serializer {

/// @breif alias for bytes
using Bytes = serializer::tools::Bytes<std::byte>;
}

#endif
