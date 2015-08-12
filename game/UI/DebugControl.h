#pragma once

#include "Dict/Dict.h"
#include "Types/Event.h"
#include "UI/NotifyPropertyChangedBase.h"
#include "Value/Value.h"

class DebugControl;
class IAppService;
class Vars;

class VarHolderBase : public ff::NotifyPropertyChangedBase
{
public:
	VarHolderBase(DebugControl* owner, ff::StringRef name);

	DebugControl* GetOwner() const;
	ff::StringRef GetName() const;
	const NsString& GetPropertyName() const;
	const NsString& GetFullPropertyName() const;

	virtual void OnVarChanged();

private:
	DebugControl* _owner;
	ff::String _name;
	NsString _fullPropertyName8;
	NsString _propertyName8;

	NS_DECLARE_REFLECTION(VarHolderBase, ff::NotifyPropertyChangedBase);
};

class ValueHolder : public VarHolderBase
{
public:
	ValueHolder(DebugControl* owner, ff::StringRef name);

	const NsString& GetValue() const;
	void SetValue(const NsString& value);
	bool GetError() const;
	void SetError(bool value);
	float GetTextEditWidth() const;

	// VarHolderBase
	virtual void OnVarChanged() override;

private:
	NsString _value;
	bool _error;

	NS_DECLARE_REFLECTION(ValueHolder, VarHolderBase);
};

class BoolHolder : public VarHolderBase
{
public:
	BoolHolder(DebugControl* owner, ff::StringRef name);

	bool GetValue() const;
	void SetValue(bool value);

	// VarHolderBase
	virtual void OnVarChanged() override;

private:
	NS_DECLARE_REFLECTION(BoolHolder, VarHolderBase);
};

class EventHolder : public VarHolderBase, public Noesis::ICommand
{
public:
	EventHolder(DebugControl* owner, ff::StringRef name);

	Noesis::ICommand* GetEventCommand() const;

	// ICommand
	Noesis::EventHandler& CanExecuteChanged() override;
	bool CanExecute(Noesis::BaseComponent* param) const override;
	void Execute(Noesis::BaseComponent* param) const override;

	NS_IMPLEMENT_INTERFACE_FIXUP;

private:
	Noesis::EventHandler _canExecuteChanged;

	NS_DECLARE_REFLECTION(EventHolder, VarHolderBase);
};

class PropertyCategory : public Noesis::BaseComponent
{
public:
	PropertyCategory(ff::StringRef name);

	ff::StringRef GetName() const;
	const NsString& GetName8() const;
	Noesis::ObservableCollection<VarHolderBase>* GetEditableProperties() const;
	const Noesis::ObservableCollection<VarHolderBase>* GetProperties() const;

private:
	ff::String _name;
	NsString _name8;
	Noesis::Ptr<Noesis::ObservableCollection<VarHolderBase>> _properties;

	NS_DECLARE_REFLECTION(PropertyCategory, Noesis::BaseComponent);
};

class ValueHolderTemplateSelector : public Noesis::DataTemplateSelector
{
public:
	virtual Noesis::DataTemplate* SelectTemplate(Noesis::BaseComponent* item, Noesis::DependencyObject* container);

	Noesis::DataTemplate* GetValueTemplate() const;
	void SetValueTemplate(Noesis::DataTemplate* value);
	Noesis::DataTemplate* GetBoolTemplate() const;
	void SetBoolTemplate(Noesis::DataTemplate* value);
	Noesis::DataTemplate* GetEventTemplate() const;
	void SetEventTemplate(Noesis::DataTemplate* value);

private:
	Noesis::Ptr<Noesis::DataTemplate> _valueTemplate;
	Noesis::Ptr<Noesis::DataTemplate> _boolTemplate;
	Noesis::Ptr<Noesis::DataTemplate> _eventTemplate;

	NS_DECLARE_REFLECTION(ValueHolderTemplateSelector, Noesis::DataTemplateSelector);
};

class DebugControlVM : public ff::NotifyPropertyChangedBase
{
public:
	DebugControlVM(DebugControl* owner);
	void Destroy();
	void Reset();
	void OnVarChanged(ff::StringRef name);

	bool IsCheatingEnabled() const;
	bool IsValid() const;
	const Noesis::ObservableCollection<PropertyCategory>* GetCategories() const;

private:
	void AddProperty(ff::StringRef name);

	DebugControl* _owner;
	Noesis::Ptr<Noesis::ObservableCollection<PropertyCategory>> _categories;
	bool _cheatingEnabled;

	NS_DECLARE_REFLECTION(DebugControlVM, ff::NotifyPropertyChangedBase);
};

class DebugControl : public Noesis::UserControl
{
public:
	DebugControl();
	virtual ~DebugControl() override;

	void Reset(Vars* vars);
	DebugControlVM* GetViewModel() const;
	bool IsValid() const;
	void PostEvent(ff::StringRef name);
	bool SetValue(ff::StringRef name, const ff::Value* value);
	ff::ValuePtr GetValue(ff::StringRef name) const;
	ff::Vector<ff::String> GetAllNames() const;

	virtual bool ConnectEvent(Noesis::BaseComponent* source, const char* event, const char* handler) override;

private:
	void OnLoaded(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&);
	void OnResetButtonClick(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&);
	void OnCloseButtonClick(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&);
	void OnClickEnableCheating(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&);

	void OnVarChanged(ff::StringRef name);
	void OnVarsDestroyed();
	void OnVarsReset();
	void SetVars(Vars* vars);

	IAppService* _appService;
	Vars* _vars;
	ff::Dict _dict;
	Noesis::Ptr<DebugControlVM> _viewModel;
	ff::EventCookie _varChangedEvent;
	ff::EventCookie _varsDestroyedEvent;
	ff::EventCookie _varsResetEvent;

	NS_DECLARE_REFLECTION(DebugControl, Noesis::UserControl);
};
