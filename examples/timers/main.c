// Copyright (c) 2020 Cesanta Software Limited
// All rights reserved
//
// Example Websocket server with timers. This is a simple Websocket echo
// server, which sends a message to all connected clients periodically,
// using timer API.
//
//  1. Start this server, type `make`
//  2. Open https://www.websocket.org/echo.html in your browser
//  3. In the "Location" text field, type ws://127.0.0.1:8000/websocket
//  4. See "hi" messages appearing periodically

#include "mongoose.h"

static const char *s_listen_on = "http://localhost:8000";

// This RESTful server implements the following endpoints:
static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    mg_ws_upgrade(c, hm, NULL);
    c->label[0] = 'W';  // Set some unique mark on a connection
  } else if (ev == MG_EV_WS_MSG) {
    // Got websocket frame. Received data is wm->data. Echo it back!
    struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
    mg_ws_send(c, wm->data.ptr, wm->data.len, WEBSOCKET_OP_TEXT);
    mg_iobuf_del(&c->recv, 0, c->recv.len);
  }
  (void) fn_data;
}

static void timer_fn(void *arg) {
  struct mg_mgr *mgr = (struct mg_mgr *) arg;
  // Broadcast "hi" message to all connected websocket clients.
  // Traverse over all connections
  for (struct mg_connection *c = mgr->conns; c != NULL; c = c->next) {
    // Send only to marked connections
    if (c->label[0] == 'W') mg_ws_send(c, "hi", 2, WEBSOCKET_OP_TEXT);
  }
}

int main(void) {
  struct mg_mgr mgr;  // Event manager
  mg_mgr_init(&mgr);  // Initialise event manager
  mg_timer_add(&mgr, 300, MG_TIMER_REPEAT, timer_fn, &mgr);  // Init timer
  mg_http_listen(&mgr, s_listen_on, fn, NULL);  // Create HTTP listener
  for (;;) mg_mgr_poll(&mgr, 1000);             // Infinite event loop
  mg_mgr_free(&mgr);                            // Free manager resources
  return 0;
}
