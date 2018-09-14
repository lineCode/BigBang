#pragma once

#include "graphlet/primitive.hpp"

namespace WarGrey::SCADA {
	private enum class GateValveStatus {
		Disabled,
		Open, Opening, Unopenable, OpenReady,
		Closed, Closing, Unclosable, CloseReady,
		FakeOpen, FakeClose,
		_ };

	private struct GateValveStyle {
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ frame_color;
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ body_color;
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ skeleton_color;
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ mask_color;
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ stem_color;
	};

	WarGrey::SCADA::GateValveStyle make_manual_valve_style(Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ color = nullptr);

	private class GateValvelet : public WarGrey::SCADA::ISymbollet<WarGrey::SCADA::GateValveStatus, WarGrey::SCADA::GateValveStyle> {
	public:
		GateValvelet(WarGrey::SCADA::GateValveStatus default_status, float radius, double degrees = 0.0);
		GateValvelet(float radius, double degrees = 0.0);

	public:
		void construct() override;
		void update(long long count, long long interval, long long uptime) override;
		void draw(Microsoft::Graphics::Canvas::CanvasDrawingSession^ ds, float x, float y, float Width, float Height) override;

	protected:
		void prepare_style(WarGrey::SCADA::GateValveStatus status, WarGrey::SCADA::GateValveStyle& style) override;
		void on_status_changed(WarGrey::SCADA::GateValveStatus status) override;

	private:
		Microsoft::Graphics::Canvas::Geometry::CanvasGeometry^ mask;
		Microsoft::Graphics::Canvas::Geometry::CanvasGeometry^ bottom_up_mask;
		Microsoft::Graphics::Canvas::Geometry::CanvasGeometry^ top_down_mask;
		Microsoft::Graphics::Canvas::Geometry::CanvasGeometry^ bottom_up_ready_mask;
		Microsoft::Graphics::Canvas::Geometry::CanvasGeometry^ top_down_ready_mask;
		Microsoft::Graphics::Canvas::Geometry::CanvasGeometry^ skeleton;
		Microsoft::Graphics::Canvas::Geometry::CanvasGeometry^ frame;
		Microsoft::Graphics::Canvas::Geometry::CanvasGeometry^ shaft;
		Microsoft::Graphics::Canvas::Geometry::CanvasCachedGeometry^ body;
		
	private:
		float sgradius;
		float fradius;

	private:
		double mask_percentage;
	};
}