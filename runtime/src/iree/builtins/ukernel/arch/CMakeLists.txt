# Copyright 2022 The IREE Authors
#
# Licensed under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

option(IREE_UKERNEL_FORCE_DISABLE_ARCH_SPECIFIC_CODE "Disable all architecture-specific code in builtin kernels" OFF)

if(IREE_UKERNEL_FORCE_DISABLE_ARCH_SPECIFIC_CODE)
  set(IREE_UKERNEL_ENABLE_ARCH_SPECIFIC_CODE FALSE)
else()
  set(IREE_UKERNEL_ENABLE_ARCH_SPECIFIC_CODE TRUE)
endif()

# This block is borrowed from boringssl's CMake code here:
# https://boringssl.googlesource.com/boringssl/+/c5f0e58e653d2d9afa8facc090ce09f8aaa3fa0d/CMakeLists.txt#43
if(CMAKE_OSX_ARCHITECTURES)
  list(LENGTH CMAKE_OSX_ARCHITECTURES NUM_ARCHES)
  if(NOT ${NUM_ARCHES} EQUAL 1)
    message(WARNING "Performance advisory: architecture-specific code paths disabled because this is a multi-architecture build.")
    set(IREE_UKERNEL_ENABLE_ARCH_SPECIFIC_CODE FALSE)
  endif()
  list(GET CMAKE_OSX_ARCHITECTURES 0 CMAKE_SYSTEM_PROCESSOR)
endif()

if(IREE_UKERNEL_ENABLE_ARCH_SPECIFIC_CODE)
  if((CMAKE_SYSTEM_PROCESSOR STREQUAL aarch64) OR (CMAKE_SYSTEM_PROCESSOR STREQUAL arm64))
    set(IREE_UKERNEL_ARCH_ARM_64 TRUE)
    add_subdirectory(arm_64)
    list(APPEND IREE_UKERNEL_ARCH_DEPS "iree::builtins::ukernel::arch::arm_64::mmt4d_select_tile_arm_64")
  endif()
endif()  # IREE_UKERNEL_ENABLE_ARCH_SPECIFIC_CODE

set(IREE_UKERNEL_POINTER_SIZE "${CMAKE_SIZEOF_VOID_P}")

iree_cc_library(
  NAME
    config
  HDRS
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

iree_cc_library(
  NAME
    ukernel_arch
  HDRS
    "mmt4d_select_tile_arch.h"
  SRCS
    "mmt4d_select_tile_arch.c"
  DEPS
    iree::builtins::ukernel::types
    ${IREE_UKERNEL_ARCH_DEPS}
  PUBLIC
)

configure_file(config.h.in config.h)
