#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

#include <typeinfo>

#include "ofxImGui.h"

class AbstractProperty
{
public:

	AbstractProperty(std::string& InName) : Name(InName) {}

	inline std::string& GetName() { return Name; }
	
	virtual inline void RenderUI() = 0;
	virtual inline const std::type_info& GetType() = 0;

	std::string Name;
};

//TODO: Numeric Range : Templated Helper Struct? Empty for most types, but float / int gets two values?
//TODO: Add Property With Range

template <typename T>
class Property : public AbstractProperty
{
public:

	Property(std::string& InName, T* InValue) : AbstractProperty(InName), Value(InValue) {}

	//TODO: Break these out into free template functions (w/ specialization), default does nothing
	virtual inline void RenderUI() override
	{
		//Implementations in Template Specializations

		//TODO: Vec4
		//TODO: arrays,maps of properties
	}

	virtual inline const std::type_info& GetType() override { return typeid(*Value); }

	inline T& Get() { return *Value; }
	inline void Set(const T& NewValue) { *Value = NewValue; }

	T* Value = nullptr;
};

template<>
inline void Property<float>::RenderUI()
{
	ImGui::SliderFloat(GetName().c_str(), Value, 0.0f, 1.0f);
}

template<>
inline void Property<int>::RenderUI()
{
	ImGui::SliderInt(GetName().c_str(), Value, 0, 100);
}

template<>
inline void Property<bool>::RenderUI()
{
	ImGui::Checkbox(GetName().c_str(), Value);
}

template<>
inline void Property<std::string>::RenderUI()
{
	std::vector<char> CharVec(Value->c_str(), Value->c_str() + Value->size() + 1);

	//TODO: user-set string size limit
	CharVec.resize(256, '\0');

	ImGui::InputText(GetName().c_str(), CharVec.data(), CharVec.size());

	*Value = CharVec.data();
}

template<>
inline void Property<ofVec3f>::RenderUI()
{
	ImGui::DragFloat3(GetName().c_str(), Value->getPtr());
}

template<>
inline void Property<ofFloatColor>::RenderUI()
{
	ImGui::ColorEdit4(GetName().c_str(), Value->v);
}

class AggregateProperty : public Property<AggregateProperty>
{
public:

	inline AggregateProperty(std::string& InName) : Property(InName, this) {}

	virtual inline void RenderUI() override
	{	
		if (ImGui::TreeNode(GetName().c_str()))
		{
			RenderChildren();
			ImGui::TreePop();
		}
	}

	virtual inline void RenderChildren()
	{
		for (auto& Property : Properties)
		{
			Property->RenderUI();
		}
	}

	template<typename T>
	inline void AddProperty(std::string& Name, T* Data)
	{
		AbstractProperty* NewProperty = new Property<T>(Name, Data);
		Properties.push_back(std::shared_ptr<AbstractProperty>(NewProperty));
	}

	inline std::vector<std::shared_ptr<AbstractProperty>>& GetProperties() { return Properties; }

	std::vector<std::shared_ptr<AbstractProperty>> Properties;

};

template<>
inline void Property<AggregateProperty>::RenderUI()
{
	Value->RenderUI();
}

/** ImGui Property Editor Window
*   Renders a currently selected property into the properties window
*   The selected TopLevel aggregate property will not have a tree node, but inner aggregate properties will
*/
struct PropertyWindow
{
	AggregateProperty* Selected = nullptr;
	bool bOpen = true;

	void Render()
	{
		if (bOpen)
		{
			std::string Title("Properties: ");

			if (Selected != nullptr)
			{
				Title += Selected->Name;
			}

			ImGui::Begin(Title.c_str());

			if (Selected != nullptr)
			{
				Selected->RenderChildren();
			}

			ImGui::End();
		}
	}
};

