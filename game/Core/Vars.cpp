#include "pch.h"
#include "Components/CameraComponent.h"
#include "Core/Globals.h"
#include "Core/Vars.h"
#include "Globals/AppGlobals.h"
#include "Services/AppService.h"

namespace Names
{
	static ff::StaticString ActionRestartGame(L"Actions.Restart Game");
	static ff::StaticString ActionRestartLevel(L"Actions.Restart Level");
	static ff::StaticString CameraAcceleration(L"Camera.Acceleration");
	static ff::StaticString CameraVelocityLimits(L"Camera.Velocity Limits");
	static ff::StaticString CameraVelocityThreshold(L"Camera.Velocity Threshold");
	static ff::StaticString CameraVelocityThresholdDivisor(L"Camera.Velocity Threshold Divisor");
	static ff::StaticString CameraViewFocusOffsetX(L"Camera.View Focus Offset X");
	static ff::StaticString CameraViewFocusWidth(L"Camera.View Focus Width");
	static ff::StaticString DebugCameraType(L"Debug.Camera Type");
	static ff::StaticString DebugDidCheat(L"Debug.Did Cheat");
	static ff::StaticString DebugKeysEnabled(L"Debug.Keys Enabled");
	static ff::StaticString DebugShowBoundingBox(L"Debug.Show Bounding Box");
	static ff::StaticString DebugShowCameraFocus(L"Debug.Show Camera Focus");
	static ff::StaticString DebugShowCollisions(L"Debug.Show Collisions");
	static ff::StaticString DebugShowHitBox(L"Debug.Show Hit Box");
	static ff::StaticString DebugShowLevelGrid(L"Debug.Show Level Grid");
	static ff::StaticString DebugShowPosition(L"Debug.Show Position");
	static ff::StaticString DebugShowVelocity(L"Debug.Show Velocity");
	static ff::StaticString LevelGravity(L"Level.Gravity");
}

Vars::Vars(IAppService* appService)
	: _appService(appService)
{
	RegisterVar(Names::ActionRestartGame, _actionRestartGame);
	RegisterVar(Names::ActionRestartLevel, _actionRestartLevel);
	RegisterVar(Names::CameraAcceleration, _cameraAcceleration);
	RegisterVar(Names::CameraVelocityLimits, _cameraVelocityLimits);
	RegisterVar(Names::CameraVelocityThreshold, _cameraVelocityThreshold);
	RegisterVar(Names::CameraVelocityThresholdDivisor, _cameraVelocityThresholdDivisor);
	RegisterVar(Names::CameraViewFocusOffsetX, _cameraViewFocusOffsetX);
	RegisterVar(Names::CameraViewFocusWidth, _cameraViewFocusWidth);
	RegisterVar(Names::DebugCameraType, _debugCameraType);
	RegisterVar(Names::DebugDidCheat, _debugDidCheat);
	RegisterVar(Names::DebugKeysEnabled, _debugKeysEnabled);
	RegisterVar(Names::DebugShowBoundingBox, _debugShowBoundingBox);
	RegisterVar(Names::DebugShowCameraFocus, _debugShowCameraFocus);
	RegisterVar(Names::DebugShowCollisions, _debugShowCollisions);
	RegisterVar(Names::DebugShowHitBox, _debugShowHitBox);
	RegisterVar(Names::DebugShowLevelGrid, _debugShowLevelGrid);
	RegisterVar(Names::DebugShowPosition, _debugShowPosition);
	RegisterVar(Names::DebugShowVelocity, _debugShowVelocity);
	RegisterVar(Names::LevelGravity, _levelGravity);

	_actionRestartGame = ff::Value::New<ff::HashValue>(GameEvents::ID_DEBUG_RESTART_GAME);
	_actionRestartLevel = ff::Value::New<ff::HashValue>(GameEvents::ID_DEBUG_RESTART_LEVEL);

	Reset();
}

Vars::~Vars()
{
	VarsDestroyed.Notify(this);
}

void Vars::Reset()
{
	Load(ff::Dict());
}

