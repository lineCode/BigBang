#include <cwchar>

#include "graphlet/textlet.hpp"

#include "forward.hpp"
#include "paint.hpp"
#include "tongue.hpp"
#include "string.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::Foundation;
using namespace Windows::System;
using namespace Windows::UI::Text;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;

static CanvasTextFormat^ default_text_font = make_bold_text_format();
static CanvasTextFormat^ default_unit_font = make_bold_text_format("Cambria", 14.0F);
static CanvasTextFormat^ default_math_font = make_bold_text_format("Cambria Math", 16.0F);

static ICanvasBrush^ default_text_color = Colours::Silver;
static ICanvasBrush^ default_number_color = Colours::Yellow;
static ICanvasBrush^ default_unit_color = Colours::make(0x23EBB9U);

static void fill_vmetrics(CanvasTextLayout^ layout, TextExtent& num_box, TextExtent& unit_box
	, TextExtent* label_box, float* tspace, float* bspace, float* height = nullptr) {
	(*label_box) = ((layout == nullptr) ? num_box : get_text_extent(layout));
	(*tspace) = std::fminf(label_box->tspace, std::fminf(num_box.tspace, unit_box.tspace));
	(*bspace) = std::fminf(label_box->bspace, std::fminf(num_box.bspace, unit_box.bspace));

	if (height != nullptr) {
		float link = label_box->height - label_box->tspace - unit_box.bspace;
		float nink = num_box.height - num_box.tspace - unit_box.bspace;
		float uink = unit_box.height - unit_box.tspace - unit_box.bspace;
		float ink_height = std::fmaxf(std::fmaxf(nink, uink), link);

		(*height) = (*tspace) + ink_height + (*bspace);
	}
}

static Platform::String^ accumulate_number(Platform::String^ src, double acc) {
	return (std::wcstod(src->Data(), nullptr) + acc).ToString();
}

/*************************************************************************************************/
DimensionStyle WarGrey::SCADA::make_plain_dimension_style(float nfsize, float ufsize, int precision) {
	DimensionStyle ds;

	ds.number_font = make_bold_text_format("Cambria Math", nfsize);
	ds.unit_font = make_bold_text_format("Cambria", ufsize);
	ds.precision = precision;

	return ds;
}

DimensionStyle WarGrey::SCADA::make_plain_dimension_style(float nfsize, unsigned int min_number, int precision) {
	auto nf = make_bold_text_format("Cambria Math", nfsize);
	auto uf = make_bold_text_format("Cambria", nfsize * 0.90F);
	DimensionStyle ds;

	ds.number_font = nf;
	ds.unit_font = uf;
	ds.minimize_number_width = get_text_extent("0", nf).width * float(min_number);
	ds.number_xfraction = 0.5F;
	ds.number_leading_space = 0.0F;
	ds.number_trailing_space = 0.0F;
	ds.label_color = Colours::Silver;
	ds.precision = precision;

	return ds;
}

DimensionStyle WarGrey::SCADA::make_setting_dimension_style(float nfsize, unsigned int min_number, ICanvasBrush^ color) {
	auto nf = make_bold_text_format("Cambria Math", nfsize);
	auto uf = make_bold_text_format("Cambria", nfsize);
	DimensionStyle ds;

	ds.number_font = nf;
	ds.unit_font = uf;
	ds.minimize_number_width = get_text_extent("0", nf).width * float(min_number);
	ds.number_border_color = color;
	ds.number_color = color;
	ds.unit_color = color;
	ds.caret_color = color;
	
	return ds;
}

DimensionStyle WarGrey::SCADA::make_highlight_dimension_style(float nfsize, unsigned int min_number
	, ICanvasBrush^ label_color, ICanvasBrush^ label_bgcolor) {
	auto nf = make_bold_text_format("Cambria Math", nfsize);
	auto uf = make_bold_text_format("Cambria", nfsize);
	DimensionStyle ds;

	ds.number_font = nf;
	ds.unit_font = uf;
	ds.minimize_label_width = get_text_extent("O", uf).height;
	ds.label_xfraction = 0.5F;
	ds.minimize_number_width = get_text_extent("0", nf).width * float(min_number);
	ds.number_leading_space = 2.0F;
	ds.number_background_color = Colours::Gray;
	ds.number_color = Colours::Background;
	ds.label_background_color = label_bgcolor;
	ds.label_color = label_color;
	
	return ds;
}

/*************************************************************************************************/
void ITextlet::set_color(ICanvasBrush^ color) {
	this->text_color = ((color == nullptr) ? default_text_color : color);
	this->notify_updated();
}

