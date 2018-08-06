#pragma once

#include "LibAVOutputMedia.hpp"
#include "LibAVInputMedia.hpp"

class HardwareDecoder : public LibAVOutputMedia {
public:
    virtual ~HardwareDecoder();
    void run() override;
    AVFrame* decode() override;
};
