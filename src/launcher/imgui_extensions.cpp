#include "imgui_extensions.h"
#include "imgui.h"
#include "imgui_internal.h"

void ImVerticalTabBarState::Initialize(int count)
{
	if (HoverTimers.size() != count)
	{
		HoverTimers.clear();
		HoverTimers.resize(count, 0.0f);
	}
	if (ActiveFadeTimers.size() != count)
	{
		ActiveFadeTimers.clear();
		ActiveFadeTimers.resize(count, 0.0f);
	}
}

bool RenderVerticalTabBar(const char* str_id, const std::vector<std::string>& tab_names, ImVerticalTabBarState& state, ImVec2 size)
{
	ImGui::PushID(str_id);
	state.Initialize((int)tab_names.size());

	// Determine layout
	ImVec2 contentRegion = ImGui::GetContentRegionAvail();
	float width = size.x > 0 ? size.x : contentRegion.x;
	float itemHeight = 40.0f; // Fixed height per tab

	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	bool valueChanged = false;
	ImVec2 startPos = ImGui::GetCursorScreenPos();

	// We use an invisible button for the click/hover logic 
	// and then draw on top of it manually

	float max_y = startPos.y; // To track total height for the indicator clip

	for (int i = 0; i < tab_names.size(); i++)
	{
		const char* label = tab_names[i].c_str();

		// --- Interaction Logic ---
		ImGui::PushID(i);

		// Update Hover Timer
		bool isHovered = ImGui::IsMouseHoveringRect(
			ImVec2(startPos.x, startPos.y + i * itemHeight),
			ImVec2(startPos.x + width, startPos.y + (i + 1) * itemHeight)
		);

		// Smooth interpolation for hover (0.0 to 1.0)
		float hoverTarget = isHovered ? 1.0f : 0.0f;
		state.HoverTimers[i] = ImLerp(state.HoverTimers[i], hoverTarget, ImGui::GetIO().DeltaTime * state.HoverFadeSpeed);

		// Click detection
		if (isHovered && ImGui::IsMouseClicked(0))
		{
			if (state.SelectedIndex != i)
			{
				state.PreviousIndex = state.SelectedIndex;
				// Start active fade for the previously selected tab
				if (state.PreviousIndex >= 0 && state.PreviousIndex < (int)state.ActiveFadeTimers.size())
				{
					state.ActiveFadeTimers[state.PreviousIndex] = 1.0f; // start at full and fade to 0
				}
				state.SelectedIndex = i;
				state.IndicatorAnimProgress = 0.0f; // Start indicator animation
				valueChanged = true;
			}
		}

		ImGui::PopID();
	}

	// --- Rendering ---
	// We iterate again to draw, but we draw everything manually using the DrawList.
	// This ensures we can layer things (Background -> Indicator -> Text)

	float totalHeight = itemHeight * tab_names.size();

	// 1. Draw Backgrounds & Text
	for (int i = 0; i < tab_names.size(); i++)
	{
		ImVec2 p_min = ImVec2(startPos.x, startPos.y + i * itemHeight);
		ImVec2 p_max = ImVec2(startPos.x + width, startPos.y + (i + 1) * itemHeight);

		// Dynamic Hover Background Color
		// Base hover background (fades in based on hover timer)
		ImVec4 hoverBg = ImLerp(ImVec4(0, 0, 0, 0), state.ColorHover, state.HoverTimers[i]);

		// If this tab is active, blend in the active color. Use the active color's alpha
		// as the blend factor so callers can control how strongly the active color
		// shows through.
		ImVec4 finalBgLeft = hoverBg;
		ImVec4 finalBgRight = hoverBg;
		if (i == state.SelectedIndex)
		{
			finalBgLeft = ImLerp(hoverBg, state.ColorActiveLeft, state.ColorActiveLeft.w);
			finalBgRight = ImLerp(hoverBg, state.ColorActiveRight, state.ColorActiveRight.w);
		}

		// Update any active fade timer early so scale/color below uses current value
		float hoverTimer = state.HoverTimers[i];
		float fadeTimer = 0.0f;
		if (i >= 0 && i < (int)state.ActiveFadeTimers.size())
		{
			fadeTimer = state.ActiveFadeTimers[i];
			if (fadeTimer > 0.0f)
			{
				fadeTimer = ImLerp(fadeTimer, 0.0f, ImGui::GetIO().DeltaTime * state.ActiveFadeSpeed);
				if (fadeTimer < 0.001f) fadeTimer = 0.0f;
				state.ActiveFadeTimers[i] = fadeTimer;
			}
		}

		// Compute scales from either hover or active-fade (both interpolate from ScaleFadeOut -> 1.0)
		// Note: selected tabs are not scaled by hover effects - they stay at 1.0 scale
		float finalScaleX = 1.0f;
		float finalScaleY = 1.0f;
		
		if (fadeTimer > 0.0f)
		{
			// Tab is fading out: scale from 1.0 -> ScaleFadeOut
			finalScaleX = state.ScaleFadeOut.x + (1.0f - state.ScaleFadeOut.x) * fadeTimer;
			finalScaleY = state.ScaleFadeOut.y + (1.0f - state.ScaleFadeOut.y) * fadeTimer;
		}
		else if (hoverTimer > 0.0f && i != state.SelectedIndex)
		{
			// Tab is being hovered (and not selected): scale from ScaleFadeOut -> 1.0
			finalScaleX = state.ScaleFadeOut.x + (1.0f - state.ScaleFadeOut.x) * hoverTimer;
			finalScaleY = state.ScaleFadeOut.y + (1.0f - state.ScaleFadeOut.y) * hoverTimer;
		}

		// Build base colors: start from hoverBg then blend in active colors for selected/current-fade
		ImVec4 baseLeft = hoverBg;
		ImVec4 baseRight = hoverBg;
		// If currently selected, blend in active color at full presence (using its alpha)
		if (i == state.SelectedIndex)
		{
			baseLeft = ImLerp(baseLeft, state.ColorActiveLeft, state.ColorActiveLeft.w);
			baseRight = ImLerp(baseRight, state.ColorActiveRight, state.ColorActiveRight.w);
		}
		// If fading from previous selection, blend in active color proportionally to fadeTimer*alpha
		if (fadeTimer > 0.0f)
		{
			float inv = fadeTimer; // 1 -> just started
			float fadeFactorLeft = state.ColorActiveLeft.w * inv;
			float fadeFactorRight = state.ColorActiveRight.w * inv;
			baseLeft = ImLerp(baseLeft, state.ColorActiveLeft, fadeFactorLeft);
			baseRight = ImLerp(baseRight, state.ColorActiveRight, fadeFactorRight);
		}

		// Compute scaled rect centered on the original using finalScaleX/Y
		ImVec2 center = ImVec2((p_min.x + p_max.x) * 0.5f, (p_min.y + p_max.y) * 0.5f);
		ImVec2 half = ImVec2((p_max.x - p_min.x) * 0.5f * finalScaleX, (p_max.y - p_min.y) * 0.5f * finalScaleY);
		ImVec2 sMin = ImVec2(center.x - half.x, center.y - half.y);
		ImVec2 sMax = ImVec2(center.x + half.x, center.y + half.y);

		ImU32 finalColLeft = ImGui::ColorConvertFloat4ToU32(baseLeft);
		ImU32 finalColRight = ImGui::ColorConvertFloat4ToU32(baseRight);
		draw_list->AddRectFilledMultiColor(sMin, sMax, finalColLeft, finalColRight, finalColRight, finalColLeft);

		// Draw Text
		ImGui::SetCursorScreenPos(ImVec2(p_min.x + 10, p_min.y + (itemHeight - ImGui::GetTextLineHeight()) / 2.0f));

		// Choose text color
		ImVec4 textColor = (i == state.SelectedIndex) ? state.ColorText : state.ColorTextInactive;
		ImGui::TextColored(textColor, "%s", tab_names[i].c_str());
	}

	// 2. Draw Sliding Indicator
	// Animate the position/height transition
	if (state.IndicatorAnimProgress < 1.0f)
	{
		state.IndicatorAnimProgress += ImGui::GetIO().DeltaTime * state.AnimationSpeed;
		if (state.IndicatorAnimProgress > 1.0f) state.IndicatorAnimProgress = 1.0f;
	}

	// Calculate start/end positions for the animation
	float targetY = startPos.y + state.SelectedIndex * itemHeight;
	float prevY = (state.PreviousIndex != -1) ? startPos.y + state.PreviousIndex * itemHeight : targetY;

	// Smooth Step (Ease out)
	float t = state.IndicatorAnimProgress;
	float smoothT = t * t * (3.0f - 2.0f * t); // Smoothstep function

	// Interpolate Position
	state.IndicatorCurrentY = ImLerp(prevY, targetY, smoothT);
	state.IndicatorCurrentHeight = itemHeight;

	// Draw the vertical line indicator with a simple glow approximation.
	ImU32 BaseColour = ImGui::ColorConvertFloat4ToU32(state.ColorIndicator);
	ImU32 CoreColour = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImVec2 coreMin = ImVec2(startPos.x, state.IndicatorCurrentY);
	ImVec2 coreMax = ImVec2(startPos.x + state.IndicatorWidth, state.IndicatorCurrentY + state.IndicatorCurrentHeight);
	draw_list->AddRectFilled(coreMin, coreMax, CoreColour, 0.0f, 0);

	// Glow layers (expand horizontally, reduce alpha)
	float expand = state.IndicatorGlowWidth - state.IndicatorWidth;
	float glowAlpha = 0.0f;
	ImVec4 glowCol = ImVec4(state.ColorIndicator.x, state.ColorIndicator.y, state.ColorIndicator.z, glowAlpha);
	ImU32 GlowColour = ImGui::ColorConvertFloat4ToU32(glowCol);
	ImVec2 gMin = ImVec2(coreMin.x - expand, coreMin.y);
	ImVec2 gMax = ImVec2(coreMax.x + expand, coreMax.y);
	draw_list->AddRectFilledMultiColor(gMin, ImVec2(coreMin.x, coreMax.y), GlowColour, BaseColour, BaseColour, GlowColour);
	draw_list->AddRectFilledMultiColor(ImVec2(coreMax.x, coreMin.y), gMax, BaseColour, GlowColour, GlowColour, BaseColour);

	// --- Cleanup ---
	// Advance the cursor so the next UI element goes under the tabs
	ImGui::SetCursorScreenPos(ImVec2(startPos.x, startPos.y + totalHeight));
	ImGui::Dummy(ImVec2(0,0));

	ImGui::PopID();
	return valueChanged;
}