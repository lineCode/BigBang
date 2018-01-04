#pragma once

#include "snip.hpp"
#include "modbus.hpp"

namespace WarGrey::SCADA {
    private class Statusbarlet : public WarGrey::SCADA::Snip {
    public:
		~Statusbarlet() noexcept;
		Statusbarlet(Platform::String^ caption, Platform::String^ plc,
			WarGrey::SCADA::IModbusConfirmation* callback,
			uint16 start_address, uint16 quantity, uint16 end_address,
			WarGrey::SCADA::ISyslogReceiver* ui_receiver = nullptr);

    public:
        void load() override;
        void update(long long count, long long interval, long long uptime, bool is_slow) override;
        void draw(Microsoft::Graphics::Canvas::CanvasDrawingSession^ ds, float x, float y, float Width, float Height) override;
        void fill_extent(float x, float y, float* w = nullptr, float* h = nullptr) override;

    private:
		Microsoft::Graphics::Canvas::Text::CanvasTextLayout^ caption;
        WarGrey::SCADA::IModbusClient* client;

	private:
		uint16 start_address;
		uint16 end_address;
		uint16 quantity;
    };

	private class Statuslinelet : public WarGrey::SCADA::Snip, public WarGrey::SCADA::ISyslogReceiver {
	public:
		~Statuslinelet() noexcept {};
		Statuslinelet(WarGrey::SCADA::Log level) : ISyslogReceiver(level) {}

	public:
		void load() override;
		void draw(Microsoft::Graphics::Canvas::CanvasDrawingSession^ ds, float x, float y, float Width, float Height) override;
		void fill_extent(float x, float y, float* w = nullptr, float* h = nullptr) override;

	public:
		void on_log_message(WarGrey::SCADA::Log level, Platform::String^ message,
			WarGrey::SCADA::SyslogMetainfo& data, Platform::String^ topic) override;

	private:
		Microsoft::Graphics::Canvas::Text::CanvasTextLayout^ status;
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ color;
	};
}
