cmake_minimum_required(VERSION 3.20)

if (NOT DEFINED exe OR "${exe}" STREQUAL "")
  message(FATAL_ERROR "Missing easydocsfinder executable path as -Dexe=...")
endif()

get_filename_component(project_root "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)
set(root "${project_root}/build/ctest_contains_miss_root")

file(REMOVE_RECURSE "${root}")
file(MAKE_DIRECTORY "${root}")

file(WRITE "${root}/a.txt" "abc\n")
file(WRITE "${root}/b.txt" "def\n")

execute_process(
  COMMAND "${exe}" "${root}" -p "*.txt" --contains "hello"
  RESULT_VARIABLE rc
  OUTPUT_VARIABLE out
  ERROR_VARIABLE err
)

set(out_all "${out}\n${err}")

file(REMOVE_RECURSE "${root}")

if (NOT rc EQUAL 0)
  message(FATAL_ERROR "contains_miss failed rc=${rc}\nstdout=${out}\nstderr=${err}")
endif()

string(FIND "${out_all}" "No matching files found." posnone)
if (posnone EQUAL -1)
  message(FATAL_ERROR "Expected 'No matching files found.'\nOutput:\n${out_all}")
endif()
