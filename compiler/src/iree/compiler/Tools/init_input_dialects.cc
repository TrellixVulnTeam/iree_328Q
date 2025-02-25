// Copyright 2022 The IREE Authors
//
// Licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "iree/compiler/Tools/init_input_dialects.h"

#ifdef IREE_HAVE_MHLO_INPUT
#include "mlir-hlo/Dialect/mhlo/IR/hlo_ops.h"
#include "stablehlo/dialect/ChloOps.h"
#endif  // IREE_HAVE_MHLO_INPUT
#ifdef IREE_HAVE_TORCH_INPUT
#include "torch-mlir-dialects/Dialect/TMTensor/IR/TMTensorDialect.h"
#endif
#ifdef IREE_HAVE_TOSA_INPUT
#include "mlir/Dialect/Tosa/IR/TosaOps.h"
#endif  // IREE_HAVE_TOSA_INPUT

namespace mlir {
namespace iree_compiler {

void registerInputDialects(DialectRegistry &registry) {
#ifdef IREE_HAVE_MHLO_INPUT
  registry.insert<mlir::chlo::ChloDialect, mlir::mhlo::MhloDialect>();
#endif  // IREE_HAVE_MHLO_INPUT
#ifdef IREE_HAVE_TORCH_INPUT
  registry.insert<mlir::torch::TMTensor::TMTensorDialect>();
#endif  // IREE_HAVE_TORCH_INPUT
#ifdef IREE_HAVE_TOSA_INPUT
  registry.insert<tosa::TosaDialect>();
#endif  // IREE_HAVE_TOSA_INPUT
}

}  // namespace iree_compiler
}  // namespace mlir
