#ifndef SERIALIZER_HPP
#define SERIALIZER_HPP
#include "member_list.hpp"
#include "serialize.hpp"

namespace serializer {

/******************************************************************************/
/*                                 serializer                                 */
/******************************************************************************/

struct serialize_t {};
struct deserialize_t {};

template <typename Conv, typename T>
struct Serializer;

template <typename Conv>
struct Serializer<Conv, serialize_t> {
  /* template <typename ...Args> */
  /* size_t operator()(typename Conv::mem_type &mem, size_t pos, */
  /*                           tools::mtf::ser_arg_type_t<Args>... args) { */

  /* } */
/* }; */

/* template <typename Conv> */
/* struct Serializer<Conv, deserialize_t> { */
  /* template <typename ...Args> */
  /* size_t operator()(typename Conv::mem_type &mem, size_t pos, */
  /*                             tools::mtf::arg_type_t<Args>... args) { */

  /* } */
};

} // namespace serializer

#endif
