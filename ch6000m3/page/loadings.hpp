#pragma once

#include "planet.hpp"
#include "plc.hpp"

#include "decorator/grid.hpp"

#include "graphlet/textlet.hpp"
#include "graphlet/statuslet.hpp"

namespace WarGrey::SCADA {
	private class LoadingsPage : public WarGrey::SCADA::Planet {
	public:
		~LoadingsPage() noexcept;
		LoadingsPage(WarGrey::SCADA::IMRMaster* plc);

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;

	public:
		bool can_select(IGraphlet* g) override;
		void on_tap(IGraphlet* g, float x, float y, bool shifted, bool ctrled) override;

	private:
		WarGrey::SCADA::IMRMaster* device;
		WarGrey::SCADA::PLCConfirmation* dashboard;
		Windows::UI::Xaml::Controls::MenuFlyout^ gate_valve_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ motor_valve_op;

	private: // never deletes these graphlets mannually
		WarGrey::SCADA::Statusbarlet* statusbar;
		WarGrey::SCADA::Statuslinelet* statusline;

	private:
		WarGrey::SCADA::GridDecorator* grid;
	};
}