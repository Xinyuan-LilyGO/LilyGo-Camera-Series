#include "netdisk.h"
#include "requests.h"
#include "utils.h"
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <MD5Builder.h>

void NetDisk::oauthInit(String appKey, String secretKey, String scope, String appName)
{
    _appKey = appKey;
    _secretKey = secretKey;
    _scope = scope;
    _appName = appName;
}


void NetDisk::oauthInit(const char *appKey, const char *secretKey, const char *scope, const char *appName)
{
    _appKey = appKey;
    _secretKey = secretKey;
    _scope = scope;
    _appName = appName;
}


bool NetDisk::getDeviceCode(getDeviceCodeRsp *rsp)
{
    bool ret = false;
    HTTPClient http;
    WiFiClient client;
    DynamicJsonDocument doc(1024);
    String url = String("https://openapi.baidu.com/oauth/2.0/device/code") + \
                 String("?response_type=device_code") + \
                 String("&client_id=") + _appKey + \
                 String("&scope=basic,netdisk");

    http.begin(url);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
        DeserializationError error = deserializeJson(doc, http.getStream());
        if (error)
        {
            log_e("deserializeJson() failed: %s", error.c_str());
            goto out;
        }

        // {
        //     "device_code": "eeb5ef56b38c422f5b6f0521f3ff4bf5",
        //     "user_code": "6ebgdke7",
        //     "verification_url": "https://openapi.baidu.com/device",
        //     "qrcode_url": "https://openapi.baidu.com/device/qrcode/078599be4b2feea400e3fed6bba9e42d/6ebgdke7",
        //     "expires_in": 300,
        //     "interval": 5
        // }
        rsp->device_code = doc["device_code"].as<const char *>();
        _device_code = doc["device_code"].as<const char *>();
        rsp->user_code = doc["user_code"].as<const char *>();
        rsp->verification_url = doc["verification_url"].as<const char *>();
        rsp->qrcode_url = doc["qrcode_url"].as<const char *>();
        rsp->expires_in = doc["expires_in"].as<int>();
        rsp->interval = doc["interval"].as<int>();

        // qrcode_display(doc["verification_url"].as<const char *>());
        ret = true;
    }
    else
    {
        log_e("connection failed, error: %s", http.errorToString(httpCode).c_str());
    }

out:
    client.stop();
    http.end();
    return ret;
}


bool NetDisk::getAccessToken()
{
    bool ret = false;
    HTTPClient http;
    WiFiClient client;
    DynamicJsonDocument doc(1024);
    String url = String("https://openapi.baidu.com/oauth/2.0/token") + \
                 String("?grant_type=device_token") + \
                 String("&code=") + _device_code + \
                 String("&client_id=") + _appKey + \
                 String("&client_secret=") + _secretKey;

    http.begin(url);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
        DeserializationError error = deserializeJson(doc, http.getStream());
        if (error)
        {
            log_e("deserializeJson() failed: %s", error.c_str());
            goto out;
        }

        // {
        //     "expires_in": 2592000,
        //     "refresh_token": "127.1b6ea930a9c3e4c475e69ea816a3f545.YGpVkvFRvH8l0ongploPIlAWDZ-LNQnzulyqseS.0V8OFw",
        //     "access_token": "126.f5e8e5126efef78e7e8ca0f84e78c216.YaokBZZN9SZTz7y3-81PzSVDqECb67EiJM4QJQD.cWgG8g",
        //     "session_secret": "",
        //     "session_key": "",
        //     "scope": "basic netdisk"
        // }
        _access_token = doc["access_token"].as<const char *>();
        _refresh_token = doc["refresh_token"].as<const char *>();
        _session_secret = doc["session_secret"].as<const char *>();
        _session_key = doc["session_key"].as<const char *>();
        _expires_in = doc["expires_in"].as<int>();
        _scope = doc["scope"].as<const char *>();
        log_i("access_token: %s", _access_token.c_str());
        ret = true;
    }
    else
    {
        log_e("connection failed, error: %s", http.errorToString(httpCode).c_str());
    }

out:
    client.stop();
    http.end();
    return ret;
}


