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

#include "pipeline.h"

// these create the branches of the pipeline
gboolean create_pipeline_begin(PipelineData* p_data);
gboolean create_encoder_branch(PipelineData* p_data, const gchar* filename);
gboolean create_nvinfer_branch(PipelineData* p_data);

// this links the entire pipeline together
gboolean link_pipeline(PipelineData* p_data);

// main pipeline creation function
gboolean create_pipeline_data(PipelineData* p_data, const gchar* filename) {
  // create the branches of the pipeline ...
  if (!create_pipeline_begin(p_data))
    return cleanup_pipeline_data(p_data);
  if (!create_encoder_branch(p_data, filename))
    return cleanup_pipeline_data(p_data);
  if (!create_nvinfer_branch(p_data))
    return cleanup_pipeline_data(p_data);

  // ... and link them together
  if (!link_pipeline(p_data))
    return cleanup_pipeline_data(p_data);

  // dump a pipeline graph to file
  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(p_data->pipeline), GST_DEBUG_GRAPH_SHOW_ALL,
                            "pipeline");

  return TRUE;
}

GstElement* create_and_add_element(GstPipeline* pipeline, const gchar* name) {
  // make an element
  GstElement* elem = gst_element_factory_make(name, name);

  // verify if was created and fail if not
  if (!GST_IS_ELEMENT(elem)) {
    GST_ERROR(ERR_ELEM, name);
    return NULL;
  }

  // add the element to the pipeline bin
  gst_bin_add(GST_BIN(pipeline), elem);

  return elem;
}

gboolean create_pipeline_begin(PipelineData* p_data) {
  // create a new Pipeline (Bin subclass) and check that it exists
  p_data->pipeline = GST_PIPELINE(gst_pipeline_new("pipeline"));
  if (!GST_IS_PIPELINE(p_data->pipeline)) {
    GST_ERROR(ERR_PIPELINE_CREATION);
    return FALSE;
  }

  // a handy pointer to the bus to avoid having to call
  // gst_pipeline_get_bus repeatedly.
  // this must be unreferenced in the cleanup function.
  p_data->bus = gst_pipeline_get_bus(p_data->pipeline);
  if (!GST_IS_BUS(p_data->bus)) {
    GST_ERROR(ERR_BUS_GET);
    return FALSE;
  }

  // create the camera, and add it to the Pipeline
#ifdef IS_TEGRA  // use the CSI camera, TODO: add usb camera support
  p_data->camera = create_and_add_element(p_data->pipeline, BC_CAMERA_CSI);
  if (!GST_IS_ELEMENT(p_data->camera)) {
    GST_ERROR(ERR_CAMERA_CREATION);
    return FALSE;
  }
  g_object_set(G_OBJECT(p_data->camera), "aeantibanding",
               0,                // disables flicker reduction
               "maxperf", TRUE,  // enable argus maximum performance
               "ee-mode", 2,     // edge enhancement off
               "tnr-mode", 2,    // high quality temporal noise reduction
               NULL);
#else
  // TODO(x86) make separate source and converter element
  GST_ERROR("X86 support unimplimentted");
  return FALSE;
#endif
  // create a capsfilter element to tell the camera what we want sent downstream
  p_data->capsfilter =
      create_and_add_element(p_data->pipeline, BC_ELEM_CAPS_FILTER);
  if (p_data->capsfilter == NULL)
    return FALSE;
  g_object_set(G_OBJECT(p_data->capsfilter), "caps",
               gst_caps_from_string(BC_CAPS_STRING), NULL);

  // create a tee (T) junction element to split the pipeline into two branches
  // (encoder branch and inference branch)
  p_data->tee = create_and_add_element(p_data->pipeline, BC_ELEM_TEE);
  if (p_data->tee == NULL)
    return FALSE;

  return TRUE;
}

