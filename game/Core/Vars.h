#pragma once

#include "Dict/Dict.h"
#include "Types/Event.h"
#include "Value/Values.h"

class IAppService;
enum class DebugCameraType;

class Vars
{
public:
	Vars(IAppService* appService);
	~Vars();

	ff::Event<const Vars*> VarsReset;
	ff::Event<const Vars*, ff::StringRef> VarChanged;
	ff::Event<const Vars*> VarsDestroyed;

	void Reset();
	void Load(const ff::Dict& dict);
	ff::Dict Save() const;
	ff::ValuePtr Get(ff::StringRef name) const;
	ff::ValuePtr Set(ff::StringRef name, const ff::Value* value);
	void SetDidCheat();
	IAppService* GetAppService() const;

	// Actions
	ff::hash_t GetActionRestartGame() const;
	ff::hash_t GetActionRestartLevel() const;

	// Camera
	ff::FixedInt GetCameraAcceleration() const;
	const ff::RectFixedInt& GetCameraVelocityLimits() const;
	ff::FixedInt GetCameraVelocityThreshold() const;
	ff::FixedInt GetCameraVelocityThresholdDivisor() const;
	ff::FixedInt GetCameraViewFocusOffsetX() const;
	ff::FixedInt GetCameraViewFocusWidth() const;

	// Debug
	bool GetDebugDidCheat() const;
	bool GetDebugKeysEnabled() const;
	bool GetDebugShowLevelGrid() const;
	bool GetDebugShowBoundingBox() const;
	bool GetDebugShowHitBox() const;
	bool GetDebugShowPosition() const;
	bool GetDebugShowVelocity() const;
	bool GetDebugShowCameraFocus() const;
	bool GetDebugShowCollisions() const;

	DebugCameraType GetDebugCameraType() const;
	void ToggleDebugCameraType();

	// Level
	const ff::PointFixedInt& GetLevelGravity() const;

private:
	struct VarInfo
	{
		std::function<ff::ValuePtr(ff::StringRef)> _getter;
		std::function<ff::ValuePtr(const ff::Value*)> _setter;
	};

	template<typename T>
	void RegisterVar(ff::StringRef name, ff::ValuePtrT<T>& value)
	{
		VarInfo info =
		{
			[&value](ff::StringRef)
			{
				return value;
			},
			[&value](const ff::Value* newValue) -> ff::ValuePtr
			{
				ff::ValuePtrT<T> newValueT = newValue;
				if (!newValueT)
				{
					newValueT = ff::Value::NewDefaultT<T>();
				}

				value = newValueT;
				return newValueT;
			}
		};

		if (value.GetType() == typeid(ff::HashValue))
		{
			// Can't set event names
			info._setter = nullptr;
		}

		assert(!_vars.KeyExists(name));
		_vars.SetKey(ff::String(name), std::move(info));
	}

	IAppService* _appService;
	ff::Map<ff::String, VarInfo> _vars;

	// Actions
	ff::ValuePtrT<ff::HashValue> _actionRestartGame;
	ff::ValuePtrT<ff::HashValue> _actionRestartLevel;

	// Camera
	ff::ValuePtrT<ff::FixedIntValue> _cameraAcceleration;
	ff::ValuePtrT<ff::RectFixedIntValue> _cameraVelocityLimits;
	ff::ValuePtrT<ff::FixedIntValue> _cameraVelocityThreshold;
	ff::ValuePtrT<ff::FixedIntValue> _cameraVelocityThresholdDivisor;
	ff::ValuePtrT<ff::FixedIntValue> _cameraViewFocusOffsetX;
	ff::ValuePtrT<ff::FixedIntValue> _cameraViewFocusWidth;

	// Debug
	ff::ValuePtrT<ff::BoolValue> _debugDidCheat;
	ff::ValuePtrT<ff::BoolValue> _debugKeysEnabled;
	ff::ValuePtrT<ff::BoolValue> _debugShowLevelGrid;
	ff::ValuePtrT<ff::BoolValue> _debugShowBoundingBox;
	ff::ValuePtrT<ff::BoolValue> _debugShowHitBox;
	ff::ValuePtrT<ff::BoolValue> _debugShowPosition;
	ff::ValuePtrT<ff::BoolValue> _debugShowVelocity;
	ff::ValuePtrT<ff::BoolValue> _debugShowCameraFocus;
	ff::ValuePtrT<ff::BoolValue> _debugShowCollisions;
	ff::ValuePtrT<ff::IntValue> _debugCameraType;

	// Level
	ff::ValuePtrT<ff::PointFixedIntValue> _levelGravity;
};
