# 测试 达到最大搜索数量就停止：创建 r1.c/r2.c/r3.c
# 运行 -p "*.c" --max-results 2
# 期望输出只包含 r1.c、r2.c，且不包含 r3.c
# 主要覆盖：--max-results 截断逻辑
cmake_minimum_required(VERSION 3.20)

if (NOT DEFINED exe OR "${exe}" STREQUAL "")
  message(FATAL_ERROR "Missing easydocsfinder executable path as -Dexe=...")
endif()

get_filename_component(project_root "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)
set(root "${project_root}/build/ctest_search_max_results_root")

file(REMOVE_RECURSE "${root}")
file(MAKE_DIRECTORY "${root}")

file(WRITE "${root}/r1.c" "int a=1;\n")
file(WRITE "${root}/r2.c" "int a=2;\n")
file(WRITE "${root}/r3.c" "int a=3;\n")

execute_process(
  COMMAND "${exe}" "${root}" -p "*.c" --max-results 2
  RESULT_VARIABLE rc
  OUTPUT_VARIABLE out
  ERROR_VARIABLE err
)

set(failed FALSE)
if (NOT rc EQUAL 0)
  set(failed TRUE)
endif()

# 统计 stdout 中以 .c 结尾的行（每个 puts() 一行）
string(REGEX MATCHALL "[^\r\n]+\\.c" found_files "${out}")
list(LENGTH found_files nfound)

if (NOT nfound EQUAL 2)
  set(failed TRUE)
endif()

string(FIND "${out}" "r1.c" p1)
string(FIND "${out}" "r2.c" p2)
string(FIND "${out}" "r3.c" p3)

if (p1 EQUAL -1 OR p2 EQUAL -1)
  set(failed TRUE)
endif()

# max=2 时，r3.c 不应出现
if (NOT p3 EQUAL -1)
  set(failed TRUE)
endif()

file(REMOVE_RECURSE "${root}")

if (failed)
  message(FATAL_ERROR "test_search_max_results failed.\nrc=${rc}\nstdout=${out}\nstderr=${err}")
endif()
