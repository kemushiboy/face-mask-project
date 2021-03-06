// Copyright 2019 The MediaPipe Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/canonical_errors.h"
#include <iostream>
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/formats/rect.pb.h"
#include "mediapipe/framework/formats/detection.pb.h"
#include "mediapipe/framework/formats/location_data.pb.h"
#include "mediapipe/framework/formats/wrapper_iris_tracking.pb.h"


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT     8080
#define MAXLINE 1024
#define LANDMARKSIZELIMIT 32
int sockfd;
struct sockaddr_in     servaddr;

namespace mediapipe {

constexpr char kLandmarksTag[] = "FACE_LANDMARKS";
constexpr char kNormLandmarksTag[] = "NORM_LANDMARKS"; // MAD note @to-fix: streaming NORM_LANDMARKS, but they're labeled LANDMARKS
constexpr char kNormRectTag[] = "NORM_RECT";
constexpr char kDetectionsTag[] = "DETECTIONS";

void setup_udp(){
  // int sockfd;
  char buffer[MAXLINE];
  // char *hello = "Hello from client";
  // struct sockaddr_in     servaddr;

  // Creating socket file descriptor
  if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
      perror("socket creation failed");
      exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));

  // Filling server information
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(PORT);
  servaddr.sin_addr.s_addr = INADDR_ANY;

  // initial testing for connection
  // while(true){
  //   sendto(sockfd, (const char *)hello, strlen(hello),
  //       0, (const struct sockaddr *) &servaddr,
  //           sizeof(servaddr));
  //   printf("Hello message sent.\n");
  // }
  // close(sockfd);
}


// A Calculator that simply passes its input Packets and header through,
// unchanged.  The inputs may be specified by tag or index.  The outputs
// must match the inputs exactly.  Any number of input side packets may
// also be specified.  If output side packets are specified, they must
// match the input side packets exactly and the Calculator passes its
// input side packets through, unchanged.  Otherwise, the input side
// packets will be ignored (allowing PassThroughCalculator to be used to
// test internal behavior).  Any options may be specified and will be
// ignored.
class MyIrisPassThroughCalculator : public CalculatorBase {
 public:
  static ::mediapipe::Status GetContract(CalculatorContract* cc) {
    if (!cc->Inputs().TagMap()->SameAs(*cc->Outputs().TagMap())) {
      return ::mediapipe::InvalidArgumentError(
          "Input and output streams to PassThroughCalculator must use "
          "matching tags and indexes.");
    }
    for (CollectionItemId id = cc->Inputs().BeginId();
         id < cc->Inputs().EndId(); ++id) {
      cc->Inputs().Get(id).SetAny();
      cc->Outputs().Get(id).SetSameAs(&cc->Inputs().Get(id));
    }
    for (CollectionItemId id = cc->InputSidePackets().BeginId();
         id < cc->InputSidePackets().EndId(); ++id) {
      cc->InputSidePackets().Get(id).SetAny();
    }
    if (cc->OutputSidePackets().NumEntries() != 0) {
      if (!cc->InputSidePackets().TagMap()->SameAs(
              *cc->OutputSidePackets().TagMap())) {
        return ::mediapipe::InvalidArgumentError(
            "Input and output side packets to PassThroughCalculator must use "
            "matching tags and indexes.");
      }
      for (CollectionItemId id = cc->InputSidePackets().BeginId();
           id < cc->InputSidePackets().EndId(); ++id) {
        cc->OutputSidePackets().Get(id).SetSameAs(
            &cc->InputSidePackets().Get(id));
      }
    }
    return ::mediapipe::OkStatus();
  }

  ::mediapipe::Status Open(CalculatorContext* cc) final {
    for (CollectionItemId id = cc->Inputs().BeginId();
         id < cc->Inputs().EndId(); ++id) {
      if (!cc->Inputs().Get(id).Header().IsEmpty()) {
        cc->Outputs().Get(id).SetHeader(cc->Inputs().Get(id).Header());
      }
    }
    if (cc->OutputSidePackets().NumEntries() != 0) {
      for (CollectionItemId id = cc->InputSidePackets().BeginId();
           id < cc->InputSidePackets().EndId(); ++id) {
        cc->OutputSidePackets().Get(id).Set(cc->InputSidePackets().Get(id));
      }
    }
    cc->SetOffset(TimestampDiff(0));


    setup_udp();


    return ::mediapipe::OkStatus();
  }

