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

#include <iostream>
#include <fstream>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../modelconfig.hpp"

using namespace testing;
using ::testing::UnorderedElementsAre;


TEST(ModelConfig, getters_setters) {
    ovms::ModelConfig config;

    config.setName("alexnet");
    auto name = config.getName();
    EXPECT_EQ(name, "alexnet");

    config.setBasePath("/path");
    auto path = config.getBasePath();
    EXPECT_EQ(path, "/path");

    config.setBackend("GPU");
    auto backend = config.getBackend();
    EXPECT_EQ(backend, "GPU");

    config.setBatchSize(5);
    auto batchSize = config.getBatchSize();
    EXPECT_EQ(batchSize, 5);

    config.setModelVersionPolicy("policy");
    auto policy = config.getModelVersionPolicy();
    EXPECT_EQ(policy, "policy");

    config.setNireq(11);
    auto nireq = config.getNireq();
    EXPECT_EQ(nireq, 11);

    ovms::model_version_t ver = 500;
    config.setVersion(ver);
    auto version = config.getVersion();
    EXPECT_EQ(version, ver);
}

TEST(ModelConfig, layout_single) {
    ovms::ModelConfig config;

    config.setLayout("NHWC");
    auto l1 = config.getLayout();
    auto l2 = config.getLayouts();
    EXPECT_EQ(l1, "NHWC");
    EXPECT_EQ(l2.size(), 0);
}

TEST(ModelConfig, layout_multi) {
    ovms::ModelConfig config;

    ovms::layouts_map_t layouts;
    layouts["A"] = "layout_A";
    layouts["B"] = "layout_B";

    config.setLayout("NHWC");
    config.setLayouts(layouts);

    auto l1 = config.getLayout();
    auto l2 = config.getLayouts();
    EXPECT_EQ(l1, "");
    EXPECT_THAT(l2, UnorderedElementsAre(
        Pair("A", "layout_A"),
        Pair("B", "layout_B")));

    config.setLayout("NHWC");
    l2 = config.getLayouts();
    EXPECT_EQ(l2.size(), 0);

    config.setLayouts(layouts);
    config.addLayout("C", "layout_C");
    l1 = config.getLayout();
    l2 = config.getLayouts();
    EXPECT_EQ(l1, "");
    EXPECT_THAT(l2, UnorderedElementsAre(
        Pair("A", "layout_A"),
        Pair("B", "layout_B"),
        Pair("C", "layout_C")));
}

TEST(ModelConfig, shape) {
    ovms::ModelConfig config;

    ovms::shape_t s1{1, 2, 3};
    ovms::shape_t s2{6, 6, 200, 300};
    ovms::shape_t s3{100, 500};

    ovms::shapes_map_t shapeMap;
    shapeMap["first"] = s1;
    shapeMap["second"] = s2;

    // single shape
    config.setShape(s1);
    auto gs1 = config.getShape();
    auto gs2 = config.getShapes();
    EXPECT_THAT(gs1, ElementsAre(1, 2, 3));
    EXPECT_EQ(gs2.size(), 0);

    // single from string
    std::string str1 = "";
    std::string str2 = "(64,128,256,   300)";
    std::string str3 = "   (     64 , 300   )   ";
    std::string str4 = "    64 , 300   )   ";

    config.setShape(str1);
    auto r_str1 = config.getShape();
    EXPECT_EQ(r_str1.size(), 0);

    config.setShape(str2);
    auto r_str2 = config.getShape();
    EXPECT_THAT(r_str2, ElementsAre(64, 128, 256, 300));

    config.setShape(str3);
    auto r_str3 = config.getShape();
    EXPECT_THAT(r_str3, ElementsAre(64, 300));

    config.setShape(str4);
    auto r_str4 = config.getShape();
    EXPECT_EQ(r_str4.size(), 0);

    // mutli shape
    config.setShapes(shapeMap);
    config.addShape("third", s3);

    gs1 = config.getShape();
    gs2 = config.getShapes();
    EXPECT_EQ(gs1.size(), 0);
    EXPECT_EQ(gs2.size(), 3);
    EXPECT_THAT(gs2["second"], ElementsAre(6, 6, 200, 300));
}

TEST(ModelConfig, plugin_config) {
    ovms::ModelConfig config;
    ovms::plugin_config_t pluginConfig{
        {"OptionA", "ValueA"},
        {"OptionX", "ValueX"},
    };

    config.setPluginConfig(pluginConfig);

    auto actualPluginConfig = config.getPluginConfig();
    EXPECT_THAT(actualPluginConfig, UnorderedElementsAre(
        Pair("OptionA", "ValueA"),
        Pair("OptionX", "ValueX")));
}

TEST(ModelConfig, mappingInputs) {
    ovms::ModelConfig config;
    ovms::mapping_config_t mapping {
        { "resnet", "value" },
        { "output", "input"}
    };

    config.setMappingInputs(mapping);
    auto ret = config.getMappingInputs();
    EXPECT_THAT(ret, UnorderedElementsAre(
        Pair("resnet", "value"),
        Pair("output", "input")));

    auto in = config.getMappingInputByKey("output");
    auto empty = config.getMappingInputByKey("notexist");

    EXPECT_EQ(in, "input");
    EXPECT_EQ(empty, "");
}

TEST(ModelConfig, mappingOutputs) {
    ovms::ModelConfig config;
    ovms::mapping_config_t mapping {
        { "resnet", "value" },
        { "output", "input"}
    };

    config.setMappingOutputs(mapping);
    auto ret = config.getMappingOutputs();
    EXPECT_THAT(ret, UnorderedElementsAre(
        Pair("resnet", "value"),
        Pair("output", "input")));

    auto in = config.getMappingOutputByKey("output");
    auto empty = config.getMappingOutputByKey("notexist");

    EXPECT_EQ(in, "input");
    EXPECT_EQ(empty, "");
}