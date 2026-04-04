#include "config_editor.h"
#include "common/config/config.h"
#include <imgui.h>
#include <string>

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
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Default Value: %s", DefaultValue.c_str());
	ImGui::Indent();
	ImGui::Indent();
	ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), Desc.c_str());
	ImGui::Unindent();
	ImGui::Unindent();
}

template<>
static void AddButton<bool>(bool& Value, const char* Name)
{
	ImGui::Checkbox(Name, &Value);
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
