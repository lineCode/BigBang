#pragma once

#include "snip.hpp"

namespace WarGrey::SCADA {
    private class Alarmlet : public WarGrey::SCADA::Snip {
    public:
        Alarmlet(float size);
        ~Alarmlet() noexcept;

    public:
        void draw(Microsoft::Graphics::Canvas::CanvasDrawingSession^ ds, float x, float y, float Width, float Height) override;
        void fill_extent(float* w = nullptr, float* h = nullptr,
            float* b = nullptr, float* t = nullptr, float* l = nullptr, float* r = nullptr)
            override;

    private:
        float size;
    };
}
