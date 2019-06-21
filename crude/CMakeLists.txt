add_library(
    crude
    crude_assert.cpp
    crude_context.cpp
    crude_convert.cpp
    crude_exchanger.cpp
    crude_runtime.cpp
    crude_valueutil.cpp
    crude_wrapper.cpp
)

target_compile_features(
    crude PUBLIC
    cxx_std_17
)

target_compile_options(
    crude PRIVATE
    -Wall -Wextra -Wpedantic -Werror -Wdeprecated
)

target_include_directories(
    crude PUBLIC
    .
)

set(V8_INCLUDE_DIR $ENV{V8_DIR}/include CACHE PATH "V8 include directory")
if (NOT $ENV{V8_LIB} STREQUAL "")
    set(V8_LIB_DIR $ENV{V8_DIR}/$ENV{V8_LIB} CACHE PATH "V8 libraries")
else()
    set(V8_LIB_DIR $ENV{V8_DIR}/lib CACHE PATH "V8 libraries")
endif()

target_include_directories (
    crude SYSTEM PUBLIC
    ${V8_INCLUDE_DIR}
)

target_link_libraries(
    crude
    -L${V8_LIB_DIR}
    -licuuc
    -lv8_libbase
    -licui18n
    -lv8
    -lv8_libplatform
)

