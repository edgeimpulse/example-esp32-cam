#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "img_converters.h"
#include "image_util.h"
#include "camera_index.h"
//#include "camera_index_html.h"
#include "Arduino.h"

#include "fb_gfx.h"
#include "fd_forward.h"
//#include "dl_lib.h"
#include "fr_forward.h"

/* Modify the following line according to your project name
   Do not forget to import the library using "Sketch">"Include Library">"Add .ZIP Library..."
*/
#include <Car_Detection_inferencing.h>

uint8_t *out_buf;
uint8_t *ei_buf;

static int8_t ei_activate = 0;
dl_matrix3du_t *resized_matrix = NULL;
size_t out_len = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
ei_impulse_result_t result = {0};

typedef struct
{
  size_t size;  //number of values used for filtering
  size_t index; //current value index
  size_t count; //value count
  int sum;
  int *values; //array to be filled with values
} ra_filter_t;

typedef struct
{
  httpd_req_t *req;
  size_t len;
} jpg_chunking_t;

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static ra_filter_t ra_filter;
httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;

static ra_filter_t *ra_filter_init(ra_filter_t *filter, size_t sample_size)
{
  memset(filter, 0, sizeof(ra_filter_t));

  filter->values = (int *)malloc(sample_size * sizeof(int));
  if (!filter->values)
  {
    return NULL;
  }
  memset(filter->values, 0, sample_size * sizeof(int));

  filter->size = sample_size;
  return filter;
}

static int ra_filter_run(ra_filter_t *filter, int value)
{
  if (!filter->values)
  {
    return value;
  }
  filter->sum -= filter->values[filter->index];
  filter->values[filter->index] = value;
  filter->sum += filter->values[filter->index];
  filter->index++;
  filter->index = filter->index % filter->size;
  if (filter->count < filter->size)
  {
    filter->count++;
  }
  return filter->sum / filter->count;
}

int raw_feature_get_data(size_t offset, size_t out_len, float *signal_ptr)
{
  size_t pixel_ix = offset * 3;
  size_t bytes_left = out_len;
  size_t out_ptr_ix = 0;

  // read byte for byte
  while (bytes_left != 0) {
    // grab the values and convert to r/g/b
    uint8_t r, g, b;
    r = ei_buf[pixel_ix];
    g = ei_buf[pixel_ix + 1];
    b = ei_buf[pixel_ix + 2];

    // then convert to out_ptr format
    float pixel_f = (r << 16) + (g << 8) + b;
    signal_ptr[out_ptr_ix] = pixel_f;

    // and go to the next pixel
    out_ptr_ix++;
    pixel_ix += 3;
    bytes_left--;
  }
  return 0;
}

void classify()
{
  Serial.println("Getting signal...");
  signal_t signal;
  signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_WIDTH;
  signal.get_data = &raw_feature_get_data;

  Serial.println("Run classifier...");
  // Feed signal to the classifier
  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false /* debug */);

  // Returned error variable "res" while data object.array in "result"
  ei_printf("run_classifier returned: %d\n", res);
  if (res != 0)
    return;

  // print the predictions
  ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
            result.timing.dsp, result.timing.classification, result.timing.anomaly);
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    ei_printf("    %s: \t%f\r\n", result.classification[ix].label, result.classification[ix].value);
  }
#if EI_CLASSIFIER_HAS_ANOMALY == 1
  ei_printf("    anomaly score: %f\r\n", result.anomaly);
#endif
}

static size_t jpg_encode_stream(void *arg, size_t index, const void *data, size_t len)
{
  jpg_chunking_t *j = (jpg_chunking_t *)arg;
  if (!index)
  {
    j->len = 0;
  }
  if (httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK)
  {
    return 0;
  }
  j->len += len;
  return len;
}