void ITextlet::set_color(unsigned int color_hex, double alpha) {
	this->set_color(Colours::make(color_hex, alpha));
}

void ITextlet::set_font(CanvasTextFormat^ font, GraphletAnchor anchor) {
	this->moor(anchor);

	this->text_font = ((font == nullptr) ? default_text_font : font);
	this->subscript_fontsize = this->text_font->FontSize * 0.618F;
	this->set_text(this->raw, this->sub_index, this->sub_count, anchor);
	this->on_font_changed();

	this->notify_updated();
}

void ITextlet::set_text(Platform::String^ content, unsigned int subidx, unsigned int subcount, GraphletAnchor anchor) {
	this->raw = content;
	this->sub_index = subidx;
	this->sub_count = subcount;

	this->moor(anchor);

	if (this->text_font == nullptr) {
		this->set_font(default_text_font);
	} else if (this->raw == nullptr) {
		this->text_layout = nullptr;
	} else {
		this->text_layout = make_text_layout(this->raw, this->text_font);
		this->set_subtext();
	}

	this->notify_updated();
}

void ITextlet::set_text(Platform::String^ content, GraphletAnchor anchor) {
	this->set_text(content, 0, 0, anchor);
}

void ITextlet::set_text(Platform::String^ symbol, Platform::String^ subsymbol, Platform::String^ suffix, GraphletAnchor anchor) {
	this->set_text(symbol + subsymbol + suffix, symbol->Length(), subsymbol->Length(), anchor);
}

void ITextlet::set_subtext() {
	if (this->sub_count > 0) {
		this->set_layout_font_size(this->sub_index, this->sub_count);
		this->set_layout_font_style(this->sub_index, this->sub_count, FontStyle::Italic);
	}
}

void ITextlet::set_text(const wchar_t *fmt, ...) {
	VSWPRINT(content, fmt);
	this->set_text(content);
}

void ITextlet::set_layout_font_size(int char_idx, int char_count) {
	this->set_layout_font_size(char_idx, char_count, this->subscript_fontsize);
}

void ITextlet::set_layout_font_size(int char_idx, int char_count, float size) {
	if (this->text_layout != nullptr) {
		this->text_layout->SetFontSize(char_idx, char_count, size);
	}
}

void ITextlet::set_layout_font_style(int char_idx, int char_count, Windows::UI::Text::FontStyle style) {
	if (this->text_layout != nullptr) {
		this->text_layout->SetFontStyle(char_idx, char_count, style);
	}
}

void ITextlet::fill_extent(float x, float y, float* w, float* h) {
	if (this->text_layout != nullptr) {
		auto box = this->text_layout->LayoutBounds;

		SET_VALUES(w, box.Width, h, box.Height);
	} else {
		SET_BOXES(w, h, 0.0F);
	}
}

void ITextlet::fill_margin(float x, float y, float* t, float* r, float* b, float* l) {
	if (this->text_layout != nullptr) {
		TextExtent te = get_text_extent(this->text_layout);

		SET_VALUES(t, te.tspace, b, te.bspace);
		SET_VALUES(l, te.lspace, r, te.rspace);
	} else {
		IGraphlet::fill_margin(x, y, t, r, b, l);
	}
}

void ITextlet::draw(CanvasDrawingSession^ ds, float x, float y, float Width, float Height) {
	if (this->text_layout != nullptr) {
		if (this->text_color == nullptr) {
			this->set_color();
		}

		ds->DrawTextLayout(this->text_layout, x, y, this->text_color);
	}
}

/*************************************************************************************************/
Labellet::Labellet(const wchar_t *fmt, ...) {
	VSWPRINT(label, fmt);
    this->set_text(label);
}

Labellet::Labellet(Platform::String^ caption, CanvasTextFormat^ font, ICanvasBrush^ color) {
	this->set_font(font);
	this->set_color(color);
	this->set_text(caption);
}

Labellet::Labellet(Platform::String^ caption, Platform::String^ subscript, CanvasTextFormat^ font, ICanvasBrush^ color) {
	this->set_font(font);
	this->set_color(color);
	this->set_text(caption, subscript);
}

Labellet::Labellet(Platform::String^ caption, CanvasTextFormat^ font, unsigned int color_hex, double alpha) {
	this->set_font(font);
	this->set_color(color_hex, alpha);
	this->set_text(caption);
}

