/* Includes ---------------------------------------------------------------- */
#include <Arduino.h>
#include <WiFi.h>

/* Modify the following line according to your project name
   Do not forget to import the library using "Sketch">"Include Library">"Add .ZIP Library..."
*/
#include <Car_Detection_inferencing.h>

#include "esp_http_server.h"
#include "img_converters.h"
#include "image_util.h"
#include "esp_camera.h"

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM

#include "camera_pins.h"

#define PART_BOUNDARY "123456789000000000000987654321"


static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;


const char* ssid = "your-ssid";
const char* password = "your-password";

dl_matrix3du_t *resized_matrix = NULL;
size_t out_len = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
ei_impulse_result_t result = {0};

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_240X240;
  config.jpeg_quality = 12;
  config.fb_count = 1;

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, 0); // lower the saturation
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
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
    r = resized_matrix->item[pixel_ix];
    g = resized_matrix->item[pixel_ix + 1];
    b = resized_matrix->item[pixel_ix + 2];

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

static esp_err_t capture_handler(httpd_req_t *req) {

  esp_err_t res = ESP_OK;
  camera_fb_t * fb = NULL;

  Serial.println("Capture image");
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  // --- Convert frame to RGB888  ---

  Serial.println("Converting to RGB888...");
  // Allocate rgb888_matrix buffer
  dl_matrix3du_t *rgb888_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
  fmt2rgb888(fb->buf, fb->len, fb->format, rgb888_matrix->item);

  // --- Resize the RGB888 frame to 96x96 in this example ---

  Serial.println("Resizing the frame buffer...");
  resized_matrix = dl_matrix3du_alloc(1, EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT, 3);
  image_resize_linear(resized_matrix->item, rgb888_matrix->item, EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT, 3, fb->width, fb->height);

  // --- Free memory ---

  dl_matrix3du_free(rgb888_matrix);
  esp_camera_fb_return(fb);

  classify();

  // --- Convert back the resized RGB888 frame to JPG to send it back to the web app ---

  Serial.println("Converting resized RGB888 frame to JPG...");
  uint8_t * _jpg_buf = NULL;
  fmt2jpg(resized_matrix->item, out_len, EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT, PIXFORMAT_RGB888, 10, &_jpg_buf, &out_len);

  // --- Send response ---

  Serial.println("Sending back HTTP response...");
  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  res = httpd_resp_send(req, (const char *)_jpg_buf, out_len);
  
  // --- Free memory ---
  
  dl_matrix3du_free(resized_matrix);
  free(_jpg_buf);
  _jpg_buf = NULL;

  return res;
}

static esp_err_t page_handler(httpd_req_t *req) {

  httpd_resp_set_type(req, "text/html");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  // httpd_resp_send(req, page, sizeof(page));
}

void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  httpd_uri_t capture_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = capture_handler,
    .user_ctx  = NULL
  };


  Serial.printf("Starting web server on port: '%d'\n", config.server_port);
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &capture_uri);
    //httpd_register_uri_handler(camera_httpd, &page_uri);
  }

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10);
}
