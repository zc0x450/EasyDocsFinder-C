# 测试 搜索不到结果：创建 a.c 和 b.txt，但运行时用 -p "*.md"
# 期望输出包含 No matching files found.
# 主要覆盖：未匹配时的“无结果提示”分支
cmake_minimum_required(VERSION 3.20)

if (NOT DEFINED exe OR "${exe}" STREQUAL "")
  message(FATAL_ERROR "Missing easydocsfinder executable path as -Dexe=...")
endif()

get_filename_component(project_root "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)
set(root "${project_root}/build/ctest_search_no_results_root")

file(REMOVE_RECURSE "${root}")
file(MAKE_DIRECTORY "${root}")

file(WRITE "${root}/a.c" "int a=0;\n")
file(WRITE "${root}/b.txt" "b\n")

execute_process(
  COMMAND "${exe}" "${root}" -p "*.md"
  RESULT_VARIABLE rc
  OUTPUT_VARIABLE out
  ERROR_VARIABLE err
)

set(out_all "${out}\n${err}")
set(failed FALSE)

if (NOT rc EQUAL 0)
  set(failed TRUE)
endif()

string(FIND "${out_all}" "No matching files found." pos)
if (pos EQUAL -1)
  set(failed TRUE)
endif()

file(REMOVE_RECURSE "${root}")

if (failed)
  message(FATAL_ERROR "test_search_no_results failed.\nrc=${rc}\nstdout=${out}\nstderr=${err}")
endif()
