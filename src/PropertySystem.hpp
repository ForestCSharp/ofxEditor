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

template <typename T>
class Property : public AbstractProperty
{
public:

	Property(std::string& InName, T* InValue) : AbstractProperty(InName), Value(InValue) {}

	virtual inline void RenderUI() override
	{ 
		const std::type_info& TypeID = typeid(*Value);

		if (TypeID == typeid(bool))
		{
			ImGui::Checkbox(GetName().c_str(), (bool*)Value);
		}
		else if (TypeID == typeid(int))
		{
			//TODO: Range
			ImGui::SliderInt(GetName().c_str(), (int*)Value, 0, 100);
		}
		else if (TypeID == typeid(float))
		{
			//TODO: Range
			ImGui::SliderFloat(GetName().c_str(), (float*)Value, 0.0f, 1.0f);
		}
		else if (TypeID == typeid(std::string))
		{
			//Copy/Paste broken

			std::string* StringPtr = (std::string*)Value;
			std::vector<char> CharVec(StringPtr->c_str(), StringPtr->c_str() + StringPtr->size() + 1);
			
			//TODO: user-set string size limit
			CharVec.resize(256, '\0');

			ImGui::InputText(GetName().c_str(), CharVec.data(), CharVec.size());

			*StringPtr = CharVec.data();
		}
		else if (TypeID == typeid(ofVec3f))
		{
			//TODO: Vector3
			ImGui::DragFloat3(GetName().c_str(), ((ofVec3f*)Value)->getPtr());
		}
		else if (TypeID == typeid(ofFloatColor))
		{
			ofFloatColor* FloatColorPtr = (ofFloatColor*)Value;
			ImGui::ColorEdit4(GetName().c_str(), FloatColorPtr->v);
		}
		//TODO: Vec4
		//TODO: arrays,maps of properties
	}

	virtual inline const std::type_info& GetType() override { return typeid(*Value); }

	inline T& Get() { return *Value; }
	inline void Set(const T& NewValue) { *Value = NewValue; }

	T* Value = nullptr;
};

class AggregateProperty : public Property<AggregateProperty>
{
public:

	inline AggregateProperty(std::string& InName) : Property(InName, this) {}

	virtual inline void RenderUI() override
	{	
		if (ImGui::TreeNode(GetName().c_str()))
		{
			for (auto& Property : Properties)
			{
				Property->RenderUI();
			}
			ImGui::TreePop();
		}
	}

	template<typename T>
	inline void AddProperty(std::string& Name, T* Data)
	{
		AbstractProperty* NewProperty = new Property<T>(Name, Data);
		//std::cout << typeid(Data).hash_code() << std::endl;
		Properties.push_back(std::shared_ptr<AbstractProperty>(NewProperty));
	}

	inline std::vector<std::shared_ptr<AbstractProperty>>& GetProperties() { return Properties; }

	std::vector<std::shared_ptr<AbstractProperty>> Properties;

};
