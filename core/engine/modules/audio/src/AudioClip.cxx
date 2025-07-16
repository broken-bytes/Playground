#include "audio/AudioClip.hxx"

#include <stdint.h>
#include <iostream>

namespace playground::audio {
    AudioClip::AudioClip() {

    }

    AudioClip::AudioClip(
        std::vector<float> buffer,
        int sampleRate, 
        int bitDepth, 
        int numSamples, 
        int numChannels, 
        float length, 
        bool stereo
    ) : _buffer(buffer), 
        _sampleRate(sampleRate), 
        _bitDepth(bitDepth),
        _numSamples(numSamples), 
        _numChannels(numChannels),
        _length(length),
        _stereo(stereo)
    {

    }
    
    AudioClip::~AudioClip() {
    
    }

    auto AudioClip::Buffer() const -> std::vector<float> {
        return _buffer;
    }

    auto AudioClip::BufferId() const -> uint32_t {
        return _bufferId;
    }


    auto AudioClip::SampleRate() const -> uint32_t {
        return _sampleRate;
    }

    auto AudioClip::BitDepth() const -> uint32_t {
        return _bitDepth;
    }
    
    auto AudioClip::Channels() const -> uint8_t {
        return _numChannels;
    }
}
