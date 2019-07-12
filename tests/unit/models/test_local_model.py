#
# Copyright (c) 2018 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
import os
import pytest
from ie_serving.models.local_model import LocalModel
from ie_serving.models.models_utils import ModelVersionStatus


def test_model_init():
    available_versions = [1, 2, 3]
    model_name = "test"
    versions_statuses = {}
    for version in available_versions:
        versions_statuses[version] = ModelVersionStatus(model_name, version)
    new_model = LocalModel(model_name=model_name, model_directory='fake_path',
                           available_versions=available_versions, engines={},
                           batch_size=None,
                           version_policy_filter=lambda versions: versions[:],
                           versions_statuses=versions_statuses)
    assert new_model.default_version == 3
    assert new_model.model_name == 'test'
    assert new_model.model_directory == 'fake_path'
    assert new_model.engines == {}


@pytest.mark.parametrize("mocker_values, expected_output", [
    ([['/data/model/3/model.bin'], ['/data/model/3/model.xml'], []],
     ['/data/model/3/model.xml', '/data/model/3/model.bin', None]),
    ([[], ['/data/model/3/model.xml'], []],
     [None, None, None]),
    ([['/data/model/3/model.bin'], [], []],
     [None, None, None]),
    ([[], [], []],
     [None, None, None])
])
def test_get_versions_files(mocker, mocker_values, expected_output):
    glob_mocker = mocker.patch('glob.glob')
    glob_mocker.side_effect = mocker_values

    xml_f, bin_f, mapping = LocalModel.get_version_files('/data/model/3/')
    assert expected_output[0] == xml_f
    assert expected_output[1] == bin_f
    assert expected_output[2] is mapping


def test_get_engines_for_model(mocker):
    engines_mocker = mocker.patch('ie_serving.models.ir_engine.IrEngine.'
                                  'build')
    engines_mocker.side_effect = ['modelv2', 'modelv4']
    available_versions = [{'xml_file': 'modelv2.xml',
                           'bin_file': 'modelv2.bin',
                           'mapping_config': 'mapping_config.json',
                           'version_number': 2, 'batch_size': None},
                          {'xml_file': 'modelv4.xml',
                           'bin_file': 'modelv4.bin',
                           'mapping_config': 'mapping_config.json',
                           'version_number': 4, 'batch_size': None}]
    versions_statuses = {}
    for version in available_versions:
        version_number = version['version_number']
        versions_statuses[version_number] = ModelVersionStatus("test",
                                                               version_number)
    output = LocalModel.get_engines_for_model(
        versions_attributes=available_versions,
        versions_statuses=versions_statuses)
    assert 2 == len(output)
    assert 'modelv2' == output[2]
    assert 'modelv4' == output[4]


def test_get_engines_for_model_with_ir_raises(mocker):
    engines_mocker = mocker.patch('ie_serving.models.ir_engine.IrEngine.'
                                  'build')
    engines_mocker.side_effect = ['modelv2', 'modelv4', Exception("test")]
    available_versions = [{'xml_file': 'modelv2.xml',
                           'bin_file': 'modelv2.bin',
                           'mapping_config': 'mapping_config.json',
                           'version_number': 2, 'batch_size': None},
                          {'xml_file': 'modelv4.xml',
                           'bin_file': 'modelv4.bin',
                           'mapping_config': 'mapping_config.json',
                           'version_number': 3, 'batch_size': None},
                          {'xml_file': 'modelv4.xml',
                           'bin_file': 'modelv4.bin',
                           'mapping_config': 'mapping_config.json',
                           'version_number': 4, 'batch_size': None}]
    versions_statuses = {}
    for version in available_versions:
        version_number = version['version_number']
        versions_statuses[version_number] = ModelVersionStatus(
            "test", version_number)
    output = LocalModel.get_engines_for_model(
        versions_attributes=available_versions,
        versions_statuses=versions_statuses)
    assert 2 == len(output)
    assert 'modelv2' == output[2]
    assert 'modelv4' == output[3]


def test_get_versions():
    model = LocalModel
    file_path = os.path.realpath(__file__)
    unit_tests_path = os.path.dirname(os.path.dirname(file_path))
    output = model.get_versions(unit_tests_path)
    assert 3 == len(output)
    output = model.get_versions(unit_tests_path + os.sep)
    assert 3 == len(output)
