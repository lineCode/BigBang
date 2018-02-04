#pragma once

#include "syslog.hpp"

namespace WarGrey::SCADA {
	private class ISprite abstract {
	public:
		virtual WarGrey::SCADA::Syslog* get_logger() = 0;

	public:
		virtual void construct() {}
		virtual void fill_extent(float x, float y, float* width = nullptr, float* height = nullptr) = 0;
		virtual void update(long long count, long long interval, long long uptime, bool is_slow) {}
		virtual void draw(Microsoft::Graphics::Canvas::CanvasDrawingSession^ ds, float x, float y, float Width, float Height) = 0;

	public:
		virtual void on_hover(float local_x, float local_y, bool shifted, bool controled) {};
		virtual void on_tap(float local_x, float local_y, bool shifted, bool controled) {};
		virtual void on_right_tap(float local_x, float local_y, bool shifted, bool controled) {};
		virtual void on_goodbye() {};

	public:
		void enable_events(bool yes) { this->deal_with_events = yes; }
		bool handles_events() { return this->deal_with_events; }

	public:
		Microsoft::Graphics::Canvas::CanvasRenderTarget^ take_snapshot(float dpi = 96.0F);
		void save(Platform::String^ path, float dpi = 96.0F);

	private:
		bool deal_with_events = false;
	};
}