bool NetDisk::refreshAccessToken()
{
    bool ret = false;
    HTTPClient http;
    WiFiClient client;
    DynamicJsonDocument doc(1024);
    String url = String("https://openapi.baidu.com/oauth/2.0/token") + \
                 String("?grant_type=refresh_token") + \
                 String("&refresh_token=") + _refresh_token + \
                 String("&client_id=") + _appKey +
                 String("&client_secret=") + _secretKey;

    http.begin(url);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
        DeserializationError error = deserializeJson(doc, http.getStream());
        if (error)
        {
            log_e("deserializeJson() failed: %s", error.c_str());
            goto out;
        }
        // {
        //     "expires_in": 2592000,
        //     "refresh_token": "127.1b6ea930a9c3e4c475e69ea816a3f545.YGpVkvFRvH8l0ongploPIlAWDZ-LNQnzulyqseS.0V8OFw",
        //     "access_token": "126.f5e8e5126efef78e7e8ca0f84e78c216.YaokBZZN9SZTz7y3-81PzSVDqECb67EiJM4QJQD.cWgG8g",
        //     "session_secret": "",
        //     "session_key": "",
        //     "scope": "basic netdisk"
        // }
        _access_token = doc["access_token"].as<const char *>();
        _refresh_token = doc["refresh_token"].as<const char *>();
        _session_secret = doc["session_secret"].as<const char *>();
        _session_key = doc["session_key"].as<const char *>();
        _expires_in = doc["expires_in"].as<int>();
        // scope = doc["scope"].as<const char *>();

        log_i("access_token: %s", _access_token.c_str());
        ret = true;
    }
    else
    {
        log_e("connection failed, error: %s", http.errorToString(httpCode).c_str());
    }

out:
    client.stop();
    http.end();
    return ret;
}


/**
 *  *********                       *********                          *********
 *  * Phone *                       * ESP32 *                          * OAuth *
 *  *********                       *********                          *********
 *     |                                |                                     |
 *     |                                |                                     |
 *     |                                | 1. Request device code, user code   |
 *     |                                |------------------------------------>|
 *     |                                |                                     |
 *     |                                | 2. Response device code, user code  |
 *     |                                |<------------------------------------|
 *     |                                |                                     |
 *     |                                | 3. Generate QR code url             |
 *     |                                |<------------------------------------|
 *     |                                |                                     |
 *     |                                |                                     |
 *     | 4. Scan QR code                |                                     |
 *     |--------------------------------------------------------------------->|
 *     |                                |                                     |
 *     | 5. Back to authorization page  |                                     |
 *     |<---------------------------------------------------------------------|
 *     |                                |                                     |
 *     | 6. Login and authorize         |                                     |
 *     |--------------------------------------------------------------------->|
 *     |                                |                                     |
 *     |                                | 7. Polling Request Access Token     |
 *     |                                |------------------------------------>|
 *     |                                |                                     |
 *     |                                | 8. Response Access Token            |
 *     |                                |<------------------------------------|
 */
bool NetDisk::authorize(authorizehook hook, int timeout)
{
    int status = E_AUTH_STATUS_GET_DEVICE_CODE;
    int timeout_count = 0;
    int expires_count = 0;
    getDeviceCodeRsp rsp;

    while (1)
    {
        switch (status)
        {
            case E_AUTH_STATUS_GET_DEVICE_CODE:
                if (getDeviceCode(&rsp))
                {
                    hook(rsp);
                    status = E_AUTH_STATUS_GET_ACCESS_TOKEN;
                }
                else if (timeout_count++ < timeout)
                {
                    delay(1000);
                }
                else
                {
                    return false;
                }
            break;

            case E_AUTH_STATUS_GET_ACCESS_TOKEN:
                if (getAccessToken())
                {
                    return true;
                }
                else
                {
                    if ((expires_count++ < (rsp.expires_in/rsp.interval)) && \
                        (timeout_count < timeout))
                    {
                        delay(rsp.interval * 1000);
                        timeout_count += rsp.interval;
                    }
                    else
                    {
                        return false;
                    }
                }
            break;

            default:
                delay(100);
            break;
        }
    }

    return false;
}


bool NetDisk::precreate()
{
    bool ret = false;
    Requests http;
    WiFiClientSecure client;
    DynamicJsonDocument doc(1024);

    log_i("precreate");
    client.setCACert(rootCACertificate);

    String url = String("https://pan.baidu.com/rest/2.0/xpan/file") + \
                 String("?method=precreate") + \
                 String("&access_token=") + _access_token;

    httpRequestData payload[] = {
        {
            .key = "path",
            .value = _path
        },
        {
            .key = "size",
            .value = String(_size)
        },
        {
            .key = "isdir",
            .value = "0"
        },
        {
            .key = "block_list",
            .value = String("[\"" + _md5 + "\"]")
        },
        {
            .key = "autoinit",
            .value = "1"
        }
    };

    http.begin(client, url);
    int httpCode = http.request("POST", payload, sizeof(payload)/sizeof(httpRequestData), nullptr, 0);
    if (httpCode == HTTP_CODE_OK)
    {
        String rsp = http.getString();
        log_i("http rsp: %s", rsp.c_str());
        DeserializationError error = deserializeJson(doc, rsp);
        if (error)
        {
            log_e("deserializeJson() failed: %s", error.c_str());
            goto out;
        }
        else
        {
            ret = doc["errno"].as<int>() == 0 ? true : false;
            // path = doc["path"].as<const char *>();
            _uploadid = doc["uploadid"].as<const char *>();
            // int return_type = doc["return_type"].as<int>();
            // String block_list = doc["block_list"][].as<int>();
            // request_id = doc["request_id"].as<const char *>();
        }
    }
    else
    {
        log_e("HTTP Code: %d - (%s)", httpCode, http.errorToString(httpCode).c_str());
    }

out:
    client.stop();
    http.end();
    return ret;
}


