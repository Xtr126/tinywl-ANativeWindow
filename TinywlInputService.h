#pragma once
#ifdef __cplusplus

#include <aidl/tinywl/BnTinywlInput.h>

extern "C" {
  #include "tinywl.h"
}

namespace tinywl {

  class TinywlInputService : public aidl::tinywl::BnTinywlInput {
    public:
      int32_t width;
      int32_t height;
      void setTinywlServer(struct tinywl_server* server);
  };
}  // namespace tinywl

#endif
