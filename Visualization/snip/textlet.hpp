#pragma once

#include "snip.hpp"
#include "canvas.hxx"

namespace WarGrey::SCADA {
    private class Textlet : public WarGrey::SCADA::Snip {
    public:
        Textlet(const wchar_t* fmt, ...);
        Textlet(Platform::String^ content = "");

        ~Textlet() noexcept;

    public:
        void change_text(Platform::String^ content);

    public:
        void draw(Microsoft::Graphics::Canvas::CanvasDrawingSession^ ds, float x, float y, float Width, float Height) override;
        void fill_extent(float* w = nullptr, float* h = nullptr,
            float* b = nullptr, float* t = nullptr, float* l = nullptr, float* r = nullptr)
            override;

    private:
        Platform::String^ content;
        Microsoft::Graphics::Canvas::Text::CanvasTextFormat^ layout_config;
    };
}