Labellet::Labellet(Platform::String^ caption, Platform::String^ subscript, CanvasTextFormat^ font, unsigned int color_hex, double alpha) {
	this->set_font(font);
	this->set_color(color_hex, alpha);
	this->set_text(caption, subscript);
}

Labellet::Labellet(Platform::String^ caption, ICanvasBrush^ color) {
	this->set_color(color);
	this->set_text(caption);
}

Labellet::Labellet(Platform::String^ caption, Platform::String^ subscript, ICanvasBrush^ color) {
	this->set_color(color);
	this->set_text(caption, subscript);
}

Labellet::Labellet(Platform::String^ caption, unsigned int color_hex, double alpha) {
	this->set_color(color_hex, alpha);
	this->set_text(caption);
}

Labellet::Labellet(Platform::String^ caption, Platform::String^ subscript, unsigned int color_hex, double alpha) {
	this->set_color(color_hex, alpha);
	this->set_text(caption, subscript);
}

/*************************************************************************************************/
IEditorlet::IEditorlet(DimensionStatus status, DimensionStyle& style, Platform::String^ unit
	, Platform::String^ label, Platform::String^ subscript)
	: IEditorlet(status, unit, label, subscript) {
	
	/** TODO: Why does not it work if pass the `style` to IStatuslet */
	this->set_style(style);
}

IEditorlet::IEditorlet(DimensionStatus status, Platform::String^ unit, Platform::String^ label, Platform::String^ subscript)
	: IStatuslet(status), unit(unit) {
	
	this->set_text(speak(label), speak(subscript));

	/** TODO: Why does not it work if pass the `status` to IStatuslet */
	this->set_status(status);
}

void IEditorlet::construct() {
	this->set_value(0.0, true);
}

void IEditorlet::update(long long count, long long interval, long long uptime) {
	if (count % 2 == 0) {
		if (this->has_caret()) {
			this->flashing = !this->flashing;
			this->notify_updated();
		}
	}
}

void IEditorlet::fill_extent(float x, float y, float* w, float* h) {
	DimensionStyle style = this->get_style();

	if (w != nullptr) {
		TextExtent nbox = (this->has_caret() ? this->caret_box : this->number_box);
		
		(*w) = std::fmaxf(nbox.width, style.minimize_number_width) + this->unit_box.width;

		if (this->text_layout != nullptr) {
			(*w) += std::fmaxf(this->text_layout->LayoutBounds.Width, style.minimize_label_width);
			(*w) += style.number_leading_space;
		}

		if (this->unit_layout != nullptr) {
			(*w) += style.number_trailing_space;
		}
	}

	if (h != nullptr) {
		TextExtent label_box;
		float tspace, bspace;

		fill_vmetrics(this->text_layout, this->number_box, this->unit_box, &label_box, &tspace, &bspace, h);
	}
}

void IEditorlet::fill_margin(float x, float y, float* t, float* r, float* b, float* l) {
	TextExtent label_box;
	float tspace, bspace;

	fill_vmetrics(this->text_layout, this->number_box, this->unit_box, &label_box, &tspace, &bspace);
	
	if (this->text_layout == nullptr) {
		TextExtent nbox = (this->has_caret() ? this->caret_box : this->number_box);
		DimensionStyle style = this->get_style();
		float region_width = std::fmaxf(nbox.width, style.minimize_number_width);
		
		label_box.lspace = (region_width - nbox.width) * style.number_xfraction + nbox.lspace;
	} else {
		DimensionStyle style = this->get_style();
		float region_width = std::fmaxf(label_box.width, style.minimize_label_width);
		
		label_box.lspace += ((region_width - label_box.width) * style.label_xfraction);
	}

	SET_VALUES(l, label_box.lspace, r, this->unit_box.rspace);
	SET_VALUES(t, tspace, b, bspace);
}

void IEditorlet::on_value_changed(double value) {
	DimensionStyle style = this->get_style();

	this->number = flstring(value, style.precision);
	this->number_layout = make_text_layout(this->number, style.number_font);
	this->number_box = get_text_extent(this->number_layout);
}

void IEditorlet::on_status_changed(DimensionStatus status) {
	this->enable_events(status == DimensionStatus::Input);
}

