#pragma once

#include "P2LibConfig.hpp"

#include <ImGui.h>

namespace P2
{
	void PushSubWindowTitleFont(const auto& style = ImGui::GetStyle())
	{
		ImGui::PushFont(nullptr, style.FontSizeBase * 2.f);
	}

	void PopSubWindowTitleFont()
	{
		ImGui::PopFont();
	}

	void SubWindowTitle(const char* text)
	{
		const auto& style = ImGui::GetStyle();
		PushSubWindowTitleFont(style);
		ImGui::Text(text);
		PopSubWindowTitleFont();
		ImGui::Separator();
	}
}
