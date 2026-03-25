# 测试 忽略整个目录：创建 keep_dir/ok.c 和 ignored_dir/skip.c
# 运行 -p "*.c" --ignore ignored_dir
# 期望只输出 ok.c，不输出 skip.c
# 主要覆盖：目录级 --ignore（递归跳过目录）是否生效
cmake_minimum_required(VERSION 3.20)

if (NOT DEFINED exe OR "${exe}" STREQUAL "")
  message(FATAL_ERROR "Missing easydocsfinder executable path as -Dexe=...")
endif()

get_filename_component(project_root "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)
set(root "${project_root}/build/ctest_ignore_dir_exclude_root")

file(REMOVE_RECURSE "${root}")
file(MAKE_DIRECTORY "${root}")
file(MAKE_DIRECTORY "${root}/keep_dir")
file(MAKE_DIRECTORY "${root}/ignored_dir")

file(WRITE "${root}/keep_dir/ok.c" "int ok=1;\n")
file(WRITE "${root}/ignored_dir/skip.c" "int skip=1;\n")

execute_process(
  COMMAND "${exe}" "${root}" -p "*.c" --ignore ignored_dir
  RESULT_VARIABLE rc
  OUTPUT_VARIABLE out
  ERROR_VARIABLE err
)

set(out_all "${out}\n${err}")
set(failed FALSE)

if (NOT rc EQUAL 0)
  set(failed TRUE)
endif()

string(FIND "${out_all}" "ok.c" pos_ok)
if (pos_ok EQUAL -1)
  set(failed TRUE)
endif()

string(FIND "${out_all}" "skip.c" pos_skip)
if (NOT pos_skip EQUAL -1)
  set(failed TRUE)
endif()

file(REMOVE_RECURSE "${root}")

if (failed)
  message(FATAL_ERROR "test_ignore_dir_exclude failed.\nrc=${rc}\nstdout=${out}\nstderr=${err}")
endif()
