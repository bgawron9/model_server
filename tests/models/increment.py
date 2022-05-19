#
# Copyright (c) 2020 Intel Corporation
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

import tensorflow as tf
from tensorflow.python.framework import ops
from tensorflow.python.saved_model import signature_constants
from tensorflow.python.saved_model import tag_constants
import numpy as np

model_dir = "/tmp/increment/2"

@tf.function
def f(x):
  return x

graph_def = f.get_concrete_function(1.).graph.as_graph_def()


class increment(tf.Module):
  def __init__(self):


    tf.import_graph_def(graph_def, name="")
    input = tf.keras.Input(name='input', shape=[10], dtype=tf.float32)  # defines input shape

    const = tf.constant(1.0, name="const")

    output = tf.add(input, const, name='output')  # defines network operation


    g = ops.get_default_graph()

    inp= g.get_tensor_by_name("input:0")

    out= g.get_tensor_by_name("output:0")



model=increment()

tf.saved_model.save(model,model_dir)

data = np.ones((1,10),np.float32)

@tf.function
def preds(input):
    print("Loading model")

    tf.saved_model.load(model_dir)
    print("Model loaded")

    graph = ops.get_default_graph()

    input = graph.get_tensor_by_name("input:0")

    preds = graph.get_tensor_by_name("output:0")
    print(preds)
    
preds(data)
