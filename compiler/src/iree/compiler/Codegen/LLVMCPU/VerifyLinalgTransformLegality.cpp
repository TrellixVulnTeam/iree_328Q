// Copyright 2022 The IREE Authors
//
// Licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "iree/compiler/Codegen/PassDetail.h"
#include "iree/compiler/Codegen/Passes.h"
#include "iree/compiler/Codegen/Utils/MarkerUtils.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/Linalg/Transforms/Transforms.h"
#include "mlir/Pass/Pass.h"

namespace mlir {
namespace iree_compiler {

namespace {
struct VerifyLinalgTransformLegalityPass
    : VerifyLinalgTransformLegalityBase<VerifyLinalgTransformLegalityPass> {
  void runOnOperation() override;
};
}  // namespace

void VerifyLinalgTransformLegalityPass::runOnOperation() {
  auto moduleOp = getOperation();
  // For now only check that there are no Linalg transform markers.
  auto walkResult = moduleOp.walk([](linalg::LinalgOp op) -> WalkResult {
    if (op->hasAttr(linalg::LinalgTransforms::kLinalgTransformMarker)) {
      return op.emitError("expected no Linalg transform markers");
    }
    return WalkResult::advance();
  });
  if (walkResult.wasInterrupted()) {
    return signalPassFailure();
  }
}

std::unique_ptr<OperationPass<ModuleOp>>
createVerifyLinalgTransformLegalityPass() {
  return std::make_unique<VerifyLinalgTransformLegalityPass>();
}

}  // namespace iree_compiler
}  // namespace mlir
