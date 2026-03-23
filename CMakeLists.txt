cmake_minimum_required(VERSION 3.20)

project(EasyDocsFinder C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(easydocsfinder
    src/main.c
)

enable_testing()

# 测试解析参数（简化版）
# 1) success: 默认参数 + 检查输出
add_test(NAME parse_ok_default
    COMMAND $<TARGET_FILE:easydocsfinder> .
)
set_tests_properties(parse_ok_default PROPERTIES
    PASS_REGULAR_EXPRESSION "Parsed arguments:"
)

# 2) success: --pattern=... + 检查输出片段
add_test(NAME parse_ok_pattern_max
    COMMAND $<TARGET_FILE:easydocsfinder> . --pattern testpattern --max-results 10
)
set_tests_properties(parse_ok_pattern_max PROPERTIES
    PASS_REGULAR_EXPRESSION "pattern=testpattern"
)

# 3) failure: 没有 roots，应返回 2
add_test(NAME parse_fail_no_roots
    COMMAND $<TARGET_FILE:easydocsfinder>
)
set_tests_properties(parse_fail_no_roots PROPERTIES
    WILL_FAIL TRUE
)
