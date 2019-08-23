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

#include "main.h"

// signal handler callback to shut down the main loop
gboolean on_SIGINT(BcData* data) {
  g_print(MSG_SIGINT);
  shutdown_pipeline(data->pipeline_data);
  g_main_loop_quit(data->main_loop);  // ask main loop to quit
  return FALSE;                       // unregister signal handler
}

gboolean parse_args(int argc, char** argv, BcArgs* args) {
  g_autoptr(GOptionContext) ctx = g_option_context_new("- Birbcam");
  g_autoptr(GError) err;

  // alternative to calling gst_init() is to get the GOption option group and
  // add it to your own context
  // https://gstreamer.freedesktop.org/documentation/application-development/basics/init.html?gi-language=c

  GOptionEntry entries[] = {
      {"output", 'o', 0, G_OPTION_ARG_FILENAME, &args->base_filename,
       "output base filename (minus extension)", "FILE"},
      {NULL},
  };

  g_option_context_add_main_entries(ctx, entries, NULL);
  g_option_context_add_group(ctx, gst_init_get_option_group());

  if (!g_option_context_parse(ctx, &argc, &argv, &err)) {
    g_print(ERR_PARSE, err->message);
    g_clear_error(&err);
    return FALSE;
  }

  // make sure the filename isn't blank
  if (!strcmp(args->base_filename, "")) {
    gst_printerr(
        "-o (output base filename) required, use --help for full usage)");
    return FALSE;
  }

  // calculate mkv and metadata filename based on passed options
  strcat(args->mkv_filename, args->base_filename);
  strcat(args->mkv_filename, ".mkv");
  GST_INFO("MKV FILENAME: %s", args->mkv_filename);
  strcat(args->meta_filename, args->base_filename);
  args->meta_type = JSON_LINES;  // TODO: add choices
  if (args->meta_type == JSON_LINES) {
    strcat(args->meta_filename, ".jl");
  } else if (args->meta_type == PROTOBUF) {
    strcat(args->meta_filename, ".brb");
  }
  GST_INFO("METADATA_FILENAME: %s", args->meta_filename);

  return TRUE;
}

int main(int argc, char** argv) {
  static char meta_buf[BUFSIZ];  // metadata buffer

  BcData data = {NULL};  // main data struct to pass around
  PipelineData p_data = {NULL};
  data.pipeline_data = &p_data;
  BcArgs args = {
      // default arguments
      (gchar*)calloc(1024, sizeof(gchar)),  // mkv_filename
      (gchar*)calloc(1024, sizeof(gchar)),  // meta_filename
      (gchar*)calloc(1020, sizeof(gchar)),  // base_filename
      JSON_LINES,                           // metadata type
  };
  data.args = &args;  // attach args to data

  // parse arguments and init GStreamer
  if (!parse_args(argc, argv, data.args))
    return -1;

  // create the pipeline and all it's elements (including bus)
  if (!create_pipeline_data(data.pipeline_data, args.mkv_filename)) {
    GST_ERROR(ERR_PIPELINE_DATA);
    return -1;
  }

  // Create the Glib MainLoop pointer, and attach it to data so it
  // can be passed around so we can shut down from anywhere easily.
  // it must be unreferenced later with g_main_loop_unref
  data.main_loop = g_main_loop_new(NULL, FALSE);

  // connect callbacks
  // TODO: only watch for bus error messages when !args.debug
  gst_bus_add_watch(data.pipeline_data->bus, (GstBusFunc)on_bus_message,
                    &data);  // on_bus_message defined in bus.h
  g_unix_signal_add(SIGINT, (GSourceFunc)on_SIGINT,
                    data.main_loop);  // handy, this function

  // connect metadata probe to fake sink pad in
  GstPad* sink_pad =
      gst_element_get_static_pad(data.pipeline_data->fakesink, "sink");
  gst_pad_add_probe(sink_pad, GST_PAD_PROBE_TYPE_BUFFER,
                    (GstPadProbeCallback)on_batch, (void*)&data, NULL);
  gst_object_unref(sink_pad);

  // set the pipeline to the playing state
  gst_element_set_state(GST_ELEMENT(data.pipeline_data->pipeline),
                        GST_STATE_PLAYING);

  // open output metadata file for writing
  data.meta_file = fopen(args.meta_filename, "w");
  // set the file to line buffered mode, TODO: change for protobuf
  if (setvbuf(data.meta_file, meta_buf, _IOLBF,
              BUFSIZ)) {  // returns 0 on success
    g_printerr("Could not set metadata file mode to line buffered.");
    return -1;
  }

  // run, main_loop run! (blocks here until main loop is shut down)
  g_main_loop_run(data.main_loop);

  // flush and close the metadata file
  fflush(data.meta_file);
  fclose(data.meta_file);

  // shut down and clean up pipeline and all elements
  cleanup_pipeline_data(data.pipeline_data);
  g_main_loop_unref(data.main_loop);

  return 0;
}
