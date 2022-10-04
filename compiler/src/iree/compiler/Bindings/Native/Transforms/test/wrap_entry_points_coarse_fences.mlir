// RUN: iree-opt --pass-pipeline='iree-abi-wrap-entry-points{invocation-model=coarse-fences}' --split-input-file %s | FileCheck %s

// CHECK-LABEL: func.func @asyncEntry(
//  CHECK-SAME:   %[[ARG0:.+]]: !hal.buffer_view, %[[ARG1:.+]]: !hal.buffer_view, %[[WAIT:.+]]: !hal.fence, %[[SIGNAL:.+]]: !hal.fence
//  CHECK-SAME: -> (
//  CHECK-SAME:   !hal.buffer_view, !hal.buffer_view
//  CHECK-SAME: ) attributes {
//  CHECK-SAME:   iree.abi.stub
//  CHECK-SAME:   iree.reflection = {iree.abi.model = "coarse-fences"}
//  CHECK-SAME: } {
//  CHECK-NEXT:   %[[ARG0_TENSOR:.+]] = hal.tensor.import wait(%[[WAIT]]) => %[[ARG0]] : !hal.buffer_view -> tensor<4xf32>
//  CHECK-NEXT:   %[[ARG1_TENSOR:.+]] = hal.tensor.import wait(%[[WAIT]]) => %[[ARG1]] : !hal.buffer_view -> tensor<4xf32>
//  CHECK-NEXT:   %[[RESULT_TENSORS:.+]]:2 = call @_asyncEntry(%[[ARG0_TENSOR]], %[[ARG1_TENSOR]])
//  CHECK-NEXT:   %[[READY_TENSORS:.+]]:2 = hal.tensor.barrier join(%[[RESULT_TENSORS]]#0, %[[RESULT_TENSORS]]#1 : tensor<4xf32>, tensor<4xf32>) => %[[SIGNAL]] : !hal.fence
//  CHECK-NEXT:   %[[RET0_VIEW:.+]] = hal.tensor.export %[[READY_TENSORS]]#0 : tensor<4xf32> -> !hal.buffer_view
//  CHECK-NEXT:   %[[RET1_VIEW:.+]] = hal.tensor.export %[[READY_TENSORS]]#1 : tensor<4xf32> -> !hal.buffer_view
//  CHECK-NEXT:   return %[[RET0_VIEW]], %[[RET1_VIEW]] : !hal.buffer_view, !hal.buffer_view
//  CHECK-NEXT: }

// CHECK-LABEL: func.func private @_asyncEntry(
func.func @asyncEntry(%arg0: tensor<4xf32>, %arg1: tensor<4xf32>) -> (tensor<4xf32>, tensor<4xf32>) {
  %0 = arith.addf %arg0, %arg1 : tensor<4xf32>
  %1 = arith.addf %0, %arg0 : tensor<4xf32>
  return %0, %1 : tensor<4xf32>, tensor<4xf32>
}
