// Copyright (c) 2004-2009 Sergey Lyubka
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
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// Unit test for the mongoose web server. Tests embedded API.


#include "mongoose.h"


#if !defined(LISTENING_PORT)
#define LISTENING_PORT "23456"
#endif

static void send_standard_reply_head(struct mg_connection *conn) {
  mg_connection_must_close(conn);
  // Connection: close is automatically added by mg_write_http_response_head()
  mg_add_response_header(conn, 0, "Content-Type", "text/plain; charset=utf-8");
  mg_write_http_response_head(conn, 200, NULL);
}

static void test_get_var(struct mg_connection *conn) {
  char *var, *buf;
  size_t buf_len;
  const char *cl;
  int var_len;
  const struct mg_request_info *ri = mg_get_request_info(conn);
  int is_form_enc = 0;

  send_standard_reply_head(conn);

  buf_len = 0;
  var = buf = NULL;
  cl = mg_get_header(conn, "Content-Length");
  mg_printf(conn, "cl: %p\n", cl);
  if ((!strcmp(ri->request_method, "POST") ||
       !strcmp(ri->request_method, "PUT"))
      && cl != NULL) {
    buf_len = atoi(cl);
    buf = malloc(buf_len);
    /* Read in two pieces, to test continuation */
    if (buf_len > 2) {
      mg_read(conn, buf, 2);
      mg_read(conn, buf + 2, buf_len - 2);
    } else {
      mg_read(conn, buf, buf_len);
    }
    is_form_enc = 1;
  } else {
    MG_ASSERT(ri->query_string != NULL); // query_string ~ "" when no query string was specified in the request
    buf_len = strlen(ri->query_string);
    buf = malloc(buf_len + 1);
    strcpy(buf, ri->query_string);
    is_form_enc = 1;
  }
  var = malloc(buf_len + 1);
  var_len = mg_get_var(buf, buf_len, "my_var", var, buf_len + 1, is_form_enc);
  mg_printf(conn, "Value: [%s]\n", var);
  mg_printf(conn, "Value size: [%d]\n", var_len);
  free(buf);
  free(var);
}

static void test_get_header(struct mg_connection *conn) {
  const char *value;
  int i;
  const struct mg_request_info *ri = mg_get_request_info(conn);

  send_standard_reply_head(conn);

  printf("HTTP headers: %d\n", ri->num_headers);
  for (i = 0; i < ri->num_headers; i++) {
    printf("[%s]: [%s]\n", ri->http_headers[i].name, ri->http_headers[i].value);
  }

  value = mg_get_header(conn, "Host");
  if (value != NULL) {
    mg_printf(conn, "Value: [%s]", value);
  }
}

static void test_get_request_info(struct mg_connection *conn) {
  int i;
  const struct mg_request_info *ri = mg_get_request_info(conn);

  send_standard_reply_head(conn);

  mg_printf(conn, "Method: [%s]\n", ri->request_method);
  mg_printf(conn, "URI: [%s]\n", ri->uri);
  mg_printf(conn, "HTTP version: [%s]\n", ri->http_version);

  for (i = 0; i < ri->num_headers; i++) {
    mg_printf(conn, "HTTP header [%s]: [%s]\n",
              ri->http_headers[i].name,
              ri->http_headers[i].value);
  }

  MG_ASSERT(ri->query_string != NULL); // query_string ~ "" when no query string was specified in the request
  mg_printf(conn, "Query string: [%s]\n", ri->query_string);
  if (ri->remote_ip.is_ip6)
    mg_printf(conn, "Remote IP: [%x:%x:%x:%x:%x:%x:%x:%x]\n",
            ri->remote_ip.ip_addr.v6[0], ri->remote_ip.ip_addr.v6[1],
            ri->remote_ip.ip_addr.v6[2], ri->remote_ip.ip_addr.v6[3],
            ri->remote_ip.ip_addr.v6[4], ri->remote_ip.ip_addr.v6[5],
            ri->remote_ip.ip_addr.v6[6], ri->remote_ip.ip_addr.v6[7]);
  else
    mg_printf(conn, "Remote IP: [%u.%u.%u.%u]\n",
            ri->remote_ip.ip_addr.v4[0], ri->remote_ip.ip_addr.v4[1],
            ri->remote_ip.ip_addr.v4[2], ri->remote_ip.ip_addr.v4[3]);
  mg_printf(conn, "Remote port: [%d]\n", ri->remote_port);
  mg_printf(conn, "Remote user: [%s]\n",
            ri->remote_user ? ri->remote_user : "");
}

static void test_error(struct mg_connection *conn) {
  const struct mg_request_info *ri = mg_get_request_info(conn);

  mg_connection_must_close(conn);
  // Connection: close is automatically added by mg_write_http_response_head()
  mg_write_http_response_head(conn, 0, NULL);  // 0 ~ use previously set response code

  mg_printf(conn, "Error: [%d]", ri->status_code);
}

static void test_post(struct mg_connection *conn) {
  const char *cl;
  char *buf;
  int len;
  const struct mg_request_info *ri = mg_get_request_info(conn);

  send_standard_reply_head(conn);

  if (strcmp(ri->request_method, "POST") == 0 &&
      (cl = mg_get_header(conn, "Content-Length")) != NULL) {
    len = atoi(cl);
    if ((buf = malloc(len)) != NULL) {
      mg_write(conn, buf, len);
      free(buf);
    }
  }
}

static const struct test_config {
  enum mg_event event;
  const char *uri;
  void (*func)(struct mg_connection *conn);
} test_config[] = {
  {MG_NEW_REQUEST, "/test_get_header", &test_get_header},
  {MG_NEW_REQUEST, "/test_get_var", &test_get_var},
  {MG_NEW_REQUEST, "/test_get_request_info", &test_get_request_info},
  {MG_NEW_REQUEST, "/test_post", &test_post},
  {MG_HTTP_ERROR, "", &test_error},
  {0, NULL, NULL}
};

static void *callback(enum mg_event event,
                      struct mg_connection *conn) {
  const struct mg_request_info *request_info = mg_get_request_info(conn);
  int i;

  for (i = 0; test_config[i].uri != NULL; i++) {
    if (event == test_config[i].event &&
        (event == MG_HTTP_ERROR ||
         !strcmp(request_info->uri, test_config[i].uri))) {
      test_config[i].func(conn);
      return "processed";
    }
  }

  return NULL;
}

int main(void) {
  struct mg_context *ctx;
  const char *options[] = {"listening_ports", LISTENING_PORT, NULL};
  const struct mg_user_class_t ucb = {
    NULL,      // Arbitrary user-defined data
    callback   // User-defined callback function
  };

  ctx = mg_start(&ucb, options);
  while (!mg_get_stop_flag(ctx)) {
    mg_sleep(100);
  }
  mg_stop(ctx);
  return 0;
}
