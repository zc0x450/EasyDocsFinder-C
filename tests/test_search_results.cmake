# 测试 能搜索到结果：在根目录创建 only1.c、子目录创建 sub/only2.c、再创建一个 note.txt
# 运行 -p "*.c" 后期望输出里包含 only1.c 和 only2.c
# 主要覆盖：pattern 文件名匹配 + 递归遍历 + 能输出匹配路径
cmake_minimum_required(VERSION 3.20)

if (NOT DEFINED exe OR "${exe}" STREQUAL "")
  message(FATAL_ERROR "Missing easydocsfinder executable path as -Dexe=...")
endif()

get_filename_component(project_root "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)
set(root "${project_root}/build/ctest_search_results_root")

file(REMOVE_RECURSE "${root}")
file(MAKE_DIRECTORY "${root}")
file(MAKE_DIRECTORY "${root}/sub")

file(WRITE "${root}/only1.c" "int x=1;\n")
file(WRITE "${root}/sub/only2.c" "int y=2;\n")
file(WRITE "${root}/note.txt" "hello\n")

execute_process(
  COMMAND "${exe}" "${root}" -p "*.c"
  RESULT_VARIABLE rc
  OUTPUT_VARIABLE out
  ERROR_VARIABLE err
)

set(out_all "${out}\n${err}")
set(failed FALSE)

if (NOT rc EQUAL 0)
  set(failed TRUE)
endif()

string(FIND "${out_all}" "only1.c" pos1)
if (pos1 EQUAL -1)
  set(failed TRUE)
endif()

string(FIND "${out_all}" "only2.c" pos2)
if (pos2 EQUAL -1)
  set(failed TRUE)
endif()

file(REMOVE_RECURSE "${root}")

if (failed)
  message(FATAL_ERROR "test_search_results failed.\nrc=${rc}\nstdout=${out}\nstderr=${err}")
endif()
