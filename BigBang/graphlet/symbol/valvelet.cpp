#include "graphlet/symbol/valvelet.hpp"

#include "polar.hpp"
#include "paint.hpp"
#include "geometry.hpp"
#include "brushes.hxx"

using namespace WarGrey::SCADA;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::Brushes;

static float default_thickness = 1.5F;
static double dynamic_mask_interval = 1.0 / 8.0;

static ValveStatus default_valve_status = ValveStatus::Manual;
static CanvasSolidColorBrush^ default_sketeton_color = Colours::DarkGray;

ValveStyle WarGrey::SCADA::make_default_valve_style(ValveStatus status) {
	ValveStyle s;

	s.skeleton_color = default_sketeton_color;

	switch (status) {
	case ValveStatus::Manual: s.mask_color = Colours::Teal; break;
	case ValveStatus::Open: s.body_color = Colours::Green; break;
	case ValveStatus::Opening: s.mask_color = Colours::Green; break;
	case ValveStatus::OpenReady: s.skeleton_color = Colours::Cyan; s.mask_color = Colours::ForestGreen; break;
	case ValveStatus::Unopenable: s.skeleton_color = Colours::Red; s.mask_color = Colours::Green; break;
	case ValveStatus::Closed: s.body_color = Colours::LightGray; break;
	case ValveStatus::Closing: s.mask_color = Colours::DarkGray; break;
	case ValveStatus::CloseReady: s.skeleton_color = Colours::Cyan; s.mask_color = Colours::DimGray; break;
	case ValveStatus::Unclosable: s.skeleton_color = Colours::Red; s.mask_color = Colours::DarkGray; break;
	case ValveStatus::FalseOpen: s.border_color = Colours::Red; s.body_color = Colours::ForestGreen; break;
	case ValveStatus::FalseClosed: s.border_color = Colours::Red; s.body_color = Colours::DimGray; break;
	}

	return s;
}

/*************************************************************************************************/
Valvelet::Valvelet(float radius, double degrees) : Valvelet(default_valve_status, radius, degrees) {}

Valvelet::Valvelet(ValveStatus default_status, float radius, double degrees)
	: IStatuslet(default_status, &make_default_valve_style), size(radius * 2.0F), degrees(degrees) {
	this->fradius = radius;
	this->sgradius = this->fradius - default_thickness * 2.0F;
	this->on_status_change(default_status);
}

void Valvelet::construct() {
	float handle_length = this->sgradius * 0.618F;
	auto handler_axis = polar_axis(handle_length, this->degrees);
	auto handler_pole = polar_pole(handle_length, this->degrees, handle_length * 0.1618F);

	this->frame = polar_rectangle(this->fradius, 60.0, this->degrees);
	this->handler = geometry_union(handler_axis, handler_pole);
	this->skeleton = polar_sandglass(this->sgradius, this->degrees);
	this->body = geometry_freeze(this->skeleton);
}

void Valvelet::update(long long count, long long interval, long long uptime) {
	switch (this->get_status()) {
	case ValveStatus::Opening: {
		this->mask_percentage
			= ((this->mask_percentage < 0.0) || (this->mask_percentage >= 1.0))
			? 0.0
			: this->mask_percentage + dynamic_mask_interval;

		this->mask = polar_masked_sandglass(this->sgradius, this->degrees, this->mask_percentage);
	} break;
	case ValveStatus::Closing: {
		this->mask_percentage
			= ((this->mask_percentage <= 0.0) || (this->mask_percentage > 1.0))
			? 1.0
			: this->mask_percentage - dynamic_mask_interval;

		this->mask = polar_masked_sandglass(this->sgradius, this->degrees, -this->mask_percentage);
	} break;
	}
}

void Valvelet::on_status_change(ValveStatus status) {
	switch (status) {
	case ValveStatus::Unopenable: {
		if (this->bottom_up_mask == nullptr) {
			this->bottom_up_mask = polar_masked_sandglass(this->sgradius, this->degrees, 0.80);
		}
		this->mask = this->bottom_up_mask;
	} break;
	case ValveStatus::Unclosable: case ValveStatus::Manual: {
		if (this->top_down_mask == nullptr) {
			this->top_down_mask = polar_masked_sandglass(this->sgradius, this->degrees, -0.80);
		}
		this->mask = this->top_down_mask;
	} break;
	case ValveStatus::OpenReady: {
		if (this->bottom_up_ready_mask == nullptr) {
			this->bottom_up_ready_mask = polar_masked_sandglass(this->sgradius, this->degrees, 0.70);
		}
		this->mask = this->bottom_up_ready_mask;
	} break;
	case ValveStatus::CloseReady: {
		if (this->top_down_ready_mask == nullptr) {
			this->top_down_ready_mask = polar_masked_sandglass(this->sgradius, this->degrees, -0.70);
		}
		this->mask = this->top_down_ready_mask;
	} break;
	default: {
		this->mask = nullptr;
		this->mask_percentage = -1.0;
	}
	}
}

void Valvelet::fill_extent(float x, float y, float* w, float* h) {
	SET_BOXES(w, h, size);
}

double Valvelet::get_direction_degrees() {
	return this->degrees;
}

void Valvelet::draw(CanvasDrawingSession^ ds, float x, float y, float Width, float Height) {
	const ValveStyle style = this->get_style();
	auto skeleton_color = (style.skeleton_color != nullptr) ? style.skeleton_color : default_sketeton_color;
	auto body_color = (style.body_color != nullptr) ? style.body_color : Colours::Background;

	float radius = this->size * 0.5F - default_thickness;
	float cx = x + radius + default_thickness;
	float cy = y + radius + default_thickness;
	
	if (style.border_color != nullptr) {
		ds->DrawGeometry(this->frame, cx, cy, style.border_color, default_thickness);
	}

	ds->DrawCachedGeometry(this->body, cx, cy, body_color);

	if (style.mask_color != nullptr) {
		auto mask = ((this->mask == nullptr) ? this->skeleton : this->mask);
		
		ds->FillGeometry(mask, cx, cy, style.mask_color);
		ds->DrawGeometry(mask, cx, cy, style.mask_color, default_thickness);
	}

	ds->DrawGeometry(this->skeleton, cx, cy, skeleton_color, default_thickness);

	if (style.handler_color != nullptr) {
		ds->DrawGeometry(this->handler, cx, cy, style.handler_color, default_thickness);
	}
}
