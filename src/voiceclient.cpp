#include "../include/voiceclient.h"

enum {
    REQ_ACK = 0,
    REQ_ADATA = 1,
    REQ_IDENT = 2,
    REQ_END = 3
};

#define FORMAT paInt16

void setup_opus(OpusEncoder **enc, OpusDecoder **dec) {
    int err;
    if (enc != NULL) {
        *enc = opus_encoder_create(SAMPLE_RATE, 1, APPLICATION, &err);
        if (err < 0) {
            fprintf(stderr, "failed to create encoder: %s\n", opus_strerror(err));
            exit(1);
        }

        err = opus_encoder_ctl(*enc, OPUS_SET_BITRATE(BITRATE));
        if (err < 0) {
            fprintf(stderr, "failed to set bitrate: %s\n", opus_strerror(err));
            exit(1);
        }
    }

    if (dec != NULL) {
        *dec = opus_decoder_create(SAMPLE_RATE, 1, &err);
        if (err < 0) {
            fprintf(stderr, "failed to create decoder: %s\n", opus_strerror(err));
            exit(1);
        }
    }
}

static int pa_read_callback(const void *input, void *output,
                       unsigned long framesPerBuffer,
                       const PaStreamCallbackTimeInfo *timeInfo,
                       PaStreamCallbackFlags statusFlags,
                       void *userData) {
    VoiceClient *client = (VoiceClient*)userData;
    int16_t *in = (int16_t*)input;
    (void)timeInfo;
    (void)statusFlags;
    std::unique_lock<std::mutex> lock(client->inm);
    for (unsigned int i = 0; i < framesPerBuffer; i++) {
        client->inbuf.push_back(*in++);
        if (client->inbuf.size() > FRAME_SIZE) {
            client->condvar.notify_all();
        }
    }
    lock.unlock();

    return 0;
}

static int pa_write_callback(const void *input, void *output,
                       unsigned long framesPerBuffer,
                       const PaStreamCallbackTimeInfo *timeInfo,
                       PaStreamCallbackFlags statusFlags,
                       void *userData) {
    VoiceClient *client = (VoiceClient*)userData;
    int16_t *out = (int16_t*)output;
    (void)timeInfo;
    (void)statusFlags;
    std::unique_lock<std::mutex> lock(client->inm);
    for (unsigned int i = 0; i < framesPerBuffer; i++) {

        int16_t val = 0;
        //TODO mutex on voicepeers?
        for (auto &peer: client->voicepeers) {
            //if (client->outbuf.size() <= FRAME_SIZE && !done) {
            //    printf("Buffer underflow!\n");
            //    done = true;
            //}
            std::lock_guard<std::mutex> lock(peer.second.outm);
            if (peer.second.outbuf.size() > FRAME_SIZE) {
                val = peer.second.outbuf.back();
                peer.second.outbuf.pop_back();
            }
            if (peer.second.outbuf.size() > FRAME_SIZE * 8) {
                printf("Buffer overflow!\n");
                //while (peer.second.outbuf.size() > FRAME_SIZE) {
                    peer.second.outbuf.pop_back();
                //}
            }
        }
        *out++ = val;
    }
    lock.unlock();

    return 0;
}

#define ERRHANDLE(err) if (err != paNoError) return err;

PaError VoiceClient::audio_run(PaDeviceIndex in, PaDeviceIndex out) {
    PaError err;
    const PaDeviceInfo *ininfo = Pa_GetDeviceInfo(in);
    const PaDeviceInfo *outinfo = Pa_GetDeviceInfo(out);
    const PaDeviceInfo *d = ininfo;
    printf("%s %d %d %lf %lf %lf %lf %lf\n", d->name, d->maxInputChannels, d->maxOutputChannels, d->defaultLowInputLatency, d->defaultLowOutputLatency, d->defaultHighInputLatency, d->defaultHighOutputLatency, d->defaultSampleRate);
    d = outinfo;
    printf("%s %d %d %lf %lf %lf %lf %lf\n", d->name, d->maxInputChannels, d->maxOutputChannels, d->defaultLowInputLatency, d->defaultLowOutputLatency, d->defaultHighInputLatency, d->defaultHighOutputLatency, d->defaultSampleRate);
    printf("%lf %lf\n", ininfo->defaultSampleRate, outinfo->defaultSampleRate);
    PaStreamParameters inparams = {in, 1, FORMAT, ininfo->defaultLowInputLatency, NULL};
    PaStreamParameters outparams = {out, 1, FORMAT, outinfo->defaultLowOutputLatency, NULL};
    
    err = Pa_OpenStream(&instream,  &inparams,  NULL, SAMPLE_RATE, paFramesPerBufferUnspecified, paNoFlag, pa_read_callback, this); ERRHANDLE(err);
    printf("Made it this far\n");
    err = Pa_OpenStream(&outstream, NULL, &outparams, SAMPLE_RATE, paFramesPerBufferUnspecified, paNoFlag, pa_write_callback, this); ERRHANDLE(err);
    printf("even further wowee\n");
    //err = Pa_OpenDefaultStream(&instream,  1, 0, FORMAT, SAMPLE_RATE, paFramesPerBufferUnspecified, pa_read_callback,  this); ERRHANDLE(err);
    //err = Pa_OpenDefaultStream(&outstream, 0, 1, FORMAT, SAMPLE_RATE, paFramesPerBufferUnspecified, pa_write_callback, this); ERRHANDLE(err);
    err = Pa_StartStream(instream);  ERRHANDLE(err);
    err = Pa_StartStream(outstream); ERRHANDLE(err);
    while (!stopped) {
        Pa_Sleep(500);
    }
    err = Pa_StopStream(instream);   ERRHANDLE(err);
    err = Pa_StopStream(outstream);  ERRHANDLE(err);
    err = Pa_CloseStream(instream);  ERRHANDLE(err);
    err = Pa_CloseStream(outstream); ERRHANDLE(err);
    return paNoError;
}

