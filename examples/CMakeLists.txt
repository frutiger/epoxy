add_executable(
    crude-example
    crude-example.m.cpp
)

target_compile_features(
    crude-example PUBLIC
    cxx_std_17
)

target_compile_options(
    crude-example PRIVATE
    -Wall -Wextra -Wpedantic -Werror -Wdeprecated
)

target_link_libraries(
    crude-example
    crude
)

