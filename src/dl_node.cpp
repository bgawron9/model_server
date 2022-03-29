//*****************************************************************************
// Copyright 2020 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//*****************************************************************************
#include "dl_node.hpp"

#include <map>
#include <utility>

#include "dlnodesession.hpp"
#include "logging.hpp"
#include "modelmanager.hpp"
#include "ov_utils.hpp"
#include "ovinferrequestsqueue.hpp"
#include "prediction_service_utils.hpp"
#include "timer.hpp"

#include "profiler.hpp"

namespace ovms {

const uint WAIT_FOR_STREAM_ID_TIMEOUT_MICROSECONDS = 1;

Status DLNode::execute(session_key_t sessionKey, PipelineEventQueue& notifyEndQueue) {
    OVMS_PROFILE_FUNCTION();
    auto& nodeSession = getNodeSession(sessionKey);
    auto& dlNodeSession = static_cast<DLNodeSession&>(nodeSession);
    return dlNodeSession.execute(notifyEndQueue, WAIT_FOR_STREAM_ID_TIMEOUT_MICROSECONDS, *this);
}

Status DLNode::fetchResults(NodeSession& nodeSession, SessionResults& nodeSessionOutputs) {
    auto& dlNodeSession = static_cast<DLNodeSession&>(nodeSession);
    const auto& sessionMetadata = nodeSession.getNodeSessionMetadata();
    SessionResult sessionResults{sessionMetadata, {}};
    auto it = nodeSessionOutputs.emplace(sessionMetadata.getSessionKey(), std::move(sessionResults));
    if (!it.second) {
        SPDLOG_LOGGER_ERROR(dag_executor_logger, "Failed to put node: {} session: {} results in node session outputs",
            getName(), nodeSession.getSessionKey());
        return StatusCode::INTERNAL_ERROR;
    }
    auto& metadataTensorResultsPair = it.first->second;
    auto& tensorResults = metadataTensorResultsPair.second;
    Status status;
    const uint waitTimeMicroseconds = 1;
    auto& inferRequest = dlNodeSession.getInferRequest(waitTimeMicroseconds);
    auto& model = dlNodeSession.getModelInstance();
    status = this->fetchResults(tensorResults, inferRequest, model, nodeSession.getSessionKey());
    return status;
}

Status DLNode::fetchResults(TensorMap& outputs, ov::InferRequest& inferRequest, ModelInstance& model, session_key_t sessionKey) {
    ReleaseSessionGuard releaseSessionGuard(this->getNodeSession(sessionKey));
    // Wait for tensor results
    SPDLOG_LOGGER_DEBUG(dag_executor_logger, "Node: {} session: {} Waiting for infer request to finish", getName(), sessionKey);
    try {
        inferRequest.wait();
    } catch (const ov::Exception& e) {
        SPDLOG_LOGGER_ERROR(dag_executor_logger, "Node: {} session: {} IE exception occured during infer request wait: {}", getName(), sessionKey, e.what());
        return StatusCode::INTERNAL_ERROR;
    } catch (std::exception& e) {
        SPDLOG_LOGGER_ERROR(dag_executor_logger, "Node: {} session: {} exception occured during infer request wait: {}", getName(), sessionKey, e.what());
        return StatusCode::INTERNAL_ERROR;
    }
    SPDLOG_LOGGER_DEBUG(dag_executor_logger, "Node: {} session: {} infer request finished", getName(), sessionKey);
    SPDLOG_LOGGER_DEBUG(dag_executor_logger, "Inference processing time for node {}; model name: {}; session: {} - {} ms",
        this->getName(),
        model.getName(),
        sessionKey,
        this->getNodeSession(sessionKey).getTimer().elapsed<std::chrono::microseconds>("inference") / 1000);

    static_cast<DLNodeSession&>(this->getNodeSession(sessionKey)).clearInputs();

    // Fill outputs map with result tensors. Fetch only those that are required in following nodes.
    for (const auto& node : this->next) {
        for (const auto& pair : node.get().getMappingByDependency(*this)) {
            const auto& output_name = pair.first;
            if (outputs.find(output_name) != outputs.end()) {
                continue;
            }

            try {
                std::string realModelOutputName;
                if (!getRealOutputName(model, output_name, &realModelOutputName).ok()) {
                    SPDLOG_LOGGER_WARN(dag_executor_logger, "Node: {} session: {} Cannot find real model output name for alias: {}", getName(), sessionKey, output_name);
                    return StatusCode::INTERNAL_ERROR;
                }
                SPDLOG_LOGGER_DEBUG(dag_executor_logger, "Node: {} session: {} Getting tensor from model: {}, inferRequestStreamId: {}, tensorName: {}",
                    getName(), sessionKey, modelName, sessionKey, realModelOutputName);
                OVMS_PROFILE_SYNC_BEGIN("ov::InferRequest::get_tensor");
                const auto tensor = inferRequest.get_tensor(realModelOutputName);
                OVMS_PROFILE_SYNC_END("ov::InferRequest::get_tensor");
                SPDLOG_LOGGER_DEBUG(dag_executor_logger, "Node: {} session: {} Creating copy of tensor from model: {}, tensorName: {}",
                    getName(), sessionKey, modelName, realModelOutputName);
                // ov::Tensor copiedTensor;
                // auto status = tensorClone(copiedTensor, tensor);
                // if (!status.ok()) {
                //     SPDLOG_LOGGER_DEBUG(dag_executor_logger, "Could not clone result tensor; node: {}; session: {}; model name: {}; output: {}",
                //         getName(),
                //         this->modelName,
                //         realModelOutputName);
                //     return status;
                // }
                outputs.emplace(std::make_pair(output_name, std::move(tensor)));
            } catch (const ov::Exception& e) {
                Status status = StatusCode::OV_INTERNAL_SERIALIZATION_ERROR;
                SPDLOG_LOGGER_DEBUG(dag_executor_logger, "Node: {} session:{} Error during getting tensor {}; exception message: {}", getName(), sessionKey, status.string(), e.what());
                return status;
            }
            SPDLOG_LOGGER_DEBUG(dag_executor_logger, "Node: {} session: {} Tensor with name {} has been prepared", getName(), sessionKey, output_name);
        }
    }
    return StatusCode::OK;
}

void DLNode::release(session_key_t sessionId) {
    SPDLOG_LOGGER_DEBUG(dag_executor_logger, "Release node: {} sessionKey: {}", getName(), sessionId);
    getNodeSession(sessionId).release();
}
bool DLNode::tryDisarm(const session_key_t& sessionKey, const uint microseconds) {
    return getNodeSession(sessionKey).tryDisarm(microseconds);
}

std::unique_ptr<NodeSession> DLNode::createNodeSession(const NodeSessionMetadata& metadata, const CollapseDetails& collapsingDetails) {
    return std::make_unique<DLNodeSession>(metadata, getName(), previous.size(), collapsingDetails,
        this->modelManager, this->modelName, this->modelVersion.value_or(0));
}

}  // namespace ovms
