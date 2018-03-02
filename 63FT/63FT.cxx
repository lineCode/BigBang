﻿#include "workbench.hxx"
#include "syslog.hpp"
#include "system.hpp"

using namespace Windows::Foundation;

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;

using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::ViewManagement;

namespace WarGrey::SCADA {
    typedef EventHandler<UnhandledErrorDetectedEventArgs^> UncaughtExceptionHandler;

    private ref class Boat63FT sealed : public Application {
    protected:
        void RebuerMain(ApplicationView^ self, FrameworkElement^ screen) {
            this->Suspending += ref new SuspendingEventHandler(this, &Boat63FT::OnSuspending);
            CoreApplication::UnhandledErrorDetected += ref new UncaughtExceptionHandler(this, &Boat63FT::OnUncaughtException);

            ApplicationView::PreferredLaunchWindowingMode = ApplicationViewWindowingMode::PreferredLaunchViewSize;
            ApplicationView::PreferredLaunchViewSize = system_screen_size();
            this->RequestedTheme = ApplicationTheme::Dark;

            // WARNING: Force Using the default TitleBar if a custom one was set once.
            CoreApplication::GetCurrentView()->TitleBar->ExtendViewIntoTitleBar = false;
			self->Title = screen->ToString();
        }

        virtual void OnLaunched(LaunchActivatedEventArgs^ e) override {
            auto self = ApplicationView::GetForCurrentView();
            auto screen = dynamic_cast<Workbench^>(Window::Current->Content);

            if (screen == nullptr) {
                screen = ref new Workbench();
                this->RebuerMain(self, screen);
            
                if (e->PreviousExecutionState == ApplicationExecutionState::Terminated) {
                    // TODO: Restore the saved session state only when appropriate, scheduling the
                    // final launch steps after the restore is complete
                }

				Window::Current->Content = screen;
            }

			if (e->PrelaunchActivated == false) {
				screen->initialize_component(adjusted_workspace_size(self->VisibleBounds, screen));
				Window::Current->Activate();
			}
        }

    private:
        void OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ e) {
			// TODO: Save application state and stop any background activity.
            // Do not assume that the application will be terminated or resumed with the contents of memory still intact.
        }

        void OnUncaughtException(Platform::Object^ sender, UnhandledErrorDetectedEventArgs^ e) {
            auto error = e->UnhandledError;

            if (!error->Handled) {
                try {
                    // if an error is returned from a delegate, it will not be marked as Handled.
                    error->Propagate();
                } catch (Platform::Exception^ e) {
                    syslog(Log::Panic, "Unhandled Error: " + e->Message);
				}
            }
        }
    };
}

int main(Platform::Array<Platform::String^>^ args) {
    // Windows::Globalization::ApplicationLanguages::PrimaryLanguageOverride = "zh-cn";
	auto lazy_main = [](ApplicationInitializationCallbackParams^ p) { ref new WarGrey::SCADA::Boat63FT(); };
    Application::Start(ref new ApplicationInitializationCallback(lazy_main));
}