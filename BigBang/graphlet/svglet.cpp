#include "graphlet/svglet.hpp"
#include "planet.hpp"

#include "colorspace.hpp"
#include "path.hpp"
#include "draw.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::UI;
using namespace Windows::Foundation;

using namespace Windows::Storage;
using namespace Windows::Storage::Streams;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::Svg;

/*************************************************************************************************/
ISvglet::ISvglet(Platform::String^ file, float width, float height, Platform::String^ rootdir) {
	this->viewport.Width = width;
	this->viewport.Height = height;
	this->ms_appx_svg = ms_appx_file(file, ".svg", rootdir);
}

ISvglet::~ISvglet() {
	this->unload(this->ms_appx_svg);
}

void ISvglet::construct() {
	this->load(this->ms_appx_svg, 0);
}

void ISvglet::on_appx(Uri^ ms_appx_svg, CanvasSvgDocument^ doc_svg, int hint) {
	this->graph_svg = doc_svg;
	this->root = this->graph_svg->Root;

	if ((this->viewport.Width <= 0.0F) || (this->viewport.Height <= 0.0F)) {
		CanvasSvgLengthUnits width_units, height_units;		
		float width = this->get_length_attribute("width", &width_units, false);
		float height = this->get_length_attribute("height", &height_units, false);

		// TODO: what if one of the lengths is measured in percentage
		if (this->viewport.Width <= 0.0F) {
			if (this->viewport.Height <= 0.0F) {
				this->viewport.Height = height;
			}
			this->viewport.Width = width * (this->viewport.Height / height);
		} else {
			this->viewport.Height = height * (this->viewport.Width / width);
		}
	}

	root->SetLengthAttribute("width", 100.0F, CanvasSvgLengthUnits::Percentage);
	root->SetLengthAttribute("height", 100.0F, CanvasSvgLengthUnits::Percentage);

	this->on_ready();
}

bool ISvglet::ready() {
	return (this->graph_svg != nullptr);
}

void ISvglet::fill_extent(float x, float y, float* w, float* h) {
	SET_VALUES(w, this->viewport.Width, h, this->viewport.Height);
}

void ISvglet::draw(CanvasDrawingSession^ ds, float x, float y, float Width, float Height) {
	ds->DrawSvg(this->graph_svg, this->viewport, x, y);
}

void ISvglet::draw_progress(CanvasDrawingSession^ ds, float x, float y, float Width, float Height) {
	Platform::String^ hint = file_name_from_path(this->ms_appx_svg);

	draw_invalid_bitmap(hint, ds, x, y, this->viewport.Width, this->viewport.Height);
}

/*************************************************************************************************/
Color ISvglet::get_fill_color(Platform::String^ id, Color& default_color) {
	return this->get_child_color_attribute(id, "fill", default_color);
}

void ISvglet::set_fill_color(Platform::String^ id, Windows::UI::Color& c) {
	this->set_child_color_attribute(id, "fill", c);
}

void ISvglet::set_fill_color(Platform::String^ id, unsigned int hex, double alpha) {
	this->set_fill_color(id, rgba(hex, alpha));
}

void ISvglet::set_fill_color(Platform::String^ id, WarGrey::SCADA::Colour^ brush) {
	this->set_fill_color(id, brush->Color);
}

Color ISvglet::get_stroke_color(Platform::String^ id, Color& default_color) {
	return this->get_child_color_attribute(id, "stroke", default_color);
}

void ISvglet::set_stroke_color(Platform::String^ id, Windows::UI::Color& c) {
	this->set_child_color_attribute(id, "stroke", c);
}

void ISvglet::set_stroke_color(Platform::String^ id, unsigned int hex, double alpha) {
	this->set_stroke_color(id, rgba(hex, alpha));
}

void ISvglet::set_stroke_color(Platform::String^ id, WarGrey::SCADA::Colour^ brush) {
	this->set_stroke_color(id, brush->Color);
}

/*************************************************************************************************/
float ISvglet::get_length_attribute(Platform::String^ name, CanvasSvgLengthUnits* units, bool inherited) {
	float value = 100.0F; // This value is also the default length if both the client program and the SVG itself do not specific the size.
	(*units) = CanvasSvgLengthUnits::Percentage;

	if (this->root != nullptr) {
		if (this->root->IsAttributeSpecified(name, inherited)) {
			value = this->root->GetLengthAttribute(name, units);
		}
	}

	return value;
}

Color ISvglet::get_child_color_attribute(Platform::String^ id, Platform::String^ attribute_name, Color& default_color) {
	/** WARNING
	* This method is intended to be invoked correctly, so itself does not check the input.
	* That means the `id` should be defined in the SVG document, or the application will crash in development stage.
	*/
	CanvasSvgNamedElement^ child = this->graph_svg->FindElementById(id);

	return child->IsAttributeSpecified(attribute_name) ? child->GetColorAttribute(attribute_name) : default_color;
}

void ISvglet::set_child_color_attribute(Platform::String^ id, Platform::String^ attribute_name, Windows::UI::Color& c) {
	// Warnings see `ISvglet::get_child_color_attribute`
	CanvasSvgNamedElement^ child = this->graph_svg->FindElementById(id);

	child->SetColorAttribute(attribute_name, c);
}

/*************************************************************************************************/
Svgmaplet::Svgmaplet(Platform::String^ file, Platform::String^ rootdir)
	: Svgmaplet(file, 0.0F, 0.0F, rootdir) {}

Svgmaplet::Svgmaplet(Platform::String^ file, float width, float height, Platform::String^ rootdir)
	: ISvglet(file, width, height, rootdir) {}

void Svgmaplet::on_ready() {}
