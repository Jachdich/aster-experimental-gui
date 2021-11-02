#ifndef __VOICE_CLIENT_H
#define __VOICE_CLIENT_H

//BAD IDEA but lazy lol
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <opus/opus.h>
#include <stdio.h>
#include <unordered_map>
#include <math.h>
#include <asio.hpp>
#include <condition_variable>
#include <mutex>
#include <deque>
#include <thread>
#include <chrono>
#include <portaudio.h>

using udp = asio::ip::udp;
#define FRAME_SIZE 960
#define SAMPLE_RATE 48000
#define APPLICATION OPUS_APPLICATION_AUDIO
#define BITRATE 64000

#define MAX_FRAME_SIZE 6*960
#define MAX_PACKET_SIZE (3*1276)

struct SoundIo;
void setup_opus(OpusEncoder **enc, OpusDecoder **dec);

struct VoicePeer {
    OpusDecoder *dec;
    std::mutex outm;
    std::deque<int16_t> outbuf;
    VoicePeer();
};

class VoiceClient {
public:
	int16_t i = 0;
    udp::socket sock;
    udp::endpoint endp;
    OpusEncoder *enc;

    std::deque<int16_t> inbuf;
    std::deque<int16_t> outbuf;
    std::mutex inm;
    std::mutex outm;
    std::condition_variable condvar;
    std::unordered_map<uint64_t, VoicePeer> voicepeers;
    PaStream *instream;
    PaStream *outstream;
    unsigned char netbuf[MAX_PACKET_SIZE + 1];
    bool stopped = false;
    bool send_ident = true;
    bool send_end = false;
    
    VoiceClient(asio::io_context &ctx);
    void start_recv();
    void handle_recv(const asio::error_code &ec, size_t nBytes);
    PaError audio_run(PaDeviceIndex in, PaDeviceIndex out);
    void stop();
    void run(uint64_t uuid);
};

#endif
