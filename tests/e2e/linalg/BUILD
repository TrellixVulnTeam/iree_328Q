# Copyright 2022 The IREE Authors
#
# Licensed under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

# Tests of end-to-end IREE support for individual ops in the TOSA dialect.
# Each test file should have a name matching the corresponding TOSA op and test only the
# functionality of that op (though may make use of other ops where necessary). Tests should be
# written using the IREE Check framework.
# See https://github.com/iree-org/iree/blob/main/docs/developers/developing_iree/testing_guide.md#iree-core-end-to-end-tests.

load("//build_tools/bazel:enforce_glob.bzl", "enforce_glob")
load("//build_tools/bazel:iree_check_test.bzl", "iree_check_single_backend_test_suite")

package(
    features = ["layering_check"],
    licenses = ["notice"],  # Apache 2.0
)

LLVM_SRCS = enforce_glob(
    [
        "conv2d.mlir",
    ],
    include = ["*.mlir"],
)

iree_check_single_backend_test_suite(
    name = "check_llvm-cpu_local-task",
    srcs = LLVM_SRCS,
    compiler_flags = ["--iree-input-type=none"],
    driver = "local-task",
    target_backend = "llvm-cpu",
)

VMVX_SRCS = enforce_glob(
    [
        "conv2d.mlir",
    ],
    include = ["*.mlir"],
)

iree_check_single_backend_test_suite(
    name = "check_vmvx_local-task",
    srcs = VMVX_SRCS,
    compiler_flags = [
        "--iree-input-type=none",
    ],
    driver = "local-task",
    target_backend = "vmvx",
)

VULKAN_SRCS = enforce_glob(
    [
        "conv2d.mlir",
    ],
    include = ["*.mlir"],
)

iree_check_single_backend_test_suite(
    name = "check_vulkan-spirv_vulkan",
    srcs = VULKAN_SRCS,
    compiler_flags = ["--iree-input-type=none"],
    driver = "vulkan",
    target_backend = "vulkan-spirv",
)

test_suite(
    name = "check",
    tests = [
        ":check_llvm-cpu_local-task",
        ":check_vmvx_local-task",
        ":check_vulkan-spirv_vulkan",
    ],
)
