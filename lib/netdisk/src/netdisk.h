#ifndef NETDISK_H_
#define NETDISK_H_

#include <string>
#include <FS.h>

class NetDisk
{
public:
    enum auth_status { E_AUTH_STATUS_GET_DEVICE_CODE, E_AUTH_STATUS_GET_ACCESS_TOKEN};
    struct getDeviceCodeRsp
    {
        String device_code;
        String user_code;
        String verification_url;
        String qrcode_url;
        int expires_in;
        int interval;
    };
    typedef void authorizehook(getDeviceCodeRsp rsp);

    void oauthInit(String appKey, String secretKey, String scope, String appName);

    void oauthInit(const char *appKey, const char *secretKey, const char *scope, const char *appName);

    bool authorize(authorizehook hook, int timeout=300);

    bool upload(File *f, int timeout = 60, String filename = "");
    bool upload(uint8_t *buf, size_t size = 0, int timeout = 60, String filename = "");

protected:
    bool getDeviceCode(getDeviceCodeRsp *rsp);

    bool getAccessToken();

    bool refreshAccessToken();

    bool precreate();

    bool uploadBlock(File *f);
    bool uploadBlock(uint8_t *buf, size_t size);

    bool create();

    String _appKey;
    String _secretKey;
    String _scope;
    String _appName;

    /** getDeviceCode rsp */
    String _device_code;
    // String _user_code;
    // String _verification_url;
    // String _qrcode_url;

    /** getAccessToken rsp */
    String _access_token;
    String _refresh_token;
    String _session_secret;
    String _session_key;
    int _expires_in;

    /** upload */
    String _md5;
    String _uploadid;
    String _path;
    size_t _size;

    /** pan.baidu.com */
    const char* rootCACertificate = \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIFCDCCA/CgAwIBAgIQBsk1GubwrG6wBvsMKqcyQTANBgkqhkiG9w0BAQsFADBh\n" \
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
    "QTAeFw0yMDAzMTMxMjAwMDBaFw0zMDAzMTMxMjAwMDBaMEwxCzAJBgNVBAYTAlVT\n" \
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxJjAkBgNVBAMTHURpZ2lDZXJ0IFNlY3Vy\n" \
    "ZSBTaXRlIENOIENBIEczMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA\n" \
    "8FvLH8zXOPwMrB5ZEdaUtfupXVj3ADIIt9uabxZnmnOJVeER4gBDBmuZ/5zvatiK\n" \
    "LqzE05lw03rvSXjiWjAwGSZQWbxz4qUIxGxOpvypsW5tbvcnKkPG9vs2tj+u+KSK\n" \
    "CCMA792c4rroXOBHjlQHl+ET+xnWc3nxobw7yL1vThEcBkCsLiu4BE5eETMzEplu\n" \
    "Z5hVT31EISTkU+L2qoVPqvl2vCLKmb4iKJYHpGIm1qVGRgf54kxfhRl9rEu4k2rQ\n" \
    "eUaJh4r5dKz1y0TFBwLIAM4nwGVc61H5S874Mt1Zw5i2kxnRymMNg5FFuCkQFIrj\n" \
    "UlFvlDohMoBNRvbtzHQAHQIDAQABo4IBzzCCAcswHQYDVR0OBBYEFETZyEozjtNS\n" \
    "jaeSlGEfmsilt+zLMB8GA1UdIwQYMBaAFAPeUDVW0Uy7ZvCj4hsbw5eyPdFVMA4G\n" \
    "A1UdDwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwEgYD\n" \
    "VR0TAQH/BAgwBgEB/wIBADAzBggrBgEFBQcBAQQnMCUwIwYIKwYBBQUHMAGGF2h0\n" \
    "dHA6Ly9vY3NwLmRpZ2ljZXJ0LmNuMEAGA1UdHwQ5MDcwNaAzoDGGL2h0dHA6Ly9j\n" \
    "cmwuZGlnaWNlcnQuY24vRGlnaUNlcnRHbG9iYWxSb290Q0EuY3JsMIHOBgNVHSAE\n" \
    "gcYwgcMwgcAGBFUdIAAwgbcwKAYIKwYBBQUHAgEWHGh0dHBzOi8vd3d3LmRpZ2lj\n" \
    "ZXJ0LmNvbS9DUFMwgYoGCCsGAQUFBwICMH4MfEFueSB1c2Ugb2YgdGhpcyBDZXJ0\n" \
    "aWZpY2F0ZSBjb25zdGl0dXRlcyBhY2NlcHRhbmNlIG9mIHRoZSBSZWx5aW5nIFBh\n" \
    "cnR5IEFncmVlbWVudCBsb2NhdGVkIGF0IGh0dHBzOi8vd3d3LmRpZ2ljZXJ0LmNv\n" \
    "bS9ycGEtdWEwDQYJKoZIhvcNAQELBQADggEBAIEbzergTPhY7dFwqeIB+fRlzYgI\n" \
    "fo40x1uqb+sH3imvCC5V57mSVVPXwEe2hDHCpo2+zGGqot60EkVtLF0+bUpCXffc\n" \
    "3mzVx6ti7GX+znUK5Vh41aoNy1AUara/iWQbcUXo32BrRPmXE+kMzt3VWxQE6ybP\n" \
    "r/l4SAcusEOmuwCTzzkTRttvWCa9VLgY5SsZ0jIj+/eRvQJRxv8x24/HrTI95OMo\n" \
    "+cTFs6iaigj+k85GmxtbpYEJJ4M8E6AtrBQ/i6/jhVY5RssEcxn0sUEvl8Lpxnlr\n" \
    "vyedD9l6digYDmB5jATBq8i8aaLiD7gyah3GHeziwuKmebv3CeTy44o+ORM=\n" \
    "-----END CERTIFICATE-----\n";