static esp_err_t inference_handler(httpd_req_t *req)
{
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  int64_t fr_start = esp_timer_get_time();

  size_t out_len, out_width, out_height;
  size_t ei_len;

  fb = esp_camera_fb_get();
  if (!fb)
  {
    Serial.println("Camera capture failed");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg"); //response header and capture page

  bool s;
  bool detected = false;

  dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
  if (!image_matrix)
  {
    esp_camera_fb_return(fb);
    Serial.println("dl_matrix3du_alloc failed");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  out_buf = image_matrix->item;
  out_len = fb->width * fb->height * 3;
  out_width = fb->width;
  out_height = fb->height;

  Serial.println("Converting to RGB888...");
  int64_t time_start = esp_timer_get_time();
  s = fmt2rgb888(fb->buf, fb->len, fb->format, out_buf);
  int64_t time_end = esp_timer_get_time();
  Serial.printf("Done in %ums\n", (uint32_t)((time_end - time_start) / 1000));

  esp_camera_fb_return(fb);
  if (!s)
  {
    dl_matrix3du_free(image_matrix);
    Serial.println("to rgb888 failed");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  dl_matrix3du_t *ei_matrix = dl_matrix3du_alloc(1, EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT, 3);
  if (!ei_matrix)
  {
    esp_camera_fb_return(fb);
    Serial.println("dl_matrix3du_alloc failed");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  ei_buf = ei_matrix->item;
  ei_len = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT * 3;

  Serial.println("Resizing the frame buffer...");
  time_start = esp_timer_get_time();
  image_resize_linear(ei_buf, out_buf, EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT, 3, out_width, out_height);
  time_end = esp_timer_get_time();
  Serial.printf("Done in %ums\n", (uint32_t)((time_end - time_start) / 1000));

  dl_matrix3du_free(image_matrix);

  classify();

  jpg_chunking_t jchunk = {req, 0};
  s = fmt2jpg_cb(ei_buf, ei_len, EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT, PIXFORMAT_RGB888, 90, jpg_encode_stream, &jchunk);

  if (!s)
  {
    Serial.println("JPEG compression failed");
    return ESP_FAIL;
  }

  dl_matrix3du_free(ei_matrix);

  int64_t fr_end = esp_timer_get_time();
  Serial.printf("JPG: %uB %ums\n", (uint32_t)(jchunk.len), (uint32_t)((fr_end - fr_start) / 1000));
  return res;
}



// Not needed for Edge Impulse inference, here as an example or for debugging
static esp_err_t capture_handler(httpd_req_t *req)
{
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  int64_t fr_start = esp_timer_get_time();

  fb = esp_camera_fb_get();
  if (!fb)
  {
    Serial.println("Camera capture failed");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg"); //antwoord header an capture pagina

  size_t out_len, out_width, out_height;
  uint8_t *out_buf;
  bool s;
  bool detected = false;

  if (fb->width > 400)
  {
    size_t fb_len = 0;
    if (fb->format == PIXFORMAT_JPEG)
    {
      fb_len = fb->len;
      res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
    }
    else
    {
      jpg_chunking_t jchunk = {req, 0};
      res = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk) ? ESP_OK : ESP_FAIL;
      httpd_resp_send_chunk(req, NULL, 0);
      fb_len = jchunk.len;
    }
    esp_camera_fb_return(fb);
    int64_t fr_end = esp_timer_get_time();
    Serial.printf("JPG: %uB %ums\n", (uint32_t)(fb_len), (uint32_t)((fr_end - fr_start) / 1000));
    return res;
  }

  dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
  if (!image_matrix)
  {
    esp_camera_fb_return(fb);
    Serial.println("dl_matrix3du_alloc failed");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  out_buf = image_matrix->item;
  out_len = fb->width * fb->height * 3;
  out_width = fb->width;
  out_height = fb->height;

  s = fmt2rgb888(fb->buf, fb->len, fb->format, out_buf);
  esp_camera_fb_return(fb);
  if (!s)
  {
    dl_matrix3du_free(image_matrix);
    Serial.println("to rgb888 failed");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  jpg_chunking_t jchunk = {req, 0};
  s = fmt2jpg_cb(out_buf, out_len, out_width, out_height, PIXFORMAT_RGB888, 90, jpg_encode_stream, &jchunk);
  dl_matrix3du_free(image_matrix);
  if (!s)
  {
    Serial.println("JPEG compression failed");
    return ESP_FAIL;
  }

  int64_t fr_end = esp_timer_get_time();
  Serial.printf("FACE: %uB %ums\n", (uint32_t)(jchunk.len), (uint32_t)((fr_end - fr_start) / 1000));
  return res;
}

// Not need for Edge Impulse inference, here as an example
static esp_err_t stream_handler(httpd_req_t *req)
{
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t *_jpg_buf = NULL;
  char *part_buf[64];
  dl_matrix3du_t *image_matrix = NULL;
  bool detected = false;

  int64_t fr_start = 0;
  int64_t fr_ready = 0;
  int64_t fr_encode = 0;

  static int64_t last_frame = 0;
  if (!last_frame)
  {
    last_frame = esp_timer_get_time();
  }

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK)
  {
    return res;
  }

  while (true)
  {
    detected = false;
    fb = esp_camera_fb_get();
    if (!fb)
    {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    }
    else
    {
      fr_start = esp_timer_get_time();
      fr_ready = fr_start;

      if (fb->width > 400)
      {
        if (fb->format != PIXFORMAT_JPEG)
        {
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if (!jpeg_converted)
          {
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        }
        else
        {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
      else
      {

        image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);

        if (!image_matrix)
        {
          Serial.println("dl_matrix3du_alloc failed");
          res = ESP_FAIL;
        }
        else
        {
          if (!fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item))
          {
            Serial.println("fmt2rgb888 failed");
            res = ESP_FAIL;
          }
          else
          {
            fr_ready = esp_timer_get_time();

            if (fb->format != PIXFORMAT_JPEG)
            {
              if (!fmt2jpg(image_matrix->item, fb->width * fb->height * 3, fb->width, fb->height, PIXFORMAT_RGB888, 90, &_jpg_buf, &_jpg_buf_len))
              {
                Serial.println("fmt2jpg failed");
                res = ESP_FAIL;
              }
              esp_camera_fb_return(fb);
              fb = NULL;
            }
            else
            {
              _jpg_buf = fb->buf;
              _jpg_buf_len = fb->len;
            }
            fr_encode = esp_timer_get_time();
          }
          dl_matrix3du_free(image_matrix);
        }
      }
    }
    if (res == ESP_OK)
    {
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if (res == ESP_OK)
    {
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if (res == ESP_OK)
    {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if (fb)
    {
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    }
    else if (_jpg_buf)
    {
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if (res != ESP_OK)
    {
      break;
    }
    int64_t fr_end = esp_timer_get_time();

    int64_t ready_time = (fr_ready - fr_start) / 1000;

    int64_t frame_time = fr_end - last_frame;
    last_frame = fr_end;
    frame_time /= 1000;
    uint32_t avg_frame_time = ra_filter_run(&ra_filter, frame_time);
    Serial.printf("MJPG: %uB %ums (%.1ffps), AVG: %ums (%.1ffps)\n",
                  (uint32_t)(_jpg_buf_len),
                  (uint32_t)frame_time,
                  1000.0 / (uint32_t)frame_time,
                  avg_frame_time,
                  1000.0 / avg_frame_time);
  }

  last_frame = 0;
  return res;
}

static esp_err_t cmd_handler(httpd_req_t *req)
{
  char *buf;
  size_t buf_len;
  char variable[32] = {
    0,
  };
  char value[32] = {
    0,
  };

  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1)
  {
    buf = (char *)malloc(buf_len);
    if (!buf)
    {
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
    {
      if (httpd_query_key_value(buf, "var", variable, sizeof(variable)) == ESP_OK &&
          httpd_query_key_value(buf, "val", value, sizeof(value)) == ESP_OK)
      {
      }
      else
      {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
      }
    }
    else
    {
      free(buf);
      httpd_resp_send_404(req);
      return ESP_FAIL;
    }
    free(buf);
  }
  else
  {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  int val = atoi(value);
  sensor_t *s = esp_camera_sensor_get();
  int res = 0;

  if (!strcmp(variable, "framesize"))
  {

    if (s->pixformat == PIXFORMAT_JPEG)
      res = s->set_framesize(s, (framesize_t)val);
  }
  else if (!strcmp(variable, "quality"))
    res = s->set_quality(s, val);
  else if (!strcmp(variable, "contrast"))
    res = s->set_contrast(s, val);
  else if (!strcmp(variable, "brightness"))
    res = s->set_brightness(s, val);
  else if (!strcmp(variable, "saturation"))
    res = s->set_saturation(s, val);
  else if (!strcmp(variable, "gainceiling"))
    res = s->set_gainceiling(s, (gainceiling_t)val);
  else if (!strcmp(variable, "colorbar"))
    res = s->set_colorbar(s, val);
  else if (!strcmp(variable, "awb"))
    res = s->set_whitebal(s, val);
  else if (!strcmp(variable, "agc"))
    res = s->set_gain_ctrl(s, val);
  else if (!strcmp(variable, "aec"))
    res = s->set_exposure_ctrl(s, val);
  else if (!strcmp(variable, "hmirror"))
    res = s->set_hmirror(s, val);
  else if (!strcmp(variable, "vflip"))
    res = s->set_vflip(s, val);
  else if (!strcmp(variable, "awb_gain"))
    res = s->set_awb_gain(s, val);
  else if (!strcmp(variable, "agc_gain"))
    res = s->set_agc_gain(s, val);
  else if (!strcmp(variable, "aec_value"))
    res = s->set_aec_value(s, val);
  else if (!strcmp(variable, "aec2"))
    res = s->set_aec2(s, val);
  else if (!strcmp(variable, "dcw"))
    res = s->set_dcw(s, val);
  else if (!strcmp(variable, "bpc"))
    res = s->set_bpc(s, val);
  else if (!strcmp(variable, "wpc"))
    res = s->set_wpc(s, val);
  else if (!strcmp(variable, "raw_gma"))
    res = s->set_raw_gma(s, val);
  else if (!strcmp(variable, "lenc"))
    res = s->set_lenc(s, val);
  else if (!strcmp(variable, "special_effect"))
    res = s->set_special_effect(s, val);
  else if (!strcmp(variable, "wb_mode"))
    res = s->set_wb_mode(s, val);
  else if (!strcmp(variable, "ae_level"))
  {

    Serial.printf("var=");
    Serial.printf(variable);

    res = s->set_ae_level(s, val);
    Serial.printf(" valvar=");
    Serial.print(val);
  }

  else if (!strcmp(variable, "ei_activate"))
    ei_activate = val;

  else
  {
    res = -1;
  }

  if (res)
  {
    return httpd_resp_send_500(req);
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);
}

static esp_err_t status_handler(httpd_req_t *req)
{
  static char json_response[1024];

  sensor_t *s = esp_camera_sensor_get();
  char *p = json_response;
  *p++ = '{';

  p += sprintf(p, "\"framesize\":%u,", s->status.framesize);
  p += sprintf(p, "\"quality\":%u,", s->status.quality);
  p += sprintf(p, "\"brightness\":%d,", s->status.brightness);
  p += sprintf(p, "\"contrast\":%d,", s->status.contrast);
  p += sprintf(p, "\"saturation\":%d,", s->status.saturation);
  p += sprintf(p, "\"special_effect\":%u,", s->status.special_effect);
  p += sprintf(p, "\"wb_mode\":%u,", s->status.wb_mode);
  p += sprintf(p, "\"awb\":%u,", s->status.awb);
  p += sprintf(p, "\"awb_gain\":%u,", s->status.awb_gain);
  p += sprintf(p, "\"aec\":%u,", s->status.aec);
  p += sprintf(p, "\"aec2\":%u,", s->status.aec2);
  p += sprintf(p, "\"ae_level\":%d,", s->status.ae_level);
  p += sprintf(p, "\"aec_value\":%u,", s->status.aec_value);
  p += sprintf(p, "\"agc\":%u,", s->status.agc);
  p += sprintf(p, "\"agc_gain\":%u,", s->status.agc_gain);
  p += sprintf(p, "\"gainceiling\":%u,", s->status.gainceiling);
  p += sprintf(p, "\"bpc\":%u,", s->status.bpc);
  p += sprintf(p, "\"wpc\":%u,", s->status.wpc);
  p += sprintf(p, "\"raw_gma\":%u,", s->status.raw_gma);
  p += sprintf(p, "\"lenc\":%u,", s->status.lenc);
  p += sprintf(p, "\"vflip\":%u,", s->status.vflip);
  p += sprintf(p, "\"hmirror\":%u,", s->status.hmirror);
  p += sprintf(p, "\"dcw\":%u,", s->status.dcw);
  p += sprintf(p, "\"colorbar\":%u,", s->status.colorbar);
  p += sprintf(p, "\"ei_activate\":%u", ei_activate);
  *p++ = '}';
  *p++ = 0;
  Serial.println(json_response);
  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, json_response, strlen(json_response));
}

static esp_err_t inference_results_handler(httpd_req_t *req)
{
  static char json_response[1024] = "";

  char *p = json_response;

  *p++ = '{';

  if (result.classification[0].label)
  {
    p += sprintf(p, "\"success\": true,");
    p += sprintf(p, "\"classification\": [");
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++)
    {
      *p++ = '{';
      p += sprintf(p, "\"label\": \"%s\",", result.classification[ix].label);
      p += sprintf(p, "\"value\":%.5f", result.classification[ix].value);
      *p++ = '}';
      if (ix != EI_CLASSIFIER_LABEL_COUNT - 1)
      {
        *p++ = ',';
      }

      //p+=sprintf(p, "\"%s\":%.5f,", result.classification[ix].label, result.classification[ix].value);
    }
    *p++ = ']';
    *p++ = ',';
  }
  else
  {
    p += sprintf(p, "\"success\": false,");
  }
  // human-readable predictions

  p += sprintf(p, "\"timing\": {");
  p += sprintf(p, "\"timing_dsp\":%d,", result.timing.dsp);
  p += sprintf(p, "\"timing_classification\":%d,", result.timing.classification);
  p += sprintf(p, "\"timing_anomaly\":%d", result.timing.anomaly);

  *p++ = '}';
  *p++ = '}';
  *p++ = 0;
  Serial.println(json_response);

  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  ei_impulse_result_t result = {0};

  return httpd_resp_send(req, json_response, strlen(json_response));
}

static esp_err_t index_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/html");
  Serial.printf("webpage loading");

  // httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
  return httpd_resp_send(req, (const char *)index_custom_html, index_custom_html_len);
}

void startCameraServer()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  httpd_uri_t index_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = index_handler,
    .user_ctx = NULL
  };

  httpd_uri_t status_uri = {
    .uri = "/status",
    .method = HTTP_GET,
    .handler = status_handler,
    .user_ctx = NULL
  };

  httpd_uri_t cmd_uri = {
    .uri = "/control",
    .method = HTTP_GET,
    .handler = cmd_handler,
    .user_ctx = NULL
  };

  httpd_uri_t capture_uri = {
    .uri = "/capture", //v12345vtm
    .method = HTTP_GET,
    .handler = capture_handler,
    .user_ctx = NULL
  };

  httpd_uri_t inference_uri = {
    .uri = "/inference",
    .method = HTTP_GET,
    .handler = inference_handler,
    .user_ctx = NULL
  };

  httpd_uri_t inference_results_uri = {
    .uri = "/inference-results",
    .method = HTTP_GET,
    .handler = inference_results_handler,
    .user_ctx = NULL
  };

  httpd_uri_t stream_uri = {
    .uri = "/stream",
    .method = HTTP_GET,
    .handler = stream_handler,
    .user_ctx = NULL
  };

  ra_filter_init(&ra_filter, 20);

  Serial.printf("Starting web server on port: '%d'\n", config.server_port);
  if (httpd_start(&camera_httpd, &config) == ESP_OK)
  {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &cmd_uri);
    httpd_register_uri_handler(camera_httpd, &status_uri);
    httpd_register_uri_handler(camera_httpd, &capture_uri);
    httpd_register_uri_handler(camera_httpd, &inference_uri);
    httpd_register_uri_handler(camera_httpd, &inference_results_uri);
  }

  config.server_port += 1;
  config.ctrl_port += 1;

  config.server_port = 9601; //stream port + also change this in the html-source in this file
  // config.ctrl_port =8081;

  Serial.printf("Starting stream server on stream port: '%d'\n", config.server_port);
  if (httpd_start(&stream_httpd, &config) == ESP_OK)
  {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}
