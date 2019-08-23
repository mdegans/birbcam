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

#ifndef BIRBCAM_C_PROBE_H
#define BIRBCAM_C_PROBE_H

#include <gst/gst.h>
#include <gstnvdsmeta.h>
#include <stdio.h>

#include "data.h"

#define JSON_RECORD "{\"f\": %d, \"t\": %d, \"h\": %d, \"l\": %d, \"w\": %d}\n"
#define JSON_RECORD_ERR "{\"f\": %d, \"error\": \"%s\"}\n"

#define ERR_METADATA_WRITE "CRITICAL: Can't write anything to metadata file!!!!"
GstPadProbeReturn on_batch(GstPad* pad, GstPadProbeInfo* info, BcData* data);

#endif  // BIRBCAM_C_PROBE_H
