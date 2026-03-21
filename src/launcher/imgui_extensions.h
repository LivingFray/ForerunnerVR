#pragma once

#include "imgui.h"
#include <vector>
#include <string>
#include <cmath>

struct ImVerticalTabBarState
{
	int SelectedIndex = 0;
	int PreviousIndex = -1; // Used to know where the indicator is coming from

	// Animation states (0.0 to 1.0)
	float IndicatorAnimProgress = 1.0f; // 1.0 means "animation finished"
	float IndicatorCurrentY = 0.0f;     // The interpolated Y position
	float IndicatorCurrentHeight = 0.0f;// The interpolated Height

	// Hover states (we need one per tab)
	std::vector<float> HoverTimers;

	// Active fade states (for when a previously-selected tab fades out)
	std::vector<float> ActiveFadeTimers;

	// Settings
	float AnimationSpeed = 12.0f;   // Higher = snappier
	float HoverFadeSpeed = 8.0f;
	ImVec4 ColorActiveLeft = ImVec4(0.2f, 0.2f, 0.25f, 0.8f);   // The "accent" color
	ImVec4 ColorActiveRight = ImVec4(0.2f, 0.2f, 0.25f, 0.5f);
	ImVec4 ColorHover = ImVec4(0.2f, 0.2f, 0.25f, 0.5f);    // Background hover color
	ImVec4 ColorText = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);     // Text color
	ImVec4 ColorTextInactive = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
	ImVec4 ColorIndicator = ImVec4(0.3f, 0.7f, 0.9f, 1.0f);// The sliding line color
	ImVec2 ScaleFadeOut = ImVec2(1.25f, 1.25f);
	float ActiveFadeSpeed = 6.0f;
	float IndicatorWidth = 3.0f;
	float IndicatorGlowWidth = 4.0f;

	void Initialize(int count);
};

// Returns true if a new tab was clicked
bool RenderVerticalTabBar(const char* str_id, const std::vector<std::string>& tab_names, ImVerticalTabBarState& state, ImVec2 size = ImVec2(150, 0));