cmake_minimum_required(VERSION 3.20)

if (NOT DEFINED exe OR "${exe}" STREQUAL "")
  message(FATAL_ERROR "Missing easydocsfinder executable path as -Dexe=...")
endif()

get_filename_component(project_root "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)
set(root "${project_root}/build/ctest_contains_hit_root")

file(REMOVE_RECURSE "${root}")
file(MAKE_DIRECTORY "${root}")

file(WRITE "${root}/a.txt" "hello\nworld\n")
file(WRITE "${root}/b.txt" "nope\n")
file(WRITE "${root}/c.c" "hello in c\n")

execute_process(
  COMMAND "${exe}" "${root}" -p "*.txt" --contains "hello"
  RESULT_VARIABLE rc
  OUTPUT_VARIABLE out
  ERROR_VARIABLE err
)

set(out_all "${out}\n${err}")

if (NOT rc EQUAL 0)
  file(REMOVE_RECURSE "${root}")
  message(FATAL_ERROR "contains_hit failed rc=${rc}\nstdout=${out}\nstderr=${err}")
endif()

# 期望：a.txt 被输出，且匹配行内容被输出；b.txt 不应出现
string(FIND "${out_all}" "a.txt" posa)
string(FIND "${out_all}" "hello" poshello)
string(FIND "${out_all}" "b.txt" posb)

file(REMOVE_RECURSE "${root}")

if (posa EQUAL -1 OR poshello EQUAL -1 OR NOT posb EQUAL -1)
  message(FATAL_ERROR "contains_hit assertion failed.\nOutput:\n${out_all}")
endif()
