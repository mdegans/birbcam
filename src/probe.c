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

#include "probe.h"

static void print_bbox(gint frame_num, NvOSD_RectParams* rect);
static void write_json(gint frame_num, NvOSD_RectParams* rect, BcData* data);
static void write_protobuf(gint frame_num,
                           NvOSD_RectParams* rect,
                           BcData* data);

// many thanks to NVIDIA's test1_app for showing me how to do this
GstPadProbeReturn on_batch(GstPad* pad, GstPadProbeInfo* info, BcData* data) {
  // get batched metadata:
  NvDsBatchMeta* batch = gst_buffer_get_nvds_batch_meta((GstBuffer*)info->data);

  NvDsMetaList* frames = NULL;
  NvDsFrameMeta* frame = NULL;
  NvDsMetaList* objects = NULL;
  NvDsObjectMeta* object = NULL;

  // for frame in batch.frame_meta_list:
  for (frames = batch->frame_meta_list; frames != NULL; frames = frames->next) {
    frame = (NvDsFrameMeta*)(frames->data);

    // for object in frame.obj_meta_list:
    for (objects = frame->obj_meta_list; objects != NULL;
         objects = objects->next) {
      object = (NvDsObjectMeta*)(objects->data);

      if (object->class_id == BIRB_ID) {
        print_bbox(frame->frame_num, &object->rect_params);
        switch (data->args->meta_type) {
          case JSON_LINES:
            write_json(frame->frame_num, &object->rect_params, data);
            break;
          case PROTOBUF:
            write_protobuf(frame->frame_num, &object->rect_params, data);
            break;
        }
      }
    }
  }
}

static void print_bbox(gint frame_num, NvOSD_RectParams* rect) {
  g_print(JSON_RECORD, frame_num, rect->top, rect->height, rect->left,
          rect->width);
}

static void write_json(gint frame_num, NvOSD_RectParams* rect, BcData* data) {
  // write the filled out json record to data->meta_file
  if (fprintf(data->meta_file, JSON_RECORD, frame_num, rect->top, rect->height,
              rect->left, rect->width) < 0) {
    // fprintf had an error, so write a short error with the frame number
    if (fprintf(data->meta_file, JSON_RECORD_ERR, frame_num, "fprintf") < 0) {
      // and if even that fails, this is critical so break out of the main loop
      g_printerr(ERR_METADATA_WRITE);
      g_main_loop_quit(data->main_loop);  // break out of the main loop
    }
  }
}

static void write_protobuf(gint frame_num,
                           NvOSD_RectParams* rect,
                           BcData* data) {
  // TODO
}
