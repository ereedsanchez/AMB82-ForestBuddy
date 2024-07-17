// designed for the esp32-cam
#include <WiFi.h>
#include <WiFiClient.h> 
#include "esp_camera.h"
#define CAMERA_MODEL_ESP_EYE
#include <ESP32_FTPClient.h>
#include "camera_pins.h"


// WiFi credentials
#define WIFI_SSID "NET-MESH-FOREST"
#define WIFI_PASS ""

char ftp_server[] = "192.168.10.221";
char ftp_user[]   = "Test";
char ftp_pass[]   = "";

void startCameraServer();
void setupLedFlash(int pin);

ESP32_FTPClient ftp (ftp_server,ftp_user,ftp_pass, 5000, 2);

// Function to take picture and upload
void takePictureAndUpload() {
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  ftp.OpenConnection();
  // Connect to FTP server
  while (ftp.isConnected() == true) {
    // Upload the image
    ftp.InitFile("Type A");
    ftp.MakeDir("Images");
    ftp.ChangeWorkDir("Images");
    ftp.NewFile("Type I");
    ftp.WriteData(fb->buf, fb->len);
    ftp.CloseFile();
    ftp.CloseConnection();
    break;
  }
  else{
    Serial.println("Connection failed... retrying in 3 seconds.");
    delay(3000);
  }

  // Release the camera buffer
  esp_camera_fb_return(fb);
}

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  Serial.println("connecting WiFi...");
  WiFi.begin( WIFI_SSID, WIFI_PASS );
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi");
  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize camera
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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;  // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

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
  // Start taking pictures every minute
  while (true) {
    takePictureAndUpload();
    delay(60000); // 60 seconds delay
  }
}

void loop() {
  takePictureAndUpload();
  delay(60000);
}
