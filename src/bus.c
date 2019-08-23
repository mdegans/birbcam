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

#include "bus.h"

// a callback to process error messages from the Bus (we need this so the
// program exits on error or quit)
gboolean on_bus_message(GstBus* bus, GstMessage* message, BcData* data) {
  switch (GST_MESSAGE_TYPE(message)) {
    case GST_MESSAGE_EOS:
      GST_WARNING("End of stream reached. This shouldn't happen.");
      g_main_loop_quit(data->main_loop);
      break;
    case GST_MESSAGE_ERROR: {
      GError* err;
      gchar* debug_info;
      gst_message_parse_error(message, &err, &debug_info);
      GST_ERROR("Error received from %s: %s", message->src->name, err->message);
      g_clear_error(&err);
      g_free(err);
      g_main_loop_quit(data->main_loop);
      break;
    }
    default:
      GST_INFO("BUS_MSG: %s: %s", message->src->name,
               GST_MESSAGE_TYPE_NAME(message));
      break;
  }
  return TRUE;
}