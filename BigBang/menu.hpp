#pragma once

#include "object.hpp"
#include "snip/snip.hpp"

namespace WarGrey::SCADA {
	WarGrey::SCADA::ISnip* menu_current_target_snip();

	void menu_append(
		Windows::UI::Xaml::Controls::MenuFlyout^ menu,
		Platform::String^ label,
		Windows::UI::Xaml::Input::ICommand^ cmd);

	void menu_show(
		Windows::UI::Xaml::Controls::MenuFlyout^ menu,
		WarGrey::SCADA::ISnip* snip,
		float local_x, float local_y,
		float xoff, float yoff);

	template <typename Menu>
	private class IMenuCommand abstract {
	public:
		virtual void execute(Menu cmd, WarGrey::SCADA::ISnip* snip) = 0;
	};

	template <typename Menu>
	private ref class MenuCommand sealed : public Windows::UI::Xaml::Input::ICommand {
		/** NOTE	
		 * Interface linguistically is not a class
		 * all the required methods therefore should be marked as `virtual` instead of `override`.
		 */
	internal:
		MenuCommand(IMenuCommand<Menu>* exe, Menu cmd) : executor(exe), command(cmd) {}
		
	public:
		virtual bool CanExecute(Platform::Object^ who_cares) {
			return true;
		}

		virtual void Execute(Platform::Object^ who_cares) {
			this->executor->execute(this->command, menu_current_target_snip());
		}

	public:
		event Windows::Foundation::EventHandler<Platform::Object^>^ CanExecuteChanged {
			// this event is useless but to satisfy the C++/CX compiler
			virtual Windows::Foundation::EventRegistrationToken add(Windows::Foundation::EventHandler<Platform::Object^>^ handler) {
				return Windows::Foundation::EventRegistrationToken{ 0L };
			}

			virtual void remove(Windows::Foundation::EventRegistrationToken token) {}
		}

	private:
		WarGrey::SCADA::IMenuCommand<Menu>* executor;
		Menu command;
	};

	template <typename Menu>
	private class CommandMenu : public WarGrey::SCADA::SharedObject {
	public:
		CommandMenu() { this->menu = ref new Windows::UI::Xaml::Controls::MenuFlyout(); }

	public:
		void append(Menu cmd, WarGrey::SCADA::IMenuCommand<Menu>* exe) {
			menu_append(this->menu, cmd.ToString(), ref new WarGrey::SCADA::MenuCommand<Menu>(exe, cmd));
		}

		void show_for(WarGrey::SCADA::ISnip* snip, float local_x, float local_y, float xoff = 2.0F, float yoff = 2.0F) {
			menu_show(this->menu, snip, local_x, local_y, xoff, yoff);
		}

	private:
		Windows::UI::Xaml::Controls::MenuFlyout^ menu;
	};
}
