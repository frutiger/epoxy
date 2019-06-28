add_library(
    epoxy
    epoxy_adapter.cpp
)

target_compile_features(
    epoxy PUBLIC
    cxx_std_17
)

target_compile_options(
    epoxy PRIVATE
    -Wall -Wextra -Wpedantic -Werror -Wdeprecated
)

target_include_directories(
    epoxy PUBLIC
    .
)