bool IEditorlet::on_char(VirtualKey key, bool wargrey_keyboard) {
	static unsigned int num0 = static_cast<unsigned int>(VirtualKey::Number0);
	static unsigned int pad0 = static_cast<unsigned int>(VirtualKey::NumberPad0);
	unsigned int keycode = static_cast<unsigned int>(key);
	bool handled = false;

	switch (key) {
	case VirtualKey::Subtract: {
		if (this->input_number == nullptr) {
			this->input_number = "-";
		}
		handled = true;
	}; break;
	case VirtualKey::Decimal: {
		if (this->decimal_position < 0) {
			this->decimal_position = this->input_number->Length() - 1;
			this->input_number += ".";
		}
		handled = true;
	}; break;
	case VirtualKey::PageUp: {
		this->input_number = accumulate_number(this->input_number, 10.0);
		handled = true;
	}; break;
	case VirtualKey::Up: {
		this->input_number = accumulate_number(this->input_number, 1.0);
		handled = true;
	}; break;
	case VirtualKey::PageDown: {
		this->input_number = accumulate_number(this->input_number, -10.0);
		handled = true;
	}; break;
	case VirtualKey::Down: {
		this->input_number = accumulate_number(this->input_number, -1.0);
		handled = true;
	}; break;
	case VirtualKey::Back: {
		if (this->input_number != nullptr) {
			int count = this->input_number->Length() - 1;

			this->input_number = ref new Platform::String(this->input_number->Data(), count);

			if (count < this->decimal_position) {
				this->decimal_position = -1;
			}
		}
		handled = true;
	}; break;
	default: {
		if ((VirtualKey::Number0 <= key) && (key <= VirtualKey::Number9)) {
			this->input_number += (keycode - num0).ToString();
			handled = true;
		} else if ((VirtualKey::NumberPad0 <= key) && (key <= VirtualKey::NumberPad9)) {
			this->input_number += (keycode - pad0).ToString();
			handled = true;
		}
	}
	}

	if (handled) {
		DimensionStyle style = this->get_style();

		this->caret_layout = make_text_layout(this->input_number, style.number_font);
		this->caret_box = get_text_extent(this->caret_layout);

		this->notify_updated();
	}

	return handled;
}

void IEditorlet::own_caret(bool on) {
	this->flashing = on;

	if (on) {
		this->input_number = nullptr;
		this->caret_layout = nullptr;
		this->caret_box = TextExtent();
		this->decimal_position = -1;
	}
}

long double IEditorlet::get_input_number() {
	long double v = 0.0;

	if (this->input_number != nullptr) {
		v = std::wcstod(this->input_number->Data(), nullptr);
	}

	return v;
}

void IEditorlet::prepare_style(DimensionStatus status, DimensionStyle& style) {
	CAS_SLOT(style.number_color, default_number_color);
	CAS_SLOT(style.unit_color, default_unit_color);
	CAS_SLOT(style.label_color, style.unit_color);

	CAS_SLOT(style.number_font, default_math_font);
	CAS_SLOT(style.unit_font, default_unit_font);
	CAS_SLOT(style.label_font, style.unit_font);

	CAS_SLOT(style.caret_color, Colours::Foreground); 

	FLCAS_SLOT(style.minimize_label_width, 0.0F);
	FLCAS_SLOT(style.label_xfraction, 1.0F);
	FLCAS_SLOT(style.number_leading_space, get_text_extent("0", style.number_font).width);

	switch (status) {
	case DimensionStatus::Input: {
		FLCAS_SLOT(style.minimize_number_width, get_text_extent("123456.789", style.number_font).width);
		FLCAS_SLOT(style.number_xfraction, 0.0F);
		FLCAS_SLOT(style.number_trailing_space, 0.0F);
		style.precision = 0;
	}; break;
	case DimensionStatus::Normal: case DimensionStatus::Highlight: {
		FLCAS_SLOT(style.minimize_number_width, 0.0F);
		FLCAS_SLOT(style.number_xfraction, 0.5F);
		FLCAS_SLOT(style.number_trailing_space, 0.0F);
		ICAS_SLOT(style.precision, 1);
	}; break;
	}

	// NOTE: the others can be `nullptr`
}

void IEditorlet::apply_style(DimensionStyle& style) {
	this->set_color(style.label_color);
	this->set_font(style.label_font);

	this->number_layout = make_text_layout(this->number, style.number_font);
	this->number_box = get_text_extent(this->number_layout);

	if (this->unit != nullptr) {
		this->unit_layout = make_text_layout(this->unit, style.unit_font);
		this->unit_box = get_text_extent(this->unit_layout);
	}
}

