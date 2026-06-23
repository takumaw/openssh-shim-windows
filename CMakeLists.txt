cmake_minimum_required(VERSION 3.21)

project(openssh_shim_windows LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(shim_core STATIC
  src/shim_core.c
)

target_include_directories(shim_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(shim_core PRIVATE
  UNICODE
  _UNICODE
  WIN32_LEAN_AND_MEAN
  NOMINMAX
)

add_executable(ssh
  src/ssh_main.c
)

target_link_libraries(ssh PRIVATE shim_core)
set_target_properties(ssh PROPERTIES OUTPUT_NAME "ssh")

add_executable(scp
  src/scp_main.c
)

target_link_libraries(scp PRIVATE shim_core)
set_target_properties(scp PROPERTIES OUTPUT_NAME "scp")

foreach(target IN ITEMS ssh scp)
  target_compile_definitions(${target} PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
  )
endforeach()

if(MSVC)
  foreach(target IN ITEMS shim_core ssh scp)
    target_compile_options(${target} PRIVATE
      /std:c17
      /W4
      /WX
      /guard:cf
    )
  endforeach()

  foreach(target IN ITEMS ssh scp)
    target_link_options(${target} PRIVATE
      /guard:cf
      /DYNAMICBASE
      /NXCOMPAT
      /OPT:REF
      /OPT:ICF
    )
  endforeach()

  set_property(TARGET shim_core ssh scp PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
  )
else()
  message(FATAL_ERROR "This project currently supports MSVC only.")
endif()
