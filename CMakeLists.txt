cmake_minimum_required(VERSION 3.20)

project(EasyDocsFinder C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # 导出compile_commands.json文件，方便代码分析工具使用

add_executable(easydocsfinder
    src/main.c
    src/search.c
)

target_include_directories(easydocsfinder PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 以下为测试
enable_testing()

# 测试解析参数（简化版）
# 1) success: 默认参数 + 只看退出码
add_test(NAME parse_ok_default
    COMMAND $<TARGET_FILE:easydocsfinder> .
)

# 2) success: --pattern=... + 只看退出码
add_test(NAME parse_ok_pattern_max
    COMMAND $<TARGET_FILE:easydocsfinder> . --pattern testpattern --max-results 10
)

# 3) failure: 没有 roots，应返回 2
add_test(NAME parse_fail_no_roots
    COMMAND $<TARGET_FILE:easydocsfinder>
)
set_tests_properties(parse_fail_no_roots PROPERTIES
    WILL_FAIL TRUE
)

# 测试基础的搜索功能
# 1) 能搜索到结果
add_test(NAME search_results
  COMMAND ${CMAKE_COMMAND} -Dexe=$<TARGET_FILE:easydocsfinder> -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_search_results.cmake
)

# 2) 搜索不到结果
add_test(NAME search_no_results
  COMMAND ${CMAKE_COMMAND} -Dexe=$<TARGET_FILE:easydocsfinder> -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_search_no_results.cmake
)

# 3) 达到最大搜索数量就停止
add_test(NAME search_max_results
  COMMAND ${CMAKE_COMMAND} -Dexe=$<TARGET_FILE:easydocsfinder> -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_search_max_results.cmake
)

# 4) 忽略整个目录
add_test(NAME ignore_dir_exclude
  COMMAND ${CMAKE_COMMAND} -Dexe=$<TARGET_FILE:easydocsfinder> -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_ignore_dir_exclude.cmake
)