void IEditorlet::draw(CanvasDrawingSession^ ds, float x, float y, float Width, float Height) {
	DimensionStyle style = this->get_style();
	CanvasTextLayout^ nlayout = this->number_layout;
	TextExtent nbox = this->number_box;
	TextExtent label_box;
	float tspace, bspace, height, base_y, box_height;
	float number_region_x = 0.0F;
	float number_x = -nbox.width;
	
	fill_vmetrics(this->text_layout, this->number_box, this->unit_box, &label_box, &tspace, &bspace, &height);
	base_y = y + height;
	box_height = height - tspace - bspace;

	if (this->text_layout != nullptr) {
		float region_width = std::fmaxf(label_box.width, style.minimize_label_width);
		float label_x = x + (region_width - label_box.width) * style.label_xfraction;

		if (style.label_background_color != nullptr) {
			ds->FillRectangle(x, y, region_width, height, style.label_background_color);
		}
		
		ds->DrawTextLayout(this->text_layout, label_x, base_y - label_box.height, style.label_color);
		
		if (style.label_border_color != nullptr) {
			ds->DrawRectangle(x + 0.5F, y + 0.5F, region_width - 1.0F, height - 1.0F, style.label_border_color);
		}
		
		x += (region_width + style.number_leading_space);
		number_region_x = x;
	}

	if (this->has_caret()) {
		nbox = this->caret_box;
		nlayout = this->caret_layout;
	}

	{ // draw number
		float region_width = std::fmaxf(nbox.width, style.minimize_number_width);
		float padding_x = ((style.number_border_color != nullptr) ? 1.0F : 0.0F);
		
		if (style.number_background_color != nullptr) {
			ds->FillRectangle(x, y, region_width, height, style.number_background_color);
		}

		if (nlayout != nullptr) {
			number_x = x + (region_width - nbox.width) * style.number_xfraction + padding_x;
			ds->DrawTextLayout(nlayout, number_x, base_y - number_box.height, style.number_color);
		}

		if (style.number_border_color != nullptr) {
			ds->DrawRectangle(x + 0.5F, y + 0.5F, region_width - 1.0F, height - 1.0F, style.number_border_color);
		}

		x += region_width;
	}

	if (this->unit_layout != nullptr) {
		x += style.number_trailing_space;

		if (style.unit_background_color != nullptr) {
			ds->FillRectangle(x, y, unit_box.width, height, style.unit_background_color);
		}

		ds->DrawTextLayout(this->unit_layout, x, base_y - unit_box.height, style.unit_color);

		if (style.unit_border_color != nullptr) {
			ds->DrawRectangle(x + 0.5F, y + 0.5F, unit_box.width - 1.0F, height - 1.0F, style.unit_border_color);
		}
	}

	if (this->flashing) {
		float padding_x = 2.0F;
		float caret_x = std::fmaxf(number_region_x + padding_x, number_x + nbox.width);

		ds->DrawLine(caret_x, y + padding_x, caret_x, y + height - padding_x, style.caret_color);
	}
}

/*************************************************************************************************/
Dimensionlet::Dimensionlet(DimensionStatus default_status, DimensionStyle& default_style, Platform::String^ unit
	, Platform::String^ label, Platform::String^ subscript)
	: IEditorlet(default_status, default_style, unitspeak(unit), label, subscript) {}

Dimensionlet::Dimensionlet(DimensionStyle& default_style, Platform::String^ unit, Platform::String^ label, Platform::String^ subscript)
	: Dimensionlet(DimensionStatus::Normal, default_style, unit, label, subscript) {}

Dimensionlet::Dimensionlet(DimensionStatus default_status, Platform::String^ unit, Platform::String^ label, Platform::String^ subscript)
	: IEditorlet(default_status, unitspeak(unit), label, subscript) {}

Dimensionlet::Dimensionlet(Platform::String^ unit, Platform::String^ label, Platform::String^ subscript)
	: Dimensionlet(DimensionStatus::Normal, unit, label, subscript) {}

/*************************************************************************************************/
Percentagelet::Percentagelet(DimensionStatus default_status, DimensionStyle& default_style
	, Platform::String^ label, Platform::String^ subscript)
	: IEditorlet(default_status, default_style, "%", label, subscript) {}

Percentagelet::Percentagelet(DimensionStyle& default_style, Platform::String^ label, Platform::String^ subscript)
	: Percentagelet(DimensionStatus::Normal, default_style, label, subscript) {}

Percentagelet::Percentagelet(DimensionStatus default_status, Platform::String^ label, Platform::String^ subscript)
	: IEditorlet(default_status, "%", label, subscript) {}

Percentagelet::Percentagelet(Platform::String^ label, Platform::String^ subscript)
	: Percentagelet(DimensionStatus::Normal, label, subscript) {}
