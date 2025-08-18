#pragma once
#ifdef __cplusplus

#include <aidl/tinywl/BnTinywlInput.h>
namespace tinywl {

  class TinywlInputService : public aidl::tinywl::BnTinywlInput {};
}  // namespace tinywl

#endif
