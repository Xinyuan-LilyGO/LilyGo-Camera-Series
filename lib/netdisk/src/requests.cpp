#include "requests.h"

/** it is imperfect */
int Requests::request(const char * type, Stream * stream, size_t size)
{
    if(!stream) {
        return returnError(HTTPC_ERROR_NO_STREAM);
    }

    // connect to server
    if(!connect()) {
        return returnError(HTTPC_ERROR_CONNECTION_REFUSED);
    }

    if(size > 0) {
        addHeader("Content-Length", String((_boundary.length() + 2) + (64 + 2) + (24 + 2) + 2 + size + (2 + _boundary.length() + 4)));
    }

    // add cookies to header, if present
    String cookie_string;
    if(generateCookieString(&cookie_string)) {
        addHeader("Cookie", cookie_string);
    }

    // send Header
    if(!sendHeader(type)) {
        return returnError(HTTPC_ERROR_SEND_HEADER_FAILED);
    }

    int buff_size = HTTP_TCP_BUFFER_SIZE;

    int len = size;
    int bytesWritten = 0;

    if(len == 0) {
        len = -1;
    }

    // if possible create smaller buffer then HTTP_TCP_BUFFER_SIZE
    if((len > 0) && (len < HTTP_TCP_BUFFER_SIZE)) {
        buff_size = len;
    }

    // create buffer for read
    uint8_t * buff = (uint8_t *) malloc(buff_size);

    if(buff) {
        _client->println(_boundary); // 52 + 2
        _client->println("Content-Disposition: form-data;name=\"file\"; filename=\"epd47.jpg\""); // 42 + 2
        _client->println("Content-Type: image/jpeg"); // 24 + 2
        _client->println(); // 2

        // read all data from stream and send it to server
        while(connected() && (stream->available() > -1) && (len > 0 || len == -1)) {

            // get available data size
            int sizeAvailable = stream->available();

            if(sizeAvailable) {

                int readBytes = sizeAvailable;

                // read only the asked bytes
                if(len > 0 && readBytes > len) {
                    readBytes = len;
                }

                // not read more the buffer can handle
                if(readBytes > buff_size) {
                    readBytes = buff_size;
                }

                // read data
                int bytesRead = stream->readBytes(buff, readBytes);

                // write it to Stream
                int bytesWrite = _client->write((const uint8_t *) buff, bytesRead);
                bytesWritten += bytesWrite;

                // are all Bytes a writen to stream ?
                if(bytesWrite != bytesRead) {
                    log_w("short write, asked for %d but got %d retry...", bytesRead, bytesWrite);

                    // check for write error
                    if(_client->getWriteError()) {
                        log_w("stream write error %d", _client->getWriteError());

                        //reset write error for retry
                        _client->clearWriteError();
                    }

                    // some time for the stream
                    delay(1);

                    int leftBytes = (readBytes - bytesWrite);

                    // retry to send the missed bytes
                    bytesWrite = _client->write((const uint8_t *) (buff + bytesWrite), leftBytes);
                    bytesWritten += bytesWrite;

                    if(bytesWrite != leftBytes) {
                        // failed again
                        log_e("short write, asked for %d but got %d failed.", leftBytes, bytesWrite);
                        free(buff);
                        return returnError(HTTPC_ERROR_SEND_PAYLOAD_FAILED);
                    }
                }

                // check for write error
                if(_client->getWriteError()) {
                    log_e("stream write error %d", _client->getWriteError());
                    free(buff);
                    return returnError(HTTPC_ERROR_SEND_PAYLOAD_FAILED);
                }

                // count bytes to read left
                if(len > 0) {
                    len -= readBytes;
                }

                delay(0);
            } else {
                delay(1);
            }
        }

        free(buff);

        if(size && (int) size != bytesWritten) {
            log_e("Stream payload bytesWritten %d and size %d mismatch!.", bytesWritten, size);
            log_e("ERROR SEND PAYLOAD FAILED!");
            return returnError(HTTPC_ERROR_SEND_PAYLOAD_FAILED);
        } else {
            log_i("Stream payload written: %d", bytesWritten);
        }

        _client->print(String("\r\n") + _boundary + String("--\r\n")); // 58
    } else {
        log_e("too less ram! need %d", HTTP_TCP_BUFFER_SIZE);
        return returnError(HTTPC_ERROR_TOO_LESS_RAM);
    }

    // handle Server Response (Header)
    return returnError(handleHeaderResponse());
}


