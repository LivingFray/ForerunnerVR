#include "config_editor.h"
#include "common/config/config.h"
#include <imgui.h>
#include <string>

constexpr float ValueStart = 0.67f;

template<typename T>
static std::string ToString(const T& V)
{
	std::ostringstream ss;
	ss << V;
	return ss.str();
}

template<>
static std::string ToString(const bool& V)
{
	return V ? "true" : "false";
}

template<typename T>
static void AddButton(T& Value, const char* Name)
{
	ImGui::TextColored(ImVec4(1.0f, 0, 0, 1.0f), "%s: %s type not supported", Name, typeid(T).name());
}

static void AddDescription(const std::string& Desc, const std::string& DefaultValue)
{
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
	ImGui::TextWrapped(Desc.c_str());
	ImGui::PopStyleColor();
	ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Default Value: %s", DefaultValue.c_str());
}

static void AddButtonLabel(const char* Name)
{
	ImGui::SetWindowFontScale(1.25f);
	ImGui::AlignTextToFramePadding();
	ImGui::Text(Name);
	ImGui::SetWindowFontScale(1.0f);
	ImGui::SameLine(ImGui::GetWindowWidth() * ValueStart);
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * (1.0f - ValueStart) - 10.0f);
}

template<>
static void AddButton<bool>(bool& Value, const char* Name)
{
	ImGui::SetWindowFontScale(1.25f);
	ImGui::AlignTextToFramePadding();
	ImGui::Text(Name);
	ImGui::SetWindowFontScale(1.0f);
	ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::GetFrameHeight() - 10.0f);
	ImGui::PushID(Name);
	ImGui::Checkbox("##checkbox", &Value);
	ImGui::PopID();
}

template<>
static void AddButton<float>(float& Value, const char* Name)
{
	AddButtonLabel(Name);
	ImGui::PushID(Name);
	ImGui::InputFloat("##inputfloat", &Value);
	ImGui::PopID();
}

template <typename T>
concept IsSerialisedEnum = requires { T::E_MAX; };

template<IsSerialisedEnum T>
static void AddButton(T& Value, const char* Name)
{
	AddButtonLabel(Name);
	ImGui::PushID(Name);
	if (ImGui::BeginCombo("##enum", ToString(Value).c_str()))
	{
		for (int i = 0; i < static_cast<int>(T::E_MAX); i++)
		{
			T Option = static_cast<T>(i);
			bool bIsThisItemSelected = (Value == Option);
			if (ImGui::Selectable(ToString(Option).c_str(), bIsThisItemSelected))
			{
				Value = Option;
			}

			if (bIsThisItemSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::PopID();
}

template<IsSerialisedEnum T>
static std::string ToString(const T& V)
{
	nlohmann::json Temp;
	to_json(Temp, V);

	return Temp;
}


void DrawConfig()
{
	using namespace Config;
#define CFG_CAT(Name) ImGui::SetNextItemOpen(true, ImGuiCond_Once); if (ImGui::TreeNode(#Name)) { using namespace Name;
#define CFG_CAT_END(Name) ImGui::TreePop(); }
#define CFG_VALUE(Type, Name, DefaultValue, Description) AddButton<Type>(Name, #Name); AddDescription(Description, ToString(DefaultValue));
FORERUNNER_CONFIGS
#undef CFG_CAT
#undef CFG_CAT_END
#undef CFG_VALUE
}
