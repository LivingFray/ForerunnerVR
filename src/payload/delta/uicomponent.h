#pragma once

// Handles positioning of UI elements, actual rendering is done in the render component
class UIComponent
{
public:

	void UpdateHUD();

	float UIDistance = 2.0f;
};