#include <cwchar>
#include <cstdarg>
#include <cstdlib>

#include "text.hpp"
#include "paint.hpp"
#include "tongue.hpp"
#include "string.hpp"
#include "textlet.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::UI;
using namespace Windows::UI::Text;
using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::Text;

Labellet::Labellet(const wchar_t *fmt, ...) {
	VSWPRINT(label, fmt);
    this->set_text(label);
}

Labellet::Labellet(Platform::String^ content) {
    this->set_text(content);
}

void Labellet::set_text(Platform::String^ content) {
    this->content = content;
    if (this->label_font == nullptr) {
        this->label_font = make_text_format();
    }
}

void Labellet::fill_extent(float x, float y, float* w, float* h) {
    TextExtent ts = get_text_extent(content, label_font);

    SET_VALUES(w, ts.width, h, ts.height);
};

void Labellet::draw(CanvasDrawingSession^ ds, float x, float y, float Width, float Height) {
    ds->DrawText(content, x, y, Colors::Snow, label_font);
}

/*************************************************************************************************/
Scalelet::Scalelet(Platform::String^ unit, Platform::String^ label, Platform::String^ subscript, Color& lcolor, Color& scolor) {
	this->scale_font = make_text_format("Courier New");
	this->unit = make_text_layout(speak(unit), this->scale_font);

	if (label != nullptr) {
		Platform::String^ symbol = speak(label);
		Platform::String^ suffix = " =";

		if (subscript == nullptr) {
			this->label = make_text_layout(symbol + suffix, this->scale_font);
		} else {
			Platform::String^ subsymbol = speak(subscript);
			float subsize = this->scale_font->FontSize * 0.618F;
			unsigned int symcount = symbol->Length();
			unsigned int subcount = subsymbol->Length();

			this->label = make_text_layout(symbol + subsymbol + suffix, this->scale_font);
			this->label->SetFontSize(symcount, subcount, subsize);
			this->label->SetFontStyle(symcount, subcount, FontStyle::Italic);
		}
	}

	this->label_color = make_solid_brush(lcolor);
	this->scale_color = make_solid_brush(scolor);
	this->set_scale(0.0F);
}

void Scalelet::set_scale(float value) {
	this->scale = make_text_layout(" " + value.ToString(), this->scale_font);
}

void Scalelet::fill_extent(float x, float y, float* w, float* h) {
	float width = this->scale->LayoutBounds.Width + this->unit->LayoutBounds.Width;
	float height = this->unit->LayoutBounds.Height;

	if (this->label != nullptr) {
		width += this->label->LayoutBounds.Width;
	}

	SET_VALUES(w, width, h, height);
};

void Scalelet::draw(CanvasDrawingSession^ ds, float x, float y, float Width, float Height) {
	float start_x = x;
	float swidth = this->scale->LayoutBounds.Width;

	if (this->label != nullptr) {
		ds->DrawTextLayout(this->label, x, y, this->label_color);
		start_x += this->label->LayoutBounds.Width;
	}

	ds->DrawTextLayout(this->scale, start_x, y, this->scale_color);
	ds->DrawTextLayout(this->unit, start_x + swidth, y, this->label_color);
}
