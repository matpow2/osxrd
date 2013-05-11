#include "datastream.h"

#include <stdlib.h>
#include <string.h>
    
// BaseStream

// implementation-specific

bool BaseStream::read(char * data, size_t len)
{
    return false;
}

void BaseStream::seek(size_t pos)
{
}

void BaseStream::write(char * data, size_t len)
{
}

size_t BaseStream::tell()
{
    return 0;
}

size_t BaseStream::get_size()
{
    return 0;
}

// util

bool BaseStream::at_end()
{
    return tell() >= get_size();
}

unsigned char BaseStream::peek(size_t peekpos)
{
    size_t pos = tell();
    seek(peekpos);
    unsigned char v = read_uint8();
    seek(pos);
    return v;
}

// readers

bool BaseStream::read(std::string & str, size_t len)
{
    str.resize(len, 0);
    return read(&str[0], len);
}

bool BaseStream::read(std::string & str)
{
    return read(str, get_size() - tell());
}

char BaseStream::read_int8()
{
    unsigned char v;
    if (!read((char*)&v, 1))
        return 0;
    return v;
}

unsigned char BaseStream::read_uint8()
{
    return (unsigned char)read_int8();
}

short BaseStream::read_int16()
{
    short v;
    if (!read((char*)&v, 2))
        return 0;
    return v;
}

unsigned short BaseStream::read_uint16()
{
    return (unsigned short)read_int16();
}

int BaseStream::read_int32()
{
    int v;
    if (!read((char*)&v, 4))
        return 0;
    return v;
}

unsigned int BaseStream::read_uint32()
{
    return (unsigned int)read_int32();
}

void BaseStream::read_string(std::string & str)
{
    size_t total_size = get_size();
    size_t size = 0;
    size_t pos = tell();
    bool has_null = false;
    while (true) {
        if (read_uint8() == 0) {
            has_null = true;
            break;
        }
        size++;
        if (at_end())
            break;
    }
    seek(pos);
    read(str, size);
    if (has_null)
        seek(pos + size + 1);
}

float BaseStream::read_float()
{
    float v;
    if (!read((char*)&v, 4))
        return 0.0f;
    return v;
}

// writers

void BaseStream::write(const std::string & str)
{
    return write((char*)&str[0], str.size());
}

void BaseStream::write_int8(char v)
{
    write(&v, 1);
}

void BaseStream::write_uint8(unsigned char v)
{
    write_int8((char)v);
}

void BaseStream::write_int16(short v)
{
    write((char*)&v, 2);
}

void BaseStream::write_uint16(unsigned short v)
{
    write_int16((short)v);
}

void BaseStream::write_int32(int v)
{
    write((char*)&v, 4);
}

void BaseStream::write_uint32(unsigned int v)
{
    write_int32((int)v);
}

void BaseStream::write_string(const std::string & str)
{
    write(str);
    write_int8(0);
}

void BaseStream::write_float(float v)
{
    write((char*)&v, 4);
}

// DataStream

DataStream::DataStream(char * data, size_t size)
: data(data), size(size), pos(0), is_writer(false)
{

}

DataStream::DataStream()
: size(0), pos(0), is_writer(true), data(NULL)
{
}

DataStream::~DataStream()
{
    if (is_writer)
        free(data);
}

void DataStream::write(char * data, size_t len)
{
    if (!ensure_size(len))
        return;
    memcpy(this->data + pos, data, len);
    pos += len;
}

bool DataStream::read(char * data, size_t len)
{
    if (!ensure_size(len))
        return false;
    memcpy(data, this->data + pos, len);
    pos += len;
    return true;
}

bool DataStream::ensure_size(size_t len)
{
    size_t new_size = pos + len;
    if (new_size <= size)
        return true;
    if (is_writer) {
        data = (char*)realloc(data, new_size);
        size = new_size;
        return true;
    } else
        return false;
}

size_t DataStream::get_size()
{
    return size;
}

size_t DataStream::tell()
{
    return pos;
}

void DataStream::seek(size_t pos)
{
    // This always evaulates to false.
    //if (pos < 0)
    //    pos = 0;
    if (pos >= size)
        pos = size - 1;
    this->pos = pos;
}