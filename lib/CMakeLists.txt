add_library(Encryption SHARED Encryption.cpp)

target_include_directories(
  Encryption PUBLIC "${CMAKE_PLUGIN_INCLUDE_DIRECTORY}")

target_link_libraries(Encryption
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")