bool NetDisk::uploadBlock(File *f)
{
    bool ret = false;
    Requests http;
    WiFiClientSecure client;
    DynamicJsonDocument doc(1024);
    char filename[128] = { 0 };

    log_i("uploadBlock");

    urlEncode("/apps/test/epd47.jpg", strlen("/apps/test/epd47.jpg"), filename, 128);

    String url = String("https://d.pcs.baidu.com/rest/2.0/pcs/superfile2") + \
                 String("?method=upload") + \
                 String("&access_token=") + _access_token + \
                 String("&type=tmpfile") + \
                 String("&path=") + String(filename) + \
                 String("&uploadid=") + _uploadid + \
                 String("&partseq=0");
    log_i("http url: %s", url.c_str());

    f->seek(0, SeekSet);
    httpRequestFiles files = {
        .key = "file",
        .filename = f->name(),
        .stream = f,
        .buffer = nullptr,
        .size = f->size(),
        .type = "image/jpeg"
    };

    client.setCACert(rootCACertificate1);
    http.begin(client, url);
    int httpCode = http.request("POST", nullptr, 0, &files, sizeof(files)/sizeof(httpRequestFiles));
    if (httpCode == HTTP_CODE_OK)
    {
        String rsp = http.getString();
        log_i("http rsp: %s", rsp.c_str());
        DeserializationError error = deserializeJson(doc, rsp);
        if (error)
        {
            log_e("deserializeJson() failed: %s", error.c_str());
            goto out;
        }
        else
        {
            String cur_md5 = doc["md5"].as<const char *>();
            // String cur_request_id = doc["request_id"].as<const char *>();

            if (cur_md5 == _md5)
            {
                ret = true;
            }
            else
            {
                log_w("md5 checksum error");
            }
        }
    }
    else
    {
        log_e("connection failed, error: %d - (%s)\n", httpCode, http.errorToString(httpCode).c_str());
    }

out:
    client.stop();
    http.end();
    return ret;
}


bool NetDisk::uploadBlock(uint8_t *buf, size_t size)
{
    bool ret = false;
    Requests http;
    WiFiClientSecure client;
    DynamicJsonDocument doc(1024);
    char filename[128] = { 0 };

    log_i("uploadBlock");

    urlEncode("/apps/test/epd47.jpg", strlen("/apps/test/epd47.jpg"), filename, 128);

    String url = String("https://d.pcs.baidu.com/rest/2.0/pcs/superfile2") + \
                 String("?method=upload") + \
                 String("&access_token=") + _access_token + \
                 String("&type=tmpfile") + \
                 String("&path=") + String(filename) + \
                 String("&uploadid=") + _uploadid + \
                 String("&partseq=0");
    log_i("http url: %s", url.c_str());

    httpRequestFiles files = {
        .key = "file",
        .filename = _path.substring(_path.lastIndexOf("/"), _path.length()),
        .stream = nullptr,
        .buffer = buf,
        .size = size,
        .type = "image/jpeg"
    };

    client.setCACert(rootCACertificate1);
    http.begin(client, url);
    int httpCode = http.request("POST", nullptr, 0, &files, sizeof(files)/sizeof(httpRequestFiles));
    if (httpCode == HTTP_CODE_OK)
    {
        String rsp = http.getString();
        log_i("http rsp: %s", rsp.c_str());
        DeserializationError error = deserializeJson(doc, rsp);
        if (error)
        {
            log_e("deserializeJson() failed: %s", error.c_str());
            goto out;
        }
        else
        {
            String cur_md5 = doc["md5"].as<const char *>();
            // String cur_request_id = doc["request_id"].as<const char *>();

            if (cur_md5 == _md5)
            {
                ret = true;
            }
            else
            {
                log_w("md5 checksum error");
            }
        }
    }
    else
    {
        log_e("connection failed, error: %d - (%s)\n", httpCode, http.errorToString(httpCode).c_str());
    }

out:
    client.stop();
    http.end();
    return ret;
}


