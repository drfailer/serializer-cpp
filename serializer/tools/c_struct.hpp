#ifndef C_STRUCT_H
#define C_STRUCT_H

namespace serializer::tools {

template <typename T>
struct CStruct {
  CStruct(T &element) : element(element) {}
  T &element;
};

} // end namespace serializer::tools

#endif
