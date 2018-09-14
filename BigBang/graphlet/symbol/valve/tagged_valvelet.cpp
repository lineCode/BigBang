#include "graphlet/symbol/valve/tagged_valvelet.hpp"

#include "string.hpp"

#include "math.hpp"
#include "text.hpp"
#include "shape.hpp"
#include "polar.hpp"
#include "paint.hpp"
#include "geometry.hpp"
#include "brushes.hxx"

using namespace WarGrey::SCADA;

using namespace Windows::Foundation;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::Brushes;

static float default_thickness = 1.5F;
static double dynamic_mask_interval = 1.0 / 8.0;

/*************************************************************************************************/
TValvelet::TValvelet(char tag, float radius, double degrees, bool rotate_tag)
	: TValvelet(TValveStatus::Disabled, tag, radius, degrees, rotate_tag) {}

TValvelet::TValvelet(TValveStatus default_status, char tag, float radius, double degrees, bool rotate_tag)
	: ISymbollet(default_status, radius, degrees), rotate_tag(rotate_tag) {
	this->tag = make_wstring(tag);
	this->sradius = radius * 0.5F;
	this->sgradius = radius * 0.5F;
	this->fradius = this->sgradius + default_thickness * 2.0F;

	{ // locate
		float sign_distance = radius - default_thickness * 0.5F - this->sradius;
		float body_distance = radius - default_thickness * 0.5F - this->sgradius;
	
		circle_point(sign_distance, this->degrees - 90.0, &this->sign_cx, &this->sign_cy);
		circle_point(body_distance, this->degrees + 90.0, &this->body_cx, &this->body_cy);
	}
}

void TValvelet::construct() {
	double adjust_degrees = this->degrees + 90.0;
	this->frame = polar_rectangle(this->fradius, 60.0, adjust_degrees);
	this->sign_body = circle(this->sradius);
	this->skeleton = polar_sandglass(this->sgradius, adjust_degrees);
	this->body = geometry_freeze(this->skeleton);

	{ // make sign
		auto tag_layout = make_text_layout(this->tag, make_bold_text_format("Monospace", this->sradius * 1.2F));
		auto tag_shape = paragraph(tag_layout);
		Rect tagbox = tag_layout->DrawBounds;

		this->sign = geometry_freeze((this->rotate_tag) ? geometry_rotate(tag_shape, this->degrees) : tag_shape);
		this->tag_xoff = tagbox.Width * 0.5F + tagbox.X;
		this->tag_yoff = tagbox.Height * 0.5F + tagbox.Y;
	}
}

void TValvelet::update(long long count, long long interval, long long uptime) {
	double adjust_degrees = this->degrees + 90.0;

	switch (this->get_status()) {
	case TValveStatus::Opening: {
		this->mask_percentage
			= ((this->mask_percentage < 0.0) || (this->mask_percentage >= 1.0))
			? 0.0
			: this->mask_percentage + dynamic_mask_interval;

		this->mask = polar_masked_sandglass(this->sgradius, adjust_degrees, -this->mask_percentage);
		this->notify_updated();
	} break;
	case TValveStatus::Closing: {
		this->mask_percentage
			= ((this->mask_percentage <= 0.0) || (this->mask_percentage > 1.0))
			? 1.0
			: this->mask_percentage - dynamic_mask_interval;

		this->mask = polar_masked_sandglass(this->sgradius, adjust_degrees, this->mask_percentage);
		this->notify_updated();
	} break;
	}
}

void TValvelet::fill_valve_origin(float* x, float* y) {
	SET_BOX(x, this->body_cx);
	SET_BOX(y, this->body_cy);
}

