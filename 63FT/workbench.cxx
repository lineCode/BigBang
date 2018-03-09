﻿#include "workbench.hxx"
#include "configuration.hpp"

#include "page/hydraulic_pressure.hpp"
#include "testbed.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::Foundation;

using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Input;

/*************************************************************************************************/
private ref class Universe sealed : public WarGrey::SCADA::UniverseDisplay {
public:
	Universe(Platform::String^ name) : UniverseDisplay(16, make_system_logger(default_logging_level, name)) {}

protected:
	void construct() override {
		//this->add_planet(new HPCWorkbench("192.168.8.101"));
		this->add_planet(new Testbed());
	}
};

/*************************************************************************************************/
Workbench::Workbench() : SplitView() {
	this->Margin = ThicknessHelper::FromUniformLength(0.0);
	this->PanePlacement = SplitViewPanePlacement::Left;
	this->DisplayMode = SplitViewDisplayMode::Overlay;
	this->OpenPaneLength = 48;
	this->IsPaneOpen = false;

	this->PointerMoved += ref new PointerEventHandler(this, &Workbench::on_pointer_moved);
}

void Workbench::initialize_component(Size region) {
	this->universe = ref new Universe("Workbench@63FT");
	this->Content = this->universe->canvas;
	this->Pane = this->universe->navigator;

	// TODO: Why SplitView::Content cannot do it on its own?
	this->KeyDown += ref new KeyEventHandler(this->universe, &UniverseDisplay::on_char);
}

void Workbench::on_pointer_moved(Platform::Object^ sender, PointerRoutedEventArgs^ args) {
	auto pt = args->GetCurrentPoint(this);
	float x = pt->Position.X;

	if (!pt->Properties->IsLeftButtonPressed) {
		if (x <= this->Margin.Left) {
			this->IsPaneOpen = true;
			args->Handled = true;
		} else if (x > this->OpenPaneLength) {
			this->IsPaneOpen = false;
			args->Handled = true;
		}
	}
}