int Requests::request(const char * type, httpRequestData *data, int datalen, httpRequestFiles *files, int fileslen)
{
    size_t content_length = 0;
    size_t i = 0;
    int ret = 0;
    if(!data && !files) {
        return returnError(HTTPC_ERROR_NO_STREAM);
    }

    // connect to server
    if(!connect()) {
        return returnError(HTTPC_ERROR_CONNECTION_REFUSED);
    }

    generateBoundary(24);
    addHeader("Content-Type", String("multipart/form-data; boundary=" + _boundary.substring(2, _boundary.length())));

    for (i = 0; i < datalen; i++) {
        content_length += _boundary.length() + 2;
        content_length += _data_disposition.length() + data[i].key.length() + _quotes.length() + 2;
        content_length += 2;
        content_length += data[i].value.length() + 2;
    }

    for (i = 0; i < fileslen; i++) {
        content_length += _boundary.length() + 2;
        content_length += _data_disposition.length() + files[i].key.length() + _quotes.length() + _filename.length() + files[i].filename.length() + _quotes.length() + 2;
        // content_length += _content_type.length() + files[i].type.length() + 2;
        content_length += 2; // 2
        content_length += files[i].size;
    }
    content_length += 2 + _boundary.length() + 2;
    addHeader("Content-Length", String(content_length));

    // add cookies to header, if present
    String cookie_string;
    if(generateCookieString(&cookie_string)) {
        addHeader("Cookie", cookie_string);
    }

    // send Header
    if(!sendHeader(type)) {
        return returnError(HTTPC_ERROR_SEND_HEADER_FAILED);
    }

    // create buffer for read
    uint8_t * buff = (uint8_t *) malloc(HTTP_TCP_BUFFER_SIZE);

    if(buff) {
        for (i = 0; i < datalen; i++) {
            _client->println(_boundary); // _boundary.length() + 2
            _client->println(_data_disposition + data[i].key + _quotes); // _data_disposition.length() + data[i].key.length() + _quotes.length() + 2
            _client->println(); // 2
            _client->println(data[i].value); // data[i].value.length() + 2
        }

        for (i = 0; i < fileslen; i++) {
            _client->println(_boundary); // _boundary.length() + 2
            _client->println(_data_disposition + files[i].key + _quotes + _filename + files[i].filename + _quotes); //_data_disposition.length()
            // _client->println(_content_type + files[i].type); // _content_type.length() + files[i].type.length() + 2
            _client->println(); // 2

            if (files[i].stream != nullptr) {
                ret = sendStreamBody(files[i].stream, files[i].size, buff);
                if (ret != 0) {
                    return returnError(ret);
                }
            }

            if (files[i].buffer != nullptr)
            {
                ret = sendBufferBody(files[i].buffer, files[i].size);
                if (ret != 0) {
                    return returnError(ret);
                }
            }

        }
        free(buff);
        _client->print(String("\r\n") + _boundary + String("--\r\n"));
    } else {
        log_e("too less ram! need %d", HTTP_TCP_BUFFER_SIZE);
        return returnError(HTTPC_ERROR_TOO_LESS_RAM);
    }

    // handle Server Response (Header)
    return returnError(handleHeaderResponse());
}