void TValvelet::prepare_style(TValveStatus status, TValveStyle& s) {
	switch (status) {
	case TValveStatus::Disabled: {
		CAS_SLOT(s.mask_color, Colours::Teal);
	}; break;
	case TValveStatus::Open: {
		CAS_SLOT(s.body_color, Colours::Green);
	}; break;
	case TValveStatus::Opening: {
		CAS_SLOT(s.mask_color, Colours::Green);
	}; break;
	case TValveStatus::OpenReady: {
		CAS_VALUES(s.skeleton_color, Colours::Cyan, s.mask_color, Colours::ForestGreen);
	}; break;
	case TValveStatus::Unopenable: {
		CAS_VALUES(s.skeleton_color, Colours::Red, s.mask_color, Colours::Green);
	}; break;
	case TValveStatus::Closed: {
		CAS_SLOT(s.body_color, Colours::Gray);
	}; break;
	case TValveStatus::Closing: {
		CAS_SLOT(s.mask_color, Colours::DarkGray);
	}; break;
	case TValveStatus::CloseReady: {
		CAS_VALUES(s.skeleton_color, Colours::Cyan, s.mask_color, Colours::DimGray);
	}; break;
	case TValveStatus::Unclosable: {
		CAS_VALUES(s.skeleton_color, Colours::Red, s.mask_color, Colours::DarkGray);
	}; break;
	case TValveStatus::FakeOpen: {
		CAS_VALUES(s.frame_color, Colours::Red, s.body_color, Colours::ForestGreen);
	}; break;
	case TValveStatus::FakeClose: {
		CAS_VALUES(s.frame_color, Colours::Red, s.body_color, Colours::DimGray);
	}; break;
	}

	CAS_SLOT(s.skeleton_color, Colours::DarkGray);
	CAS_SLOT(s.body_color, Colours::Background);
	CAS_SLOT(s.tag_color, s.skeleton_color);

	// NOTE: The others can be nullptr;
}

void TValvelet::on_status_changed(TValveStatus status) {
	double adjust_degrees = this->degrees + 90.0;

	switch (status) {
	case TValveStatus::Unopenable: {
		if (this->bottom_up_mask == nullptr) {
			this->bottom_up_mask = polar_masked_sandglass(this->sgradius, adjust_degrees, -0.80);
		}
		this->mask = this->bottom_up_mask;
	} break;
	case TValveStatus::Unclosable: case TValveStatus::Disabled: {
		if (this->top_down_mask == nullptr) {
			this->top_down_mask = polar_masked_sandglass(this->sgradius, adjust_degrees, 0.80);
		}
		this->mask = this->top_down_mask;
	} break;
	case TValveStatus::OpenReady: {
		if (this->bottom_up_ready_mask == nullptr) {
			this->bottom_up_ready_mask = polar_masked_sandglass(this->sgradius, adjust_degrees, -0.70);
		}
		this->mask = this->bottom_up_ready_mask;
	} break;
	case TValveStatus::CloseReady: {
		if (this->top_down_ready_mask == nullptr) {
			this->top_down_ready_mask = polar_masked_sandglass(this->sgradius, adjust_degrees, 0.70);
		}
		this->mask = this->top_down_ready_mask;
	} break;
	default: {
		this->mask = nullptr;
		this->mask_percentage = -1.0;
	}
	}
}

void TValvelet::draw(CanvasDrawingSession^ ds, float x, float y, float Width, float Height) {
	const TValveStyle style = this->get_style();
	float radiusX = this->width * 0.5F - default_thickness;
	float radiusY = this->height * 0.5F - default_thickness;
	float cx = x + radiusX + default_thickness;
	float cy = y + radiusY + default_thickness;
	float scx = cx + this->sign_cx;
	float scy = cy + this->sign_cy;
	float bcx = cx + this->body_cx;
	float bcy = cy + this->body_cy;

	{ // draw sign as the tag
		ds->DrawLine(scx, scy, bcx, bcy, style.skeleton_color, default_thickness * 1.618F);
		ds->FillGeometry(this->sign_body, scx, scy, Colours::Background);
		ds->DrawGeometry(this->sign_body, scx, scy, style.tag_color, default_thickness);
		ds->DrawCachedGeometry(this->sign, scx - this->tag_xoff, scy - this->tag_yoff, style.tag_color);
	}

	ds->DrawCachedGeometry(this->body, bcx, bcy, style.body_color);

	if (style.mask_color != nullptr) {
		auto mask = ((this->mask == nullptr) ? this->skeleton : this->mask);

		ds->FillGeometry(mask, bcx, bcy, style.mask_color);
		ds->DrawGeometry(mask, bcx, bcy, style.mask_color, default_thickness);
	}

	if (style.frame_color != nullptr) {
		ds->DrawGeometry(this->frame, bcx, bcy, style.frame_color, default_thickness);
	}

	ds->DrawGeometry(this->skeleton, bcx, bcy, style.skeleton_color, default_thickness);
}

/*************************************************************************************************/
MotorValvelet::MotorValvelet(float radius, double degrees, bool rotate_tag)
	: TValvelet('M', radius, degrees, rotate_tag) {}

MotorValvelet::MotorValvelet(TValveStatus default_status, float radius, double degrees, bool rotate_tag)
	: TValvelet(default_status, 'M', radius, degrees, rotate_tag) {}