// Copyright 2022 The IREE Authors
//
// Licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "iree/compiler/Pipelines/Pipelines.h"

#include "iree/compiler/Bindings/Native/Transforms/Passes.h"
#include "iree/compiler/Bindings/TFLite/Transforms/Passes.h"
#include "iree/compiler/Dialect/Flow/Transforms/Passes.h"
#include "iree/compiler/Dialect/HAL/Transforms/Passes.h"
#include "iree/compiler/Dialect/Stream/Transforms/Passes.h"
#include "iree/compiler/Dialect/Util/Transforms/Passes.h"
#include "iree/compiler/Dialect/VM/Transforms/Passes.h"
#include "iree/compiler/InputConversion/Common/Passes.h"
#include "iree/compiler/Modules/HAL/Inline/Transforms/Passes.h"
#include "iree/compiler/Modules/HAL/Loader/Transforms/Passes.h"

#ifdef IREE_HAVE_MHLO_INPUT
#include "iree/compiler/InputConversion/MHLO/Passes.h"
#endif  // IREE_HAVE_MHLO_INPUT
#ifdef IREE_HAVE_TORCH_INPUT
#include "iree/compiler/InputConversion/TMTensor/Passes.h"
#endif  // IREE_HAVE_TORCH_INPUT
#ifdef IREE_HAVE_TOSA_INPUT
#include "iree/compiler/InputConversion/TOSA/Passes.h"
#endif  // IREE_HAVE_TOSA_INPUT

namespace mlir {
namespace iree_compiler {

void buildIREEVMTransformPassPipeline(
    BindingOptions bindingOptions, InputDialectOptions inputOptions,
    HighLevelOptimizationOptions highLevelOptimizationOptions,
    SchedulingOptions schedulingOptions,
    IREE::HAL::TargetOptions executableOptions,
    IREE::VM::TargetOptions targetOptions, IREEVMPipelineHooks &hooks,
    OpPassManager &passManager) {
  // Input pipelines can result in changes to the exported functions and types
  // and must run before generating bindings.
  // After input processing, there should only be IREE legal types in
  // signatures.
  switch (inputOptions.type) {
    case InputDialectOptions::Type::none:
      break;
#ifdef IREE_HAVE_MHLO_INPUT
    case InputDialectOptions::Type::mhlo:
      MHLO::buildMHLOInputConversionPassPipeline(passManager);
      break;
    case InputDialectOptions::Type::xla:
      MHLO::buildXLACleanupPassPipeline(passManager);
      MHLO::buildMHLOInputConversionPassPipeline(passManager);
      break;
#endif  // IREE_HAVE_MHLO_INPUT
#ifdef IREE_HAVE_TORCH_INPUT
    case InputDialectOptions::Type::tm_tensor:
      passManager.addNestedPass<func::FuncOp>(
          TMTensor::createConvertTMTensorToLinalgExtPass());
      break;
#endif  // IREE_HAVE_TORCH_INPUT
#ifdef IREE_HAVE_TOSA_INPUT
    case InputDialectOptions::Type::tosa:
      buildTOSAInputConversionPassPipeline(passManager);
      break;
#endif  // IREE_HAVE_TOSA_INPUT
  }

  buildCommonInputConversionPassPipeline(passManager);

  // Now that inputs are legalized, generate wrapper for entry functions.
  if (bindingOptions.native) {
    // TODO(benvanik): pass down execution model to the ABI pipeline so that
    // it can change default function signature behavior
    IREE::ABI::buildTransformPassPipeline(passManager);
  }
  if (bindingOptions.tflite) {
    IREE::TFLite::buildTransformPassPipeline(passManager);
  }

  IREE::Flow::TransformOptions flowOptions;
  flowOptions.constExprHoisting =
      highLevelOptimizationOptions.constExprHoisting;
  flowOptions.numericPrecisionReduction =
      highLevelOptimizationOptions.numericPrecisionReduction;

  // Enable const-eval via hook. For debug builds, we assert if enabled without
  // a hook. For release, we just silently skip enabling const-eval.
  if (highLevelOptimizationOptions.constEval) {
    assert(hooks.buildConstEvalPassPipelineCallback &&
           "if const-eval is enabled the buildConstEvalPassPipelineCallback "
           "hook must be enabled");
  }
  if (highLevelOptimizationOptions.constEval &&
      hooks.buildConstEvalPassPipelineCallback) {
    flowOptions.buildConstEvalPassPipeline =
        hooks.buildConstEvalPassPipelineCallback;
  }

  if (highLevelOptimizationOptions.stripAssertions) {
    // Strip std.assert & co after we perform optimizations; prior to this we
    // may use the assertions to derive information during analysis.
    passManager.addPass(IREE::Util::createStripDebugOpsPass());
  }

  IREE::Stream::TransformOptions streamOptions;
  // TODO(benvanik): find a way to share the enums w/o circular deps.
  streamOptions.dumpStatisticsFormat =
      (IREE::Stream::DumpOutputFormat)schedulingOptions.dumpStatisticsFormat;
  streamOptions.dumpStatisticsFile = schedulingOptions.dumpStatisticsFile;

  switch (schedulingOptions.executionModel) {
    case SchedulingOptions::ExecutionModel::HostOnly:
      // No flow/stream processing (implies no tensors).
      break;
    default:
      IREE::Flow::buildFlowTransformPassPipeline(passManager, flowOptions);
      IREE::Stream::buildStreamTransformPassPipeline(passManager,
                                                     streamOptions);
      break;
  }

  switch (schedulingOptions.executionModel) {
    case SchedulingOptions::ExecutionModel::HostOnly:
      // No HAL required.
      break;
    default:
    case SchedulingOptions::ExecutionModel::AsyncInternal:
    case SchedulingOptions::ExecutionModel::AsyncExternal:
      IREE::HAL::buildHALTransformPassPipeline(passManager, executableOptions);
      break;
    case SchedulingOptions::ExecutionModel::InlineStatic:
      IREE::HAL::Inline::buildHALInlineStaticTransformPassPipeline(
          passManager, executableOptions);
      break;
    case SchedulingOptions::ExecutionModel::InlineDynamic:
      IREE::HAL::Loader::buildHALInlineDynamicTransformPassPipeline(
          passManager, executableOptions);
      break;
  }

  IREE::VM::buildVMTransformPassPipeline(passManager, targetOptions);
  passManager.addPass(IREE::Util::createDropCompilerHintsPass());
}

void buildDefaultIREEVMTransformPassPipeline(OpPassManager &passManager) {
  // Note that the production compiler will provide hooks here that enable
  // additional, whole-program related features, whereas this pipeline will
  // only use the defaults. In practice, this means that things like const
  // jitting are not supported by this pipeline.
  static IREEVMPipelineHooks defaultHooks;

  buildIREEVMTransformPassPipeline(
      BindingOptions::FromFlags::get(), InputDialectOptions::FromFlags::get(),
      HighLevelOptimizationOptions::FromFlags::get(),
      SchedulingOptions::FromFlags::get(),
      IREE::HAL::TargetOptions::FromFlags::get(),
      IREE::VM::TargetOptions::FromFlags::get(), defaultHooks, passManager);
}

void registerIREEVMTransformPassPipeline() {
  PassPipelineRegistration<> transformPassPipeline(
      "iree-transformation-pipeline",
      "Runs the full IREE input to VM transformation pipeline",
      [](OpPassManager &passManager) {
        buildDefaultIREEVMTransformPassPipeline(passManager);
      });
}

}  // namespace iree_compiler
}  // namespace mlir