void Vars::Load(const ff::Dict& dict)
{
	ff::Dict defaultDict;
	if (_appService)
	{
		defaultDict = _appService->GetDefaultVars();
	}

	for (const ff::KeyValue<ff::String, VarInfo>& i : _vars)
	{
		ff::StringRef name = i.GetKey();
		const VarInfo& info = i.GetValue();

		if (info._setter != nullptr)
		{
			ff::ValuePtr value = dict.GetValue(name);
			if (value || name != Names::DebugDidCheat.GetString())
			{
				if (!value)
				{
					value = defaultDict.GetValue(name);
				}

				info._setter(value);
			}
		}
	}

	VarsReset.Notify(this);
}

ff::Dict Vars::Save() const
{
	ff::Dict dict;

	for (const ff::KeyValue<ff::String, VarInfo>& i : _vars)
	{
		ff::StringRef name = i.GetKey();
		const VarInfo& info = i.GetValue();

		if (info._getter != nullptr)
		{
			dict.SetValue(name, info._getter(name));
		}
	}

	return dict;
}

ff::ValuePtr Vars::Get(ff::StringRef name) const
{
	auto i = _vars.GetKey(name);
	assertRetVal(i, nullptr);

	const VarInfo& info = i->GetValue();
	assertRetVal(info._getter != nullptr, nullptr);

	return info._getter(name);
}

ff::ValuePtr Vars::Set(ff::StringRef name, const ff::Value* value)
{
	auto i = _vars.GetKey(name);
	assertRetVal(i, nullptr);

	const VarInfo& info = i->GetValue();
	assertRetVal(info._setter != nullptr, nullptr);
	ff::ValuePtr newValue = info._setter(value);

	VarChanged.Notify(this, name);

	if (name != Names::DebugDidCheat.GetString())
	{
		SetDidCheat();
	}

	return newValue;
}

void Vars::SetDidCheat()
{
	if (!_debugDidCheat.GetValue())
	{
		Set(Names::DebugDidCheat, ff::Value::New<ff::BoolValue>(true));
	}
}

IAppService* Vars::GetAppService() const
{
	return _appService;
}

ff::hash_t Vars::GetActionRestartGame() const
{
	return _actionRestartGame.GetValue();
}

ff::hash_t Vars::GetActionRestartLevel() const
{
	return _actionRestartLevel.GetValue();
}

ff::FixedInt Vars::GetCameraAcceleration() const
{
	return _cameraAcceleration.GetValue();
}

const ff::RectFixedInt& Vars::GetCameraVelocityLimits() const
{
	return _cameraVelocityLimits.GetValue();
}

ff::FixedInt Vars::GetCameraVelocityThreshold() const
{
	return _cameraVelocityThreshold.GetValue();
}

ff::FixedInt Vars::GetCameraVelocityThresholdDivisor() const
{
	return _cameraVelocityThresholdDivisor.GetValue();
}

ff::FixedInt Vars::GetCameraViewFocusOffsetX() const
{
	return _cameraViewFocusOffsetX.GetValue();
}

ff::FixedInt Vars::GetCameraViewFocusWidth() const
{
	return _cameraViewFocusWidth.GetValue();
}

bool Vars::GetDebugDidCheat() const
{
	return _debugDidCheat.GetValue();
}

bool Vars::GetDebugKeysEnabled() const
{
	return _debugKeysEnabled.GetValue();
}

bool Vars::GetDebugShowLevelGrid() const
{
	return _debugShowLevelGrid.GetValue();
}

bool Vars::GetDebugShowBoundingBox() const
{
	return _debugShowBoundingBox.GetValue();
}

bool Vars::GetDebugShowHitBox() const
{
	return _debugShowHitBox.GetValue();
}

bool Vars::GetDebugShowPosition() const
{
	return _debugShowPosition.GetValue();
}

bool Vars::GetDebugShowVelocity() const
{
	return _debugShowVelocity.GetValue();
}

bool Vars::GetDebugShowCameraFocus() const
{
	return _debugShowCameraFocus.GetValue();
}

bool Vars::GetDebugShowCollisions() const
{
	return _debugShowCollisions.GetValue();
}

DebugCameraType Vars::GetDebugCameraType() const
{
	return (DebugCameraType)_debugCameraType.GetValue();
}

void Vars::ToggleDebugCameraType()
{
	int newValue = (_debugCameraType.GetValue() + 1) % (int)DebugCameraType::Count;
	Set(Names::DebugCameraType, ff::Value::New<ff::IntValue>(newValue));
}

const ff::PointFixedInt& Vars::GetLevelGravity() const
{
	return _levelGravity.GetValue();
}
