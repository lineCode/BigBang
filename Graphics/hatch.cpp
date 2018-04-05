#include <algorithm>

#include "hatch.hpp"

#include "box.hpp"
#include "text.hpp"
#include "shape.hpp"
#include "geometry.hpp"
#include "transformation.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::Foundation;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Geometry;

static const float scale_lmark_ratio = 1.0F;
static const float scale_space_ratio = 1.0F;

inline static Platform::String^ make_scale_string(float scale, unsigned int width, float* scale_xoff) {
	Platform::String^ s = scale.ToString();
	(*scale_xoff) = float(width - s->Length());

	return s;
}

static CanvasGeometry^ make_vhatch(float width, float interval, unsigned char step, float x = 0.0F, float y = 0.0F) {
	CanvasPathBuilder^ mark = ref new CanvasPathBuilder(CanvasDevice::GetSharedDevice());
	float short_x = x + width * (1.0F - 0.618F);
	float height = interval * step;

	mark->BeginFigure(x + width, y);
	mark->AddLine(x + width, y + height);
	for (unsigned char i = 0; i <= step; i++) {
		float ythis = interval * float(i) + y;

		mark->EndFigure(CanvasFigureLoop::Open);
		mark->BeginFigure((i % 2 == 0) ? x : short_x, ythis);
		mark->AddLine(x + width, ythis);
	}
	mark->EndFigure(CanvasFigureLoop::Open);

	return geometry_stroke(CanvasGeometry::CreatePath(mark), 1.0F);
}

/*************************************************************************************************/
CanvasGeometry^ vhatch(float height, float vmin, float vmax, unsigned char step, CanvasTextFormat^ ft, float* mw, float* my, float* mh) {
	Platform::String^ min_mark = vmin.ToString(); 
	Platform::String^ max_mark = vmax.ToString();
	unsigned int span = std::max(max_mark->Length(), min_mark->Length());
	CanvasTextFormat^ font = ((ft == nullptr) ? make_text_format(8.0F) : ft);
	TextExtent ts = get_text_extent(max_mark, font);
	float ch = ts.width / span;
	float em = ts.height - ts.tspace - ts.bspace;
	float mark_width = ch * scale_lmark_ratio;
	float mark_x = float(span) * ch + scale_space_ratio * ch;
	float mark_y = em * 0.5F;
	float interval = (height - em) / float(step);
	float delta = (vmax - vmin) / float(step);
	float scale_xoff;

	auto marks = make_vhatch(mark_width, interval, step, mark_x, mark_y);
	for (char i = 0; i <= step; i += 2) {
		Platform::String^ scale = make_scale_string(vmax - delta * float(i), span, &scale_xoff);

		auto translation = make_translation_matrix(scale_xoff * ch, interval * float(i) - ts.tspace);
		marks = geometry_union(marks, paragraph(make_text_layout(scale, font)), translation);
	}

	SET_BOX(mw, mark_x + mark_width);
	SET_BOX(my, mark_y);
	SET_BOX(mh, interval * float(step));

	return marks;
}