gboolean create_encoder_branch(PipelineData* p_data, const gchar* filename) {
  // create the encoder queue to buffer data and run everything downstream
  // in it's own thread
  p_data->enc_queue = gst_element_factory_make(BC_ELEM_QUEUE, "enc_queue");
  if (!p_data->enc_queue) {
    GST_ERROR(ERR_ELEM, "(encoder) queue");
    return FALSE;
  }
  gst_bin_add(GST_BIN(p_data->pipeline), p_data->enc_queue);

  // create and configure the h265 encoder
  p_data->encoder = create_and_add_element(p_data->pipeline, BC_ELEM_ENCODER);
  if (p_data->encoder == NULL)
    return FALSE;
  g_object_set(G_OBJECT(p_data->encoder), "bitrate", 4000000, NULL);

  // create the parser
  p_data->parser = create_and_add_element(p_data->pipeline, BC_ELEM_PARSER);
  if (p_data->parser == NULL)
    return FALSE;

  // create and configure the muxer
  p_data->muxer = create_and_add_element(p_data->pipeline, BC_ELEM_MUXER);
  if (p_data->muxer == NULL)
    return FALSE;
  g_object_set(G_OBJECT(p_data->muxer), "writing-app", "birbcam", NULL);
  // write index every minute so if something happens, the file will still be
  // seekable (probably, haven't tested this)  TODO: test this
  g_object_set(G_OBJECT(p_data->muxer), "min-index-interval", (guint64)6e+10,
               NULL);

  // filesink
  p_data->filesink = create_and_add_element(p_data->pipeline, BC_ELEM_FILESINK);
  if (p_data->filesink == NULL)
    return FALSE;
  g_object_set(G_OBJECT(p_data->filesink), "location", filename, NULL);

  return TRUE;
}

gboolean create_nvinfer_branch(PipelineData* p_data) {
  // create the inference queue to run everything downstream in it's own thread
  // and to buffer input
  p_data->infer_queue = gst_element_factory_make(BC_ELEM_QUEUE, "infer_queue");
  if (!p_data->infer_queue) {
    GST_ERROR("Could not create inference queue.");
    return FALSE;
  }
  gst_bin_add(GST_BIN(p_data->pipeline), p_data->infer_queue);

  // create nvstreammux element to add the metadata
  p_data->streammux =
      create_and_add_element(p_data->pipeline, BC_ELEM_STREAM_MUX);
  if (p_data->streammux == NULL)
    return FALSE;
  g_object_set(G_OBJECT(p_data->streammux), "batch-size", 1, "width", 384,
               "height", 216, NULL);

  // create primary inference element (no secondary as of yet, maybe use the
  // Coral, but will need to write a plugin for that since Google's is written
  // in python, no really)
  p_data->infer = create_and_add_element(p_data->pipeline, BC_ELEM_INFERENCE);
  if (p_data->infer == NULL)
    return FALSE;
  g_object_set(G_OBJECT(p_data->infer), "config-file-path", "../birbcam.cfg",
               NULL);

  // create a fakesink, onto which a probe will be attached to call on_batch
  // for each batch of frames to parse the metadata
  p_data->fakesink = create_and_add_element(p_data->pipeline, BC_ELEM_FAKESINK);
  if (p_data->fakesink == NULL) {
    return FALSE;
  }
  return TRUE;
}

gboolean link_pipeline(PipelineData* p_data) {
#ifdef IS_TEGRA
  // link pipeline beginning
  if (!gst_element_link_many(p_data->camera, p_data->capsfilter, p_data->tee,
                             NULL)) {
    // "Could not link pipeline %s."
    GST_ERROR(ERR_LINK, "beginning");
    return FALSE;
  }
#else  // X86
  // link: camera, converter, capsfilter, tee
#endif
  // link and connect encoder branch
  if (!gst_element_link_many(p_data->enc_queue, p_data->encoder, p_data->parser,
                             p_data->muxer, p_data->filesink, NULL)) {
    GST_ERROR(ERR_LINK, "encoder branch");
    return FALSE;
  }

  // link and connect inference branch
  if (!gst_element_link_pads(p_data->infer_queue, "src", p_data->streammux,
                             "sink_0")) {
    GST_ERROR(ERR_LINK, "inference queue and stream muxer");
    return FALSE;
  }
  if (!gst_element_link_many(p_data->streammux, p_data->infer, p_data->fakesink,
                             NULL)) {
    GST_ERROR(ERR_LINK, "inference branch");
    return FALSE;
  }

  // link the branches to the tee
  if (!gst_element_link(p_data->tee, p_data->enc_queue)) {
    GST_ERROR(ERR_LINK, "tee and encoder queue");
  }
  if (!gst_element_link(p_data->tee, p_data->infer_queue)) {
    GST_ERROR(ERR_LINK, "tee and inference queue");
  }

  return TRUE;
}

gboolean shutdown_pipeline(PipelineData* p_data) {
  // set the pipeline to the playing state
  gst_element_set_state(GST_ELEMENT(p_data->pipeline), GST_STATE_NULL);
}

gboolean cleanup_pipeline_data(PipelineData* p_data) {
  // unreference the bus because gst_pipeline_get_bus requires it
  if (p_data->bus) {
    gst_object_unref(p_data->bus);
  }
  shutdown_pipeline(p_data);
  // unreference the pipeline (and all elements added to it implicitly)
  if (p_data->pipeline)
    gst_object_unref(p_data->pipeline);
  return FALSE;
}