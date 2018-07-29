#pragma once

#include "LibAVOutputMedia.hpp"
#include "LibAVInputMedia.hpp"

class SoftwareDecoder : public LibAVOutputMedia {
public:
    virtual ~SoftwareDecoder();
    void run() override;
    AVFrame* decode() override;
};
