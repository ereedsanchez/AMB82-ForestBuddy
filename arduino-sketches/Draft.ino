/*  This example uses the camera to capture a JPEG image,
    and saves the image to SD Card.

 Example guide:
 https://www.amebaiot.com/en/amebapro2-arduino-video-jpeg-sdcard/
*/
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFi_Udp.h>
#include "VideoStream.h"
#include "AmebaFatFS.h"

#define CHANNEL  0
#define FILENAME "image.jpg"

// needed to retrieve time from local ntp server.
char ssid[] = "Archer";
char password[] = "Uu!W*pu6jrnj";

WiFiUDP ntpUDP;
// Connects to Nyc mesh level 2 ntp server
NTPClient timeclient(ntpUDP, "north-america.pool.ntp.org");

VideoSetting config(VIDEO_FHD, CAM_FPS, VIDEO_JPEG, 1);

uint32_t img_addr = 0;
uint32_t img_len = 0;

AmebaFatFS fs;

void setup()
{
    Serial.begin(115200);

    WiFi.begin(ssid,password);

      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
    timeclient.begin();

    Serial.println("This is a test, time recieved from server is: " + timeclient.getFormattedTime());
    delay(5000);

    Camera.configVideoChannel(CHANNEL, config);
    Camera.videoInit();
    Camera.channelBegin(CHANNEL);

    fs.begin();

}

void loop()
{
    File file = fs.open(String(fs.getRootPath()) + String(FILENAME));

    delay(1000);
    Camera.getImage(CHANNEL, &img_addr, &img_len);
    file.write((uint8_t *)img_addr, img_len);
    file.close();
    fs.end();
    //Serial.println("Picture is in " + fs.readDir("/"));
    delay(60000);
}