  ::mediapipe::Status Process(CalculatorContext* cc) final {
    cc->GetCounter("PassThrough")->Increment();
    if (cc->Inputs().NumEntries() == 0) {
      return tool::StatusStop();
    }
    for (CollectionItemId id = cc->Inputs().BeginId();
         id < cc->Inputs().EndId(); ++id) {
      if (!cc->Inputs().Get(id).IsEmpty()) {


        /*-------------------------------------------------------------------*/
        /*------------ EDITS to original pass_through_calculator ------------*/
        /*-------------------------------------------------------------------*/

        WrapperIrisTracking* wrapper = new WrapperIrisTracking();
        wrapper->InitAsDefaultInstance();

        //std::cout << cc->Inputs().Get(id).Name() << std::endl;

        if(false){
        //if (cc->Inputs().Get(id).Name() == "face_landmarks"){
          // the type is a NormalizedLandmarkList, but you need the kLandmarksTag
          // in order for it not to crash for some reason ...
          const NormalizedLandmarkList& landmarks = cc->Inputs().Tag(kLandmarksTag).Get<NormalizedLandmarkList>();


          for (int i = 0; i < landmarks.landmark_size(); ++i) {
              const NormalizedLandmark& landmark = landmarks.landmark(i);
               //std::cout << "Landmark " << i <<":\n" << landmark.DebugString() << '\n';

              wrapper->mutable_landmarks()->add_landmark();
              int size = wrapper->mutable_landmarks()->landmark_size()-1;
              wrapper->mutable_landmarks()->mutable_landmark(size)->set_x(landmark.x());
              wrapper->mutable_landmarks()->mutable_landmark(size)->set_y(landmark.y());
              wrapper->mutable_landmarks()->mutable_landmark(size)->set_z(landmark.z());
              wrapper->mutable_landmarks()->mutable_landmark(size)->set_visibility(landmark.visibility());
          }
        }

        if(false){
        //if (cc->Inputs().Get(id).Name() == "face_detections"){
          // Palm is detected once, not continuously — when it first shows up in the image
          const auto& detections = cc->Inputs().Tag(kDetectionsTag).Get<std::vector<Detection>>();
          for (int i = 0; i < detections.size(); ++i) {
              const Detection& detection = detections[i];
               //std::cout << "\n----- Detection -----\n " << detection.DebugString() << '\n';
               wrapper->mutable_detection()->add_detection();

          }
        }
        if(false){
        //if (cc->Inputs().Get(id).Name() == "face_rect"){
          // The Hand Rect is an x,y center, width, height, and angle (in radians)
          const NormalizedRect& rect = cc->Inputs().Tag(kNormRectTag).Get<NormalizedRect>();

          wrapper->mutable_rect()->set_x_center(rect.x_center());
          wrapper->mutable_rect()->set_y_center(rect.y_center());
          wrapper->mutable_rect()->set_width(rect.width());
          wrapper->mutable_rect()->set_height(rect.height());
          wrapper->mutable_rect()->set_rotation(rect.rotation());


          // std::cout << "Face Rect: " << rect.DebugString() << '\n';
          // std::string msg_buffer;
          // rect.SerializeToString(&msg_buffer);
          // sendto(sockfd, msg_buffer.c_str(), msg_buffer.length(),
          //     0, (const struct sockaddr *) &servaddr,
          //         sizeof(servaddr));

        }

        //if(false){
        if (cc->Inputs().Get(id).Name() == "right_eye_rect_from_landmarks"){
          const NormalizedRect& right_eye_rect = cc->Inputs().Tag("RIGHT_EYE_RECT").Get<NormalizedRect>();
          wrapper->mutable_right_eye_rect()->set_x_center(right_eye_rect.x_center());
          wrapper->mutable_right_eye_rect()->set_y_center(right_eye_rect.y_center());
          wrapper->mutable_right_eye_rect()->set_width(right_eye_rect.width());
          wrapper->mutable_right_eye_rect()->set_height(right_eye_rect.height());
          wrapper->mutable_right_eye_rect()->set_rotation(right_eye_rect.rotation());

          //std::cout << right_eye_rect.DebugString() << std::endl;

          // std::string msg_buffer;
          // right_eye_rect.SerializeToString(&msg_buffer);
          // sendto(sockfd, msg_buffer.c_str(), msg_buffer.length(),
          //     0, (const struct sockaddr *) &servaddr,
          //         sizeof(servaddr));
        }

        //if(false){
        if (cc->Inputs().Get(id).Name() == "left_eye_rect_from_landmarks"){
          const NormalizedRect& left_eye_rect = cc->Inputs().Tag("LEFT_EYE_RECT").Get<NormalizedRect>();
          wrapper->mutable_left_eye_rect()->set_x_center(left_eye_rect.x_center());
          wrapper->mutable_left_eye_rect()->set_y_center(left_eye_rect.y_center());
          wrapper->mutable_left_eye_rect()->set_width(left_eye_rect.width());
          wrapper->mutable_left_eye_rect()->set_height(left_eye_rect.height());
          wrapper->mutable_left_eye_rect()->set_rotation(left_eye_rect.rotation());

          //std::cout << left_eye_rect.DebugString() << std::endl;

          // std::string msg_buffer;
          // right_eye_rect.SerializeToString(&msg_buffer);
          // sendto(sockfd, msg_buffer.c_str(), msg_buffer.length(),
          //     0, (const struct sockaddr *) &servaddr,
          //         sizeof(servaddr));
        }

        if(false){
        //if (cc->Inputs().Get(id).Name() == "right_iris_landmarks"){
          // the type is a NormalizedLandmarkList, but you need the kLandmarksTag
          // in order for it not to crash for some reason ...
          const NormalizedLandmarkList& landmarks = cc->Inputs().Tag("IRIS_LANDMARKS_RIGHT").Get<NormalizedLandmarkList>();

          for (int i = 0; i < landmarks.landmark_size(); ++i) {
              const NormalizedLandmark& landmark = landmarks.landmark(i);
               //std::cout << "iris_landmarks_right " << i <<":\n" << landmark.DebugString() << '\n';

              wrapper->mutable_iris_landmarks_right()->add_landmark();
              int size = wrapper->mutable_iris_landmarks_right()->landmark_size()-1;
              wrapper->mutable_iris_landmarks_right()->mutable_landmark(size)->set_x(landmark.x());
              wrapper->mutable_iris_landmarks_right()->mutable_landmark(size)->set_y(landmark.y());
              wrapper->mutable_iris_landmarks_right()->mutable_landmark(size)->set_z(landmark.z());
              wrapper->mutable_iris_landmarks_right()->mutable_landmark(size)->set_visibility(landmark.visibility());
          }
        }

        if(false){
        //if (cc->Inputs().Get(id).Name() == "left_iris_landmarks"){
          // the type is a NormalizedLandmarkList, but you need the kLandmarksTag
          // in order for it not to crash for some reason ...
          const NormalizedLandmarkList& landmarks = cc->Inputs().Tag("IRIS_LANDMARKS_LEFT").Get<NormalizedLandmarkList>();

          for (int i = 0; i < landmarks.landmark_size(); ++i) {
              const NormalizedLandmark& landmark = landmarks.landmark(i);
              // std::cout << "iris_landmarks_left " << i <<":\n" << landmark.DebugString() << '\n';

              wrapper->mutable_iris_landmarks_left()->add_landmark();
              int size = wrapper->mutable_iris_landmarks_left()->landmark_size()-1;
              wrapper->mutable_iris_landmarks_left()->mutable_landmark(size)->set_x(landmark.x());
              wrapper->mutable_iris_landmarks_left()->mutable_landmark(size)->set_y(landmark.y());
              wrapper->mutable_iris_landmarks_left()->mutable_landmark(size)->set_z(landmark.z());
              wrapper->mutable_iris_landmarks_left()->mutable_landmark(size)->set_visibility(landmark.visibility());
          }
        }

        if (cc->Inputs().Get(id).Name() == "right_eye_contour_landmarks"){
          // the type is a NormalizedLandmarkList, but you need the kLandmarksTag
          // in order for it not to crash for some reason ...
          const NormalizedLandmarkList& landmarks = cc->Inputs().Tag("EYE_LANDMARKS_RIGHT").Get<NormalizedLandmarkList>();

          for (int i = 0; i < landmarks.landmark_size(); ++i) {
             if(i >= LANDMARKSIZELIMIT ){
                break;
              }
              const NormalizedLandmark& landmark = landmarks.landmark(i);
               //std::cout << "eye_landmarks_right " << i <<":\n" << landmark.DebugString() << '\n';

              wrapper->mutable_eye_landmarks_right()->add_landmark();
              int size = wrapper->mutable_eye_landmarks_right()->landmark_size()-1;
              
              wrapper->mutable_eye_landmarks_right()->mutable_landmark(size)->set_x(landmark.x());
              wrapper->mutable_eye_landmarks_right()->mutable_landmark(size)->set_y(landmark.y());
          }
        }

        if (cc->Inputs().Get(id).Name() == "left_eye_contour_landmarks"){
          // the type is a NormalizedLandmarkList, but you need the kLandmarksTag
          // in order for it not to crash for some reason ...
          const NormalizedLandmarkList& landmarks = cc->Inputs().Tag("EYE_LANDMARKS_LEFT").Get<NormalizedLandmarkList>();

          for (int i = 0; i < landmarks.landmark_size(); ++i) {
             if(i >= LANDMARKSIZELIMIT ){
                break;
              }
              const NormalizedLandmark& landmark = landmarks.landmark(i);
               //std::cout << "eye_landmarks_left " << i <<":\n" << landmark.DebugString() << '\n';

              wrapper->mutable_eye_landmarks_left()->add_landmark();
              int size = wrapper->mutable_eye_landmarks_left()->landmark_size()-1;
              
              wrapper->mutable_eye_landmarks_left()->mutable_landmark(size)->set_x(landmark.x());
              wrapper->mutable_eye_landmarks_left()->mutable_landmark(size)->set_y(landmark.y());
              
          }
        }

        // std::cout << "  Passing " << cc->Inputs().Get(id).Name() << " to "
                // << cc->Outputs().Get(id).Name() << " at "
                // << cc->InputTimestamp().DebugString() << std::endl;

        // nothing gets to here
        if (cc->Inputs().HasTag(kNormLandmarksTag)) {
          // std::cout << "kNormLandmarksTag" << std::endl << std::endl;
          // std::cout << "  Passing " << cc->Inputs().Get(id).Name() << " to "
                  // << cc->Outputs().Get(id).Name() << " at "
                  // << cc->InputTimestamp().DebugString() << std::endl;
        }

        std::string msg_buffer;
        wrapper->SerializeToString(&msg_buffer);

        sendto(sockfd, msg_buffer.c_str(), msg_buffer.length(),
            0, (const struct sockaddr *) &servaddr,
                sizeof(servaddr));

      /*-------------------------------------------------------------------*/

        VLOG(3) << "Passing " << cc->Inputs().Get(id).Name() << " to "
                << cc->Outputs().Get(id).Name() << " at "
                << cc->InputTimestamp().DebugString();
        cc->Outputs().Get(id).AddPacket(cc->Inputs().Get(id).Value());
      }
    }
    return ::mediapipe::OkStatus();
  }

  ::mediapipe::Status Close(CalculatorContext* cc) {
    if (!cc->GraphStatus().ok()) {
      return ::mediapipe::OkStatus();
    }
    close(sockfd);
    return ::mediapipe::OkStatus();
  }

};
REGISTER_CALCULATOR(MyIrisPassThroughCalculator);

}  // namespace mediapipe
