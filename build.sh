#!/bin/bash
cd /Users/tatsuya/sicf/mediapipe
bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1     mediapipe/examples/desktop/iris_tracking:iris_tracking_cpu