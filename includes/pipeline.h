// Copyright (c) 2019 Michael de Gans
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef BIRBCAM_C_PIPELINE_H
#define BIRBCAM_C_PIPELINE_H

#define ERR_ELEM "Could not create GstElement: %s"
#define ERR_LINK "Could not link pipeline %s."
#define ERR_PIPELINE_CREATION "Pipeline creation failed."
#define ERR_PIPELINE_DATA "Could not create PipelineData struct."
#define ERR_BUS_GET "Could not get bus."
#define ERR_CAMERA_CREATION "Could not create camera."

#include <unistd.h>

#include <glib.h>
#include <gst/gst.h>

#include "nvds_config.h"

// sources
#define BC_CAMERA_CSI NVDS_ELEM_SRC_CAMERA_CSI
#define BC_ELEM_CAMERA_V4L2 NVDS_ELEM_SRC_CAMERA_V4L2
// utilities
#define BC_ELEM_CAPS_FILTER NVDS_ELEM_CAPS_FILTER
#define BC_CAPS_STRING                                            \
  "video/x-raw(memory:NVMM), width=(int)1920, height=(int)1080, " \
  "format=(string)NV12, framerate=(fraction)30/1"
#define BC_ELEM_QUEUE NVDS_ELEM_QUEUE
#define BC_ELEM_TEE NVDS_ELEM_TEE
// inference elements
#define BC_ELEM_STREAM_MUX NVDS_ELEM_STREAM_MUX
#define BC_ELEM_INFERENCE NVDS_ELEM_PGIE
// encoders
#define BC_ELEM_ENC_H265 NVDS_ELEM_ENC_H265
#define BC_ELEM_ENC_H264 NVDS_ELEM_ENC_H264
#define BC_ELEM_ENCODER BC_ELEM_ENC_H265
// video stream parsers
#define BC_ELEM_PARSE_H265 "h265parse"
#define BC_ELEM_PARSER BC_ELEM_PARSE_H265
// video container muxers
#define BC_ELEM_MKV NVDS_ELEM_MKV
#define BC_ELEM_MUXER BC_ELEM_MKV
// sink elements
#define BC_ELEM_FAKESINK NVDS_ELEM_SINK_FAKESINK
#define BC_ELEM_FILESINK NVDS_ELEM_SINK_FILE

// a struct to pass the pipeline elements to callbacks
typedef struct {
  // pipeline will unreference all its children on gst_object_unref()
  GstPipeline* pipeline;
  GstBus* bus;

  // pipeline beginning and split to T
  GstElement* camera;
#ifndef IS_TEGRA
  GstElement* converter;
#endif
  GstElement* capsfilter;
  GstElement* tee;

  // encoder branch of T split
  GstElement* enc_queue;
  GstElement* encoder;
  GstElement* parser;
  GstElement* muxer;
  GstElement* filesink;

  // metadata branch of T split
  GstElement* infer_queue;
  GstElement* streammux;
  GstElement* infer;
  GstElement* fakesink;
} PipelineData;

// create the pipeline and a struct to pass it and its members around
gboolean create_pipeline_data(PipelineData* p_data, const gchar* filename);
// returns false on cleanup success
gboolean cleanup_pipeline_data(PipelineData* p_data);
gboolean shutdown_pipeline(PipelineData* p_data);

#endif  // BIRBCAM_C_PIPELINE_H