int Requests::sendStreamBody(Stream *stream, size_t size, uint8_t * buff)
{
    int len = 0;
    int buff_size = 0;
    int bytesWritten = 0;
    buff_size = HTTP_TCP_BUFFER_SIZE;

    len = size;
    bytesWritten = 0;

    if(len == 0) {
        len = -1;
    }

    // if possible create smaller buffer then HTTP_TCP_BUFFER_SIZE
    if((len > 0) && (len < HTTP_TCP_BUFFER_SIZE)) {
        buff_size = len;
    }

    // read all data from stream and send it to server
    while(connected() && (stream->available() > -1) && (len > 0 || len == -1)) {

        // get available data size
        int sizeAvailable = stream->available();

        if(sizeAvailable) {

            int readBytes = sizeAvailable;

            // read only the asked bytes
            if(len > 0 && readBytes > len) {
                readBytes = len;
            }

            // not read more the buffer can handle
            if(readBytes > buff_size) {
                readBytes = buff_size;
            }

            // read data
            int bytesRead = stream->readBytes(buff, readBytes);

            // write it to Stream
            int bytesWrite = _client->write((const uint8_t *) buff, bytesRead);
            bytesWritten += bytesWrite;

            // are all Bytes a writen to stream ?
            if(bytesWrite != bytesRead) {
                log_d("short write, asked for %d but got %d retry...", bytesRead, bytesWrite);

                // check for write error
                if(_client->getWriteError()) {
                    log_w("stream write error %d", _client->getWriteError());

                    //reset write error for retry
                    _client->clearWriteError();
                }

                // some time for the stream
                delay(1);

                int leftBytes = (readBytes - bytesWrite);

                // retry to send the missed bytes
                bytesWrite = _client->write((const uint8_t *) (buff + bytesWrite), leftBytes);
                bytesWritten += bytesWrite;

                if(bytesWrite != leftBytes) {
                    // failed again
                    log_e("short write, asked for %d but got %d failed.", leftBytes, bytesWrite);
                    free(buff);
                    return HTTPC_ERROR_SEND_PAYLOAD_FAILED;
                }
            }

            // check for write error
            if(_client->getWriteError()) {
                log_e("stream write error %d", _client->getWriteError());
                free(buff);
                return HTTPC_ERROR_SEND_PAYLOAD_FAILED;
            }

            // count bytes to read left
            if(len > 0) {
                len -= readBytes;
            }

            delay(0);
        } else {
            delay(1);
        }
    }

    if(size && (int) size != bytesWritten) {
        log_e("Stream payload bytesWritten %d and size %d mismatch!.", bytesWritten, size);
        log_e("ERROR SEND PAYLOAD FAILED!");
        return HTTPC_ERROR_SEND_PAYLOAD_FAILED;
    } else {
        log_i("Stream payload written: %d", bytesWritten);
    }
    return 0;
}


int Requests::sendBufferBody(uint8_t * buff, size_t size)
{
    int buff_size = HTTP_TCP_BUFFER_SIZE;
    int bytesWritten = 0;
    int len = size;

    // if possible create smaller buffer then HTTP_TCP_BUFFER_SIZE
    if((size > 0) && (size < HTTP_TCP_BUFFER_SIZE)) {
        buff_size = size;
    }

    // read all data from stream and send it to server
    while(connected() && len > 0) {

        bytesWritten += _client->write(&buff[bytesWritten], buff_size);
        delay(1);
        len = size - bytesWritten;

        if (buff_size > len)
        {
            buff_size = len;
        }
    }

    if(size && (int) size != bytesWritten) {
        log_e("Stream payload bytesWritten %d and size %d mismatch!.", bytesWritten, size);
        log_e("ERROR SEND PAYLOAD FAILED!");
        return HTTPC_ERROR_SEND_PAYLOAD_FAILED;
    } else {
        log_i("Stream payload written: %d", bytesWritten);
    }

    return 0;
}


void Requests::generateBoundary(const int len)
{
    String str = "--";
    char c;

    for (size_t i = 0; i < len; i++)
    {
        c = '0' + rand()%10;
        str += c;
    }

    _boundary = "--" + str;
}
