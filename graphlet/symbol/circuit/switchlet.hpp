#pragma once
#pragma warning (disable: 4250)

#include "graphlet/primitive.hpp"

namespace WarGrey::SCADA {
	private enum class SwitchState { Normal, Breakdown, _ };

	private struct SwitchStyle {
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ color;
	};

	private class Switchlet
		: public WarGrey::SCADA::ISymbollet<WarGrey::SCADA::SwitchState, WarGrey::SCADA::SwitchStyle>
		, public WarGrey::SCADA::IValuelet<bool> {
	public:
		Switchlet(WarGrey::SCADA::SwitchState default_state, float radius, float thickness = 1.5F, double degrees = 0.0);
		Switchlet(float radius, float thickness = 1.5F, double degrees = 0.0);

	public:
		void construct() override;
		void draw(Microsoft::Graphics::Canvas::CanvasDrawingSession^ ds, float x, float y, float Width, float Height) override;

	protected:
		void prepare_style(WarGrey::SCADA::SwitchState status, WarGrey::SCADA::SwitchStyle& style) override;

	private:
		Microsoft::Graphics::Canvas::Geometry::CanvasCachedGeometry^ endpoints;
		Microsoft::Graphics::Canvas::Geometry::CanvasCachedGeometry^ body;
		
	private:
		float thickness;

	private:
		float left_x;
		float left_y;
		float right_x;
		float right_y;
		float handle_x;
		float handle_y;
	};
}
