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

#ifndef BIRBCAM_C_DATA_H
#define BIRBCAM_C_DATA_H

#include "pipeline.h"  // where PipelineData struct is defined

#define BIRB_ID 1  // the detection id of a birb TODO: use a real number

typedef enum {
  JSON_LINES,
  PROTOBUF,
} MetaType;

typedef struct {  // struct to hold parsed arguments
  gchar* base_filename;
  gchar* mkv_filename;
  gchar* meta_filename;
  MetaType meta_type;
} BcArgs;

// main data struct to pass around through callback hell. Hail Satan!
typedef struct {
  PipelineData* pipeline_data;
  GMainLoop* main_loop;
  BcArgs* args;
  FILE* meta_file;
} BcData;

#endif  // BIRBCAM_C_DATA_H