bool NetDisk::create()
{
    bool ret = false;
    HTTPClient http;
    WiFiClientSecure client;
    DynamicJsonDocument doc(1024);

    String url = String("https://pan.baidu.com/rest/2.0/xpan/file") + \
                 String("?method=create") + \
                 String("&access_token=") + _access_token;

    String httpRequestData = String("path=") + _path + \
                             String("&size=") + String(_size) + \
                             String("&isdir=0") + \
                             String("&block_list=[\"") + _md5 + String("\"]") + \
                             String("&uploadid=") + _uploadid;

    log_i("create");
    log_i("Request Data: %s", httpRequestData.c_str());

    client.setCACert(rootCACertificate);
    http.begin(client, url);
    int httpCode = http.POST(httpRequestData);
    if (httpCode == HTTP_CODE_OK)
    {
        String rsp = http.getString();
        log_i("http rsp: %s", rsp.c_str());
        DeserializationError error = deserializeJson(doc, rsp);
        if (error)
        {
            log_e("deserializeJson() failed: %s", error.c_str());
            goto out;
        }
        else
        {
            ret = doc["errno"].as<int>() == 0 ? true : false;
            // path = doc["path"].as<const char *>();
            // uploadid = doc["uploadid"].as<const char *>();
            // int return_type = doc["return_type"].as<int>();
            // String block_list = doc["block_list"][].as<int>();
        }
    }
    else
    {
        log_e("connection failed, error: %s", http.errorToString(httpCode).c_str());
    }

out:
    client.stop();
    http.end();
    return ret;
}


bool NetDisk::upload(File *f, int timeout, String filename)
{
    int status = 0;
    int count = 0;

    if (!f)
    {
        log_e("Invalid file handle");
        return false;
    }

    if (!f->size() > 4 * 1024 * 1024)
    {
        log_w("Uploading files over 4M is not supported");
        return false;
    }

    if (filename.length() != 0)
    {
        _path = String("/apps/") + _appName + "/" + filename;
    }
    else
    {
        _path = String("/apps/") + _appName + "/" + f->name();
    }
    _size = f->size();

    while (1)
    {
        switch (status)
        {
            case 0:
                MD5Builder b;
                f->seek(0, SeekSet);
                b.begin();
                b.addStream(*f, f->size());
                b.calculate();
                _md5 = b.toString();
                status++;
            break;

            case 1:
                if (precreate())
                {
                    status++;
                }
                else
                {
                    delay(200);
                }
            break;

            case 2:
                if (uploadBlock(f))
                {
                    status++;
                }
                else
                {
                    delay(200);
                }
            break;

            case 3:
                if (create())
                {
                    // status++;
                    return true;
                }
                else
                {
                    delay(200);
                }
            break;

            default:
                delay(200);
            break;
        }
        if (count++ > (timeout * 1000)/200)
        {
            // time out
            return false;
        }
    }
    return false;
}


String calculateMD5(uint8_t *buf, size_t size)
{
    MD5Builder b;
    size_t i = 0;

    b.begin();
    if (size > ((1 << 16) - 1))
    {
        for (i = 0; i < size / 4096; i++)
        {
            b.add(&buf[i * 4096], 4096);
        }
        b.add(&buf[i * 4096], size - (i * 4096));
    }
    else
    {
        b.add(buf, size);
    }

    b.calculate();
    return b.toString();
}

bool NetDisk::upload(uint8_t *buf, size_t size, int timeout, String filename)
{
    int status = 0;
    int count = 0;

    if (size == 0)
    {
        log_w("length is too small");
        return false;
    }
    

    if (size > 4 * 1024 * 1024)
    {
        log_w("Uploading files over 4M is not supported");
        return false;
    }

    _size = size;

    if (filename.length() != 0)
    {
        _path = String("/apps/") + _appName + "/" + filename;
    }
    else
    {
        _path = String("/apps/") + _appName + "/" + String("newfile");
    }

    while (1)
    {
        switch (status)
        {
            case 0:
            {
                _md5 = calculateMD5(buf, size);
                log_i("buf md5: %s", _md5.c_str());
                status++;
            }
            break;

            case 1:
                if (precreate())
                {
                    status++;
                }
                else
                {
                    delay(200);
                }
            break;

            case 2:
                if (uploadBlock(buf, size))
                {
                    status++;
                }
                else
                {
                    delay(200);
                }
            break;

            case 3:
                if (create())
                {
                    // status++;
                    return true;
                }
                else
                {
                    delay(200);
                }
            break;

            default:
                delay(200);
            break;
        }
        if (count++ > timeout)
        {
            // time out
            return false;
        }
    }
    return false;
}

