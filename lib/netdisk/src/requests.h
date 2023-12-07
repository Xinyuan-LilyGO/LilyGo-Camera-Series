
#ifndef REQUESTS_H_
#define REQUESTS_H_

#include <HTTPClient.h>

struct httpRequestData
{
    String key;
    String value;
};

struct httpRequestFiles
{
    String key;
    String filename;
    Stream *stream;
    uint8_t *buffer;
    size_t size;
    String type;
};

class Requests: public HTTPClient
{
public:
    int request(const char * type, Stream * stream, size_t size = 0);

    int request(const char * type, httpRequestData *data, int datalen, httpRequestFiles *files, int fileslen);

protected:
    void generateBoundary(const int len);
    int sendStreamBody(Stream *stream, size_t size, uint8_t * buff);
    int sendBufferBody(uint8_t * buff, size_t size);
    String _boundary;
    String _data_disposition = "Content-Disposition: form-data;name=\"";
    String _quotes = "\"";
    String _filename = "; filename=\"";
    String _content_type = "Content-Type: ";
    // String _semicolon = ";";
};

#endif
