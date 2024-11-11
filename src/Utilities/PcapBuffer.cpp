#include "PcapBuffer.h"

#include <string.h>

#define MAX_SIZE 15
#define MAX_PACKET_LEN 2324

static void write(fs::File file, const uint8_t* buf, uint32_t len){
  file.write(buf, len);
}

static void write(fs::File file, uint16_t n){
  uint8_t buf[2];
  buf[0] = n;
  buf[1] = n >> 8;
  write(file, buf, 2);
}

static void write(fs::File file, int32_t n){
  uint8_t buf[4];
  buf[0] = n;
  buf[1] = n >> 8;
  buf[2] = n >> 16;
  buf[3] = n >> 24;
  write(file, buf,4);
}

static void write(fs::File file, uint32_t n){
  uint8_t buf[4];
  buf[0] = n;
  buf[1] = n >> 8;
  buf[2] = n >> 16;
  buf[3] = n >> 24;
  write(file, buf,4);
}

PcapBuffer::PcapBuffer(std::string baseName, fs::FS* fs, uint32_t dataLinkType)
    : m_blocks()
{
    uint32_t counter = 0;
    while (true)
    {
        auto filename = "/" + baseName + "_" + std::to_string(counter) + ".pcap";
        if (!fs->exists(filename.c_str()))
        {
            m_file = fs->open(filename.c_str(), "w");
            if (m_file)
            {
                break;
            }
            else
            {
                counter++;
            }
        }
        else
        {
            counter++;
        }
    }

    write(m_file, uint32_t(0xa1b2c3d4)); // magic number
    write(m_file, uint16_t(2)); // major version number
    write(m_file, uint16_t(4)); // minor version number
    write(m_file, int32_t(0)); // GMT to local correction
    write(m_file, uint32_t(0)); // accuracy of timestamps
    write(m_file, uint32_t(MAX_PACKET_LEN)); // max length of captured packets, in octets
    write(m_file, uint32_t(dataLinkType)); // data link type
}

PcapBuffer::~PcapBuffer()
{
    while (!m_blocks.empty())
    {
        const auto& block = m_blocks.front();
        m_blocks.pop();    
        free(block.buffer);
    }

    m_file.close();
}

void PcapBuffer::flushToDisk()
{
    while (!m_blocks.empty())
    {
        const auto& block = m_blocks.front();
        m_blocks.pop();

        uint32_t microSeconds = block.time;
        uint32_t seconds = (microSeconds/1000)/1000; // e.g. 45200400/1000/1000 = 45200 / 1000 = 45s
        microSeconds -= seconds*1000*1000; // e.g. 45200400 - 45*1000*1000 = 45200400 - 45000000 = 400us (because we only need the offset)

        write(m_file, seconds); // ts_sec
        write(m_file, microSeconds); // ts_usec
        write(m_file, block.length); // incl_len
        write(m_file, block.length); // orig_len
        write(m_file, block.buffer, block.length); // packet payload
        
        free(block.buffer);
    }
}

bool PcapBuffer::append(uint8_t *packet, uint32_t len)
{
    if (m_blocks.size() > MAX_SIZE)
    {
        return false;
    }

    unsigned long curTime = micros();

    uint8_t* buffer = (uint8_t*) malloc(len);

    if (!buffer)
    {
        return false;
    }

    memcpy(buffer, packet, len);

    PcapBufferBlock block {
        .time = curTime,
        .buffer = buffer,
        .length = len
    };

    m_blocks.push(block);
    return true;
}