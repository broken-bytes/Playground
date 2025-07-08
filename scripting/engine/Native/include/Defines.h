#if defined(__clang__)
  #define SWIFT_NAME(name) __attribute__((swift_name(name)))
#else
  #define SWIFT_NAME(name)
#endif
