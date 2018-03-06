﻿#include <cmath>

#include "text.hpp"
#include "paint.hpp"
#include "tongue.hpp"
#include "shape.hpp"
#include "snip/gaugelet.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::UI;
using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;

static const float scale_lmark_ratio = 1.0F;
static const float scale_space_ratio = 1.0F;
static const float mbody_width_ratio = 5.0F;

Gaugelet::Gaugelet(Platform::String^ caption, int range, unsigned char step, Color& color)
	: range(range), step((step > 0) ? step : 10) {
	this->label_font = make_text_format(12.0F);
	this->caption = make_text_layout(caption, this->label_font);
	this->color = make_solid_brush(color);
	this->set_scale(0.0F, true);
}

void Gaugelet::initialize_meter() {
    auto font = make_text_format(this->label_font->FontSize * 2.0F / 3.0F);
    TextExtent zero = get_text_extent("0", font);

    this->ch = zero.width;
    this->mark_interval = zero.height * 0.8F;

    { // make scales and marks
		float linespacing = this->mark_interval * 2.0F;
		float long_mark_length = this->ch * scale_lmark_ratio;
		float short_mark_length = long_mark_length * 0.618F;
		float short_mark_x = long_mark_length - short_mark_length;
		int delta = this->range / this->step;

        auto marks = blank();
        Platform::String^ scale_sequences = "";
        for (char i = 0; i <= step; i++) {
            auto ythis = mark_interval * i;

            if (i % 2 == 0) {
                marks = geometry_union(marks, hline(0.0F, ythis, ch));
                scale_sequences = scale_sequences + " " + (this->range - delta * i).ToString();
            } else {
                marks = geometry_union(marks, hline(short_mark_x, ythis, short_mark_length));
            }
        }

        this->scale_marks = geometry_freeze(marks);
        this->scales = make_vertical_layout(scale_sequences, font, linespacing, CanvasHorizontalAlignment::Right);
    }
}

void Gaugelet::update_scale() {
	this->cscale = make_text_layout(this->scale.ToString(), this->label_font);
}

void Gaugelet::construct() {
	float scale_ratio = float(range.ToString()->Length());

	this->initialize_meter();
    this->meter_width = this->ch * (scale_ratio + scale_space_ratio + scale_lmark_ratio + mbody_width_ratio);
    this->width = std::fmax(this->caption->LayoutBounds.Width, this->meter_width);
    this->height = this->caption->LayoutBounds.Height + this->scales->LayoutBounds.Height - this->mark_interval * 0.5F;
}

void Gaugelet::fill_extent(float x, float y, float* w, float* h) {
	SET_VALUES(w, this->width, h, this->height);
}

void Gaugelet::draw(CanvasDrawingSession^ ds, float x, float y, float Width, float Height) {
	float caption_x = x + (this->width - this->caption->LayoutBounds.Width) * 0.5F;
    float meter_x = x + (this->width - this->meter_width) * 0.5F;
    float meter_y = y + this->caption->LayoutBounds.Height;
    
    ds->DrawTextLayout(this->caption, caption_x, y, Colors::Khaki);
    this->draw_meter(ds, meter_x, meter_y, this->scale, this->range, this->scales, this->cscale, this->color);
}

void Gaugelet::draw_meter(CanvasDrawingSession^ ds, float x, float y, float scale, int range
	, CanvasTextLayout^ scales, CanvasTextLayout^ cscale, ICanvasBrush^ color) {
	float body_width = this->ch * mbody_width_ratio;
	float body_height = this->mark_interval * float(this->step);
    float body_x = x + this->meter_width - body_width;
    float body_y = y + scales->DrawBounds.Y;
	float current_height = std::fmin(scale * body_height / float(range), body_height);

    ds->FillRectangle(body_x, body_y, body_width, body_height, Colors::Gray);
    ds->FillRectangle(body_x, body_y + body_height - current_height, body_width, current_height, color);
	ds->DrawRectangle(body_x, body_y, body_width, body_height, Colors::GhostWhite);

    { // draw scales and marks
		float scale_mark_xoff = this->ch * scale_lmark_ratio;
		float scale_xoff = -scales->LayoutBounds.X - scale_mark_xoff - this->ch * scale_space_ratio;
		float scale_yoff = -this->mark_interval * 0.5F;
		float scale_width = scales->LayoutBounds.Width;

        // WARNING: the box mode of the Direct2D text layout is not intuitive
		ds->DrawTextLayout(scales, body_x + scale_xoff - scale_width, y + scale_yoff, Colors::WhiteSmoke);
		ds->DrawCachedGeometry(this->scale_marks, body_x - scale_mark_xoff, body_y, Colors::WhiteSmoke);
	}

	{ // draw current value and label
		float scale_x = body_x + (body_width - cscale->LayoutBounds.Width) * 0.5F;
		float scale_y = body_y + body_height;

		ds->DrawTextLayout(cscale, scale_x, scale_y, Colors::Yellow);
	}
}