#include <iostream>

void VoiceClient::start_recv() {
    printf("Start recv\n");
    std::cout << endp << "\n";
    /*
    sock.async_receive_from(
        asio::buffer(netbuf, MAX_FRAME_SIZE),
        endp,
        [this](const asio::error_code &ec, size_t bytes) {
            this->handle_recv(ec, bytes);
        }
    );*/
    while(!stopped || send_end) {
        asio::error_code ec;
        size_t len = sock.receive_from(asio::buffer(netbuf, MAX_FRAME_SIZE), endp);
        handle_recv(ec, len);
    }
}

VoicePeer::VoicePeer() {
    setup_opus(NULL, &this->dec);
}

void VoiceClient::handle_recv(const asio::error_code &ec, size_t nBytes) {
    if (ec) {
        fprintf(stderr, "ERROR (Recv): %s\n", ec.message().c_str());
    } else {

        uint8_t req = netbuf[0];
        printf("handle_recv %d\n", req);
        if (req == REQ_ACK) {
            printf("Got ack\n");
            if (send_ident) {
                send_ident = false;
            } else if (send_end) {
                send_end = false;
            } else {
                printf("Spurious ack\n");
            }
        } else if (req == REQ_ADATA) {
            uint64_t uuid;
            memcpy(&uuid, netbuf + 1, 8);
//            printf("Recvd %d bytes from uuid %lu ", nBytes, uuid);

            if (voicepeers.count(uuid) == 0) {
                voicepeers[uuid];
            }

            VoicePeer &peer = voicepeers[uuid];
            opus_int16 out[MAX_FRAME_SIZE];
            int frame_size = opus_decode(peer.dec, netbuf + 9, nBytes - 9, out, MAX_FRAME_SIZE, 0);
            if (frame_size < 0) {
                fprintf(stderr, "decoder failed: %s\n", opus_strerror(frame_size));
                exit(1);
            }

            std::lock_guard<std::mutex> lock(peer.outm);
            for (int i = 0; i < frame_size; i++) {
                peer.outbuf.push_front(out[i]);
            }
        } else {
            printf("Got spurious request of type %d\n", req);
        }

        if (!stopped || send_end) {
            //start_recv();
        }
    }
}

VoiceClient::VoiceClient(asio::io_context &ctx) : sock(ctx) {
    udp::resolver resolver(ctx);
    asio::error_code ec;
    //udp::resolver::query query(udp::v4(), "127.0.0.1", "2346"); 
    auto endpoints = resolver.resolve("cospox.com", "2346", ec); // *resolver.resolve(query).begin();
    endp = *endpoints.begin();
    if (ec) {
        printf("Error: Couldn't connec to voice server: %s\n", ec.message().c_str());
    }
    sock.open(udp::v4());
    sock.set_option(asio::ip::udp::socket::reuse_address(true));
}


void VoiceClient::run(uint64_t uuid) {
    opus_int16 in[FRAME_SIZE];

    unsigned char cbits[MAX_PACKET_SIZE];
    int nBytes;
    //TODO bad idea
    int end_iters = 0;
    //setup_opus(&enc, &dec);

    while ((!stopped || send_end) && end_iters <= 20) {
        //sanity check: Sometimes, it can be requested to stop before/during ident period, just discard the ident req
        if (send_end && send_ident) send_ident = 0;
        if (send_ident) {
            uint8_t data[9];
            data[0] = REQ_IDENT;
            memcpy(data + 1, &uuid, 8);
            sock.send_to(asio::buffer(data, 9), endp);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        } else if (send_end) {
            uint8_t data[1] = {REQ_END};
            printf("Sending end\n");
            end_iters++;
            sock.send_to(asio::buffer(data, 1), endp);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        } else {
            std::unique_lock<std::mutex> lck(inm);
            while (inbuf.size() < FRAME_SIZE && !stopped) condvar.wait(lck);
            if (!stopped) {
                for (int i = 0; i < FRAME_SIZE; i++) {
                    in[i] = inbuf.front();
                    inbuf.pop_front();
                }
                lck.unlock();
                nBytes = opus_encode(enc, in, FRAME_SIZE, cbits, MAX_PACKET_SIZE);
                if (nBytes < 0) {
                    fprintf(stderr, "encode failed: %s\n", opus_strerror(nBytes));
                    exit(1);
                }

                uint8_t *sendbuf = (uint8_t*)malloc(nBytes + 1);
                sendbuf[0] = REQ_ADATA;
                memcpy(sendbuf + 1, cbits, nBytes);

                sock.send_to(asio::buffer(sendbuf, nBytes + 1), endp);
                free(sendbuf);
            }
        }
    }
    opus_encoder_destroy(enc);
    for (auto &it : voicepeers) {
        opus_decoder_destroy(it.second.dec);
    }
}

void VoiceClient::stop() {
    stopped = true;
    send_end = true;
    condvar.notify_all();
}