    /** d.pcs.baidu.com */
    const char* rootCACertificate1 = \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIFDDCCA/SgAwIBAgIQBR8Mft3IjbrwDFDihfQiZTANBgkqhkiG9w0BAQsFADBh\n" \
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
    "QTAeFw0yMDAzMTMxMjAwNDhaFw0zMDAzMTMxMjAwNDhaMFAxCzAJBgNVBAYTAlVT\n" \
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxKjAoBgNVBAMTIURpZ2lDZXJ0IFNlY3Vy\n" \
    "ZSBTaXRlIFBybyBDTiBDQSBHMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoC\n" \
    "ggEBALqD6tXbzIpRYNzoznLbHwz+chdbruBQJ80/GqdVld/xtIvi2yIE0aPZ/awW\n" \
    "D7yc353pX9eNysCtYMr+uLlqHxSU3DgMh+HdZG3wam+rGbO2I6l5tjxJKP5BSysG\n" \
    "oxnL2ZnONKTiDOu5RL5JrD35R64w6FWGFx+DsKW/e3WnR80NDH+JWFFzwi/bzHFh\n" \
    "HJ6gsaeAiWqMunCyxbejtHnPcmDs7Mca0d8tb7vqs8KbHwEjMMTfE8lLGYL2rHy3\n" \
    "q/uSfMUminrjr4TTgVoHhhjABA0y5f0FF8S4SNBDB9GbUrToL4nuX9AtuC65j+u/\n" \
    "cQ6I/T1Tdyks5seRKiFamCtMxo0CAwEAAaOCAc8wggHLMB0GA1UdDgQWBBR7o/r/\n" \
    "9dUJXR75Kv+FU+2vR6jXejAfBgNVHSMEGDAWgBQD3lA1VtFMu2bwo+IbG8OXsj3R\n" \
    "VTAOBgNVHQ8BAf8EBAMCAYYwHQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMC\n" \
    "MBIGA1UdEwEB/wQIMAYBAf8CAQAwMwYIKwYBBQUHAQEEJzAlMCMGCCsGAQUFBzAB\n" \
    "hhdodHRwOi8vb2NzcC5kaWdpY2VydC5jbjBABgNVHR8EOTA3MDWgM6Axhi9odHRw\n" \
    "Oi8vY3JsLmRpZ2ljZXJ0LmNuL0RpZ2lDZXJ0R2xvYmFsUm9vdENBLmNybDCBzgYD\n" \
    "VR0gBIHGMIHDMIHABgRVHSAAMIG3MCgGCCsGAQUFBwIBFhxodHRwczovL3d3dy5k\n" \
    "aWdpY2VydC5jb20vQ1BTMIGKBggrBgEFBQcCAjB+DHxBbnkgdXNlIG9mIHRoaXMg\n" \
    "Q2VydGlmaWNhdGUgY29uc3RpdHV0ZXMgYWNjZXB0YW5jZSBvZiB0aGUgUmVseWlu\n" \
    "ZyBQYXJ0eSBBZ3JlZW1lbnQgbG9jYXRlZCBhdCBodHRwczovL3d3dy5kaWdpY2Vy\n" \
    "dC5jb20vcnBhLXVhMA0GCSqGSIb3DQEBCwUAA4IBAQCFMP6Exs4uwBILlV3yCOg1\n" \
    "9T0GhyY1XFLHJkG/zgYmdoFZc4N0I7NIuMKEZkcaOc13Wt6QkS6GpMO3aZkXYTfl\n" \
    "9zpDtwdqSpL43nkCxd+nxB1A3N+9D/ZswoOUeBi0FI/fhKTh4B8sdRoVBYRJEhef\n" \
    "5J0LRssXrSRiYxmITdX2N2zCcIL6/17YXds2BTxvfCHDWGWnnAys4i0W5ccgQ5bt\n" \
    "nPHVW3hEZCO9nbrAOl4swdyFTV1Q0UUG/wZRYZEkFKdpfBOjCWDDrKo1SGV1Qys8\n" \
    "6y96+AgqbDB91mdnxIr1MuTtHGn/Q1YE2x9OSTb2f2k0ArhFxFdcjKTfLLewG1xD\n" \
    "-----END CERTIFICATE-----\n";
};

#endif
