
#include <Arduino.h>
#include <WiFi.h>
#include "qrcode.h"
#include <LittleFS.h>
#include "netdisk.h"

/**
 * Enter your WiFi credentials
 */
const char *ssid = "";
const char *password = "";

/**
 * Enter your APP credentials
 * 
 * Refer to (documentation)[https://pan.baidu.com/union/doc/fl0hhnulu],
 * create an application, obtain the following information and fill in it.
 */
const char *appKey = "";
const char *secretKey = "";
const char *scope = "";
const char *appName = "";

File f;
NetDisk xpan;

void setup()
{
    Serial.begin(115200);
    delay(10);

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    if (!LittleFS.begin())
    {
        Serial.printf("An Error has occurred while mounting LittleFS\n");
        return;
    }

    f = LittleFS.open("/logo.jpg", "r");

    xpan.oauthInit(appKey, secretKey, scope, appName);
    xpan.authorize([](NetDisk::getDeviceCodeRsp rsp) {
            Serial.println("user_code: " + rsp.user_code);
            qrcode_display(rsp.verification_url.c_str());
        }, 300);
}


void loop()
{
    xpan.upload(&f, 60);
    delay(60 * 1000);
}
