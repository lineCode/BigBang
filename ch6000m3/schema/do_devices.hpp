#pragma once

namespace WarGrey::SCADA {
	// DB300, starts from 1
	template<typename OP>
	uint16 DO_overflow_command(OP cmd) {
		uint16 index = 0U;

		switch (cmd) {
		case OP::Up:   index = 857U; break;
		case OP::Down: index = 858U; break;
		case OP::Stop: index = 859U; break;
		case OP::Auto: index = 868U; break;
		}

		return index;
	}

	template<typename OP>
	uint16 DO_tank_heater_command(OP cmd) {
		uint16 index = 0U;

		switch (cmd) {
		case OP::Start:  index = 665U; break;
		case OP::Stop:   index = 666U; break;
		case OP::Cancel: index = 667U; break;
		case OP::Auto:   index = 668U; break;
		}

		return index;
	}
}
