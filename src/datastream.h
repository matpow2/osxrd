#ifndef ST_DATASTREAM_H
#define ST_DATASTREAM_H

#include <iostream>
#include <fstream>

class BaseStream
{
public:
    bool read(std::string & str, size_t len);
    bool read(std::string & str);
    char read_int8();
    unsigned char read_uint8();
    short read_int16();
    unsigned short read_uint16();
    int read_int32();
    unsigned int read_uint32();
    void read_string(std::string & str);
    float read_float();

    void write(const std::string & str);
    void write_int8(char v);
    void write_uint8(unsigned char v);
    void write_int16(short v);
    void write_uint16(unsigned short v);
    void write_int32(int v);
    void write_uint32(unsigned int v);
    void write_string(const std::string & str);
    void write_float(float v);

    bool at_end();
    unsigned char peek(size_t peekpos);

    // implementation-specific
    virtual bool read(char * data, size_t len);
    virtual void write(char * data, size_t len);
    virtual size_t get_size();
    virtual size_t tell();
    virtual void seek(size_t pos);
};

class DataStream : public BaseStream
{
public:
    char * data;
    size_t size;
    size_t pos;
    bool is_writer;

    DataStream(char * data, size_t size);
    DataStream();
    ~DataStream();

    using BaseStream::read;
    using BaseStream::write;
    bool read(char * data, size_t len);
    void write(char * data, size_t len);
    bool ensure_size(size_t size);
    size_t get_size();
    size_t tell();
    void seek(size_t pos);
};

#endif // ST_DATASTREAM_H
