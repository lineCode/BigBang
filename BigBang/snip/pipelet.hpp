#pragma once

#include <cmath>
#include "snip/snip.hpp"

namespace WarGrey::SCADA {
    private class Screwlet : public WarGrey::SCADA::Snip {
    public:
        Screwlet(float width, float height, float thickness = 0.0F,
            double color = 120.0, double saturation = 0.607,
            double bodylight = 0.339, double highlight = 0.839);

    public:
        void load() override;
        void draw(Microsoft::Graphics::Canvas::CanvasDrawingSession^ ds, float x, float y, float Width, float Height) override;
        void fill_extent(float x, float y, float* w = nullptr, float* h = nullptr,
            float* d = nullptr, float* s = nullptr, float* l = nullptr, float* r = nullptr)
            override;

    private:
        float width;
        float height;
        float pipe_thickness;
        float pipe_ascent;
        float connector_width;
        float connector_rx;

    private:
        Windows::UI::Color color;
        Windows::UI::Color connector_color;
        Windows::UI::Color highlight_color;
        Microsoft::Graphics::Canvas::Geometry::CanvasCachedGeometry^ connector;
        Microsoft::Graphics::Canvas::Brushes::CanvasLinearGradientBrush^ connector_brush;
        Microsoft::Graphics::Canvas::Brushes::CanvasLinearGradientBrush^ pipe_brush;
    };

    private class Pipelet : public WarGrey::SCADA::Snip {
    public:
        Pipelet(float width, float height = 0.0F, float thickness = 0.0F,
            double color = nan("DimGray"), double saturation = 0.0,
            double bodylight = 0.412, double highlight = 0.753);

    public:
        void load() override;
        void update(long long count, long long interval, long long uptime, bool is_slow) override;
        void draw(Microsoft::Graphics::Canvas::CanvasDrawingSession^ ds, float x, float y, float Width, float Height) override;
        void fill_extent(float x, float y, float* w = nullptr, float* h = nullptr,
            float* d = nullptr, float* s = nullptr, float* l = nullptr, float* r = nullptr)
            override;

    private:
        float width;
        float height;
        float thickness;
        float ascent;
        float connector_width;
        float connector_rx;

    private:
        Windows::UI::Color color;
        Windows::UI::Color connector_color;
        Windows::UI::Color highlight_color;
        Microsoft::Graphics::Canvas::Geometry::CanvasCachedGeometry^ connector;
        Microsoft::Graphics::Canvas::Geometry::CanvasCachedGeometry^ hollow_body;
        Microsoft::Graphics::Canvas::Geometry::CanvasGeometry^ body_mask;
        Microsoft::Graphics::Canvas::Geometry::CanvasStrokeStyle^ cartoon_style;
        Microsoft::Graphics::Canvas::Brushes::CanvasLinearGradientBrush^ connector_brush;
        Microsoft::Graphics::Canvas::Brushes::CanvasLinearGradientBrush^ brush;
    };
}