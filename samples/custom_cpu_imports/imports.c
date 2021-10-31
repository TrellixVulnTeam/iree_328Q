// Copyright 2022 The IREE Authors
//
// Licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <stdio.h>

#include "iree/base/api.h"
#include "iree/hal/local/executable_loader.h"

static int iree_debug_print_cstring(void* context, void* params,
                                    void* reserved) {
  const char* string = (const char*)params;
  fprintf(stdout, "%p: %s", context, string);
  fflush(stdout);
  return 0;
}

static iree_status_t iree_samples_custom_cpu_import_provider_resolve(
    void* self, iree_string_view_t symbol_name, void** out_fn_ptr,
    void** out_fn_context) {
  if (iree_string_view_equal(symbol_name,
                             IREE_SV("iree_debug_print_cstring"))) {
    *out_fn_ptr = iree_debug_print_cstring;
    *out_fn_context = 123;
    return iree_ok_status();
  }
  return iree_status_from_code(IREE_STATUS_NOT_FOUND);
}

iree_hal_executable_import_provider_t iree_samples_custom_cpu_import_provider(
    void) {
  iree_hal_executable_import_provider_t import_provider = {
      .self = NULL,
      .resolve = iree_samples_custom_cpu_import_provider_resolve,
  };
  return import_provider;
}
