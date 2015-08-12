#include "pch.h"
#include "Core/Vars.h"
#include "Services/AppService.h"
#include "UI/DebugControl.h"
#include "UI/DebugPage.h"
#include "Value/Values.h"

static ff::StaticString s_didCheatName(L"Debug.Did Cheat");

static NsString StringToNoesis(ff::StringRef text)
{
	int len = (int)text.size() + 1;
	NsString ns;
	ns.resize(::WideCharToMultiByte(CP_ACP, 0, text.c_str(), len, nullptr, 0, nullptr, nullptr));
	::WideCharToMultiByte(CP_ACP, 0, text.c_str(), len, &ns[0], (int)ns.size(), nullptr, nullptr);
	return ns;
}

NS_IMPLEMENT_REFLECTION(VarHolderBase)
{
	NsMeta<Noesis::TypeId>("Jumper.VarHolderBase");
	NsProp("PropertyName", &VarHolderBase::GetPropertyName);
	NsProp("FullPropertyName", &VarHolderBase::GetFullPropertyName);
}

VarHolderBase::VarHolderBase(DebugControl* owner, ff::StringRef name)
	: _owner(owner)
	, _name(name)
{
	size_t dot = _name.find('.');
	_fullPropertyName8 = ::StringToNoesis(_name);
	_propertyName8 = (dot != ff::INVALID_SIZE) ? ::StringToNoesis(_name.substr(dot + 1)) : _fullPropertyName8;
}

DebugControl* VarHolderBase::GetOwner() const
{
	return _owner;
}

ff::StringRef VarHolderBase::GetName() const
{
	return _name;
}

const NsString& VarHolderBase::GetPropertyName() const
{
	return _propertyName8;
}

const NsString& VarHolderBase::GetFullPropertyName() const
{
	return _fullPropertyName8;
}

void VarHolderBase::OnVarChanged()
{
}

NS_IMPLEMENT_REFLECTION(ValueHolder)
{
	NsMeta<Noesis::TypeId>("Jumper.ValueHolder");
	NsProp("Value", &ValueHolder::GetValue, &ValueHolder::SetValue);
	NsProp("Error", &ValueHolder::GetError, &ValueHolder::SetError);
	NsProp("TextEditWidth", &ValueHolder::GetTextEditWidth);
}

ValueHolder::ValueHolder(DebugControl* owner, ff::StringRef name)
	: VarHolderBase(owner, name)
	, _error(false)
{
	ff::ValuePtrT<ff::StringValue> value = GetOwner()->GetValue(GetName());
	_value = ::StringToNoesis(value.GetValue());
}

const NsString& ValueHolder::GetValue() const
{
	return _value;
}

void ValueHolder::SetValue(const NsString& value)
{
	if (_value != value)
	{
		_value = value;
		OnPropertyChanged("Value");

		ff::ValuePtr strValue = ff::Value::New<ff::StringValue>(ff::String::from_utf8(value.c_str()));
		SetError(!GetOwner()->SetValue(GetName(), strValue));
	}
}

bool ValueHolder::GetError() const
{
	return _error;
}

void ValueHolder::SetError(bool value)
{
	if (_error != value)
	{
		_error = value;
		OnPropertyChanged("Error");
	}
}

float ValueHolder::GetTextEditWidth() const
{
	ff::ValuePtr value = GetOwner()->GetValue(GetName());
	size_t count = (value && value->GetIndexChildCount()) ? value->GetIndexChildCount() : 1;
	return 35.0f * count;
}

void ValueHolder::OnVarChanged()
{
	SetError(false);

	ff::ValuePtrT<ff::StringValue> value = GetOwner()->GetValue(GetName());
	NsString value8 = ::StringToNoesis(value.GetValue());

	if (_value != value8)
	{
		_value = value8;
		OnPropertyChanged("Value");
	}
}

NS_IMPLEMENT_REFLECTION(BoolHolder)
{
	NsMeta<Noesis::TypeId>("Jumper.BoolHolder");
	NsProp("Value", &BoolHolder::GetValue, &BoolHolder::SetValue);
}

BoolHolder::BoolHolder(DebugControl* owner, ff::StringRef name)
	: VarHolderBase(owner, name)
{
}

bool BoolHolder::GetValue() const
{
	return GetOwner()->GetValue(GetName())->Convert<ff::BoolValue>()->GetValue<ff::BoolValue>();
}

void BoolHolder::SetValue(bool value)
{
	GetOwner()->SetValue(GetName(), ff::Value::New<ff::BoolValue>(value));
}

void BoolHolder::OnVarChanged()
{
	OnPropertyChanged("Value");
}

NS_IMPLEMENT_REFLECTION(EventHolder)
{
	NsMeta<Noesis::TypeId>("Jumper.EventHolder");
	NsProp("EventCommand", &EventHolder::GetEventCommand);
	NsImpl<Noesis::ICommand>();
}

EventHolder::EventHolder(DebugControl* owner, ff::StringRef name)
	: VarHolderBase(owner, name)
{
}

Noesis::ICommand* EventHolder::GetEventCommand() const
{
	return const_cast<EventHolder*>(this);
}

Noesis::EventHandler& EventHolder::CanExecuteChanged()
{
	return _canExecuteChanged;
}

bool EventHolder::CanExecute(Noesis::BaseComponent* param) const
{
	return !GetName().empty();
}

void EventHolder::Execute(Noesis::BaseComponent* param) const
{
	GetOwner()->PostEvent(GetName());
}

NS_IMPLEMENT_REFLECTION(PropertyCategory)
{
	NsMeta<Noesis::TypeId>("Jumper.PropertyCategory");
	NsProp("Name", &PropertyCategory::GetName8);
	NsProp("Properties", &PropertyCategory::GetProperties);
}

PropertyCategory::PropertyCategory(ff::StringRef name)
	: _name(name)
	, _name8(::StringToNoesis(name))
	, _properties(Noesis::MakePtr<Noesis::ObservableCollection<VarHolderBase>>())
{
}

ff::StringRef PropertyCategory::GetName() const
{
	return _name;
}

const NsString& PropertyCategory::GetName8() const
{
	return _name8;
}

Noesis::ObservableCollection<VarHolderBase>* PropertyCategory::GetEditableProperties() const
{
	return _properties;
}

const Noesis::ObservableCollection<VarHolderBase>* PropertyCategory::GetProperties() const
{
	return _properties;
}

NS_IMPLEMENT_REFLECTION(ValueHolderTemplateSelector)
{
	NsMeta<Noesis::TypeId>("Jumper.ValueHolderTemplateSelector");
	NsProp("ValueTemplate", &ValueHolderTemplateSelector::GetValueTemplate, &ValueHolderTemplateSelector::SetValueTemplate);
	NsProp("BoolTemplate", &ValueHolderTemplateSelector::GetBoolTemplate, &ValueHolderTemplateSelector::SetBoolTemplate);
	NsProp("EventTemplate", &ValueHolderTemplateSelector::GetEventTemplate, &ValueHolderTemplateSelector::SetEventTemplate);
}

Noesis::DataTemplate* ValueHolderTemplateSelector::SelectTemplate(Noesis::BaseComponent* item, Noesis::DependencyObject* container)
{
	if (Noesis::DynamicCast<ValueHolder*>(item))
	{
		return _valueTemplate;
	}

	if (Noesis::DynamicCast<BoolHolder*>(item))
	{
		return _boolTemplate;
	}

	if (Noesis::DynamicCast<EventHolder*>(item))
	{
		return _eventTemplate;
	}

	return nullptr;
}

Noesis::DataTemplate* ValueHolderTemplateSelector::GetValueTemplate() const
{
	return _valueTemplate;
}

void ValueHolderTemplateSelector::SetValueTemplate(Noesis::DataTemplate* value)
{
	_valueTemplate.Reset(value);
}

Noesis::DataTemplate* ValueHolderTemplateSelector::GetBoolTemplate() const
{
	return _boolTemplate;
}

void ValueHolderTemplateSelector::SetBoolTemplate(Noesis::DataTemplate* value)
{
	_boolTemplate.Reset(value);
}

Noesis::DataTemplate* ValueHolderTemplateSelector::GetEventTemplate() const
{
	return _eventTemplate;
}

void ValueHolderTemplateSelector::SetEventTemplate(Noesis::DataTemplate* value)
{
	_eventTemplate.Reset(value);
}

NS_IMPLEMENT_REFLECTION(DebugControlVM)
{
	NsMeta<Noesis::TypeId>("Jumper.DebugControlVM");
	NsProp("IsCheatingEnabled", &DebugControlVM::IsCheatingEnabled);
	NsProp("IsValid", &DebugControlVM::IsValid);
	NsProp("Categories", &DebugControlVM::GetCategories);
}

DebugControlVM::DebugControlVM(DebugControl* owner)
	: _owner(owner)
	, _categories(Noesis::MakePtr<Noesis::ObservableCollection<PropertyCategory>>())
	, _cheatingEnabled(false)
{
}

void DebugControlVM::Destroy()
{
	_owner = nullptr;
	OnPropertyChanged("IsValid");
}

void DebugControlVM::Reset()
{
	_categories->Clear();
	_cheatingEnabled = _owner && _owner->GetValue(s_didCheatName)->GetValue<ff::BoolValue>();

	if (_cheatingEnabled)
	{
		for (ff::StringRef name : _owner->GetAllNames())
		{
			AddProperty(name);
		}
	}

	OnPropertyChanged("IsCheatingEnabled");
	OnPropertyChanged("IsValid");
}

void DebugControlVM::OnVarChanged(ff::StringRef name)
{
	for (int i = 0; i < _categories->Count(); i++)
	{
		PropertyCategory* category = _categories->Get(i);

		for (int h = 0; h < category->GetProperties()->Count(); h++)
		{
			VarHolderBase* var = category->GetProperties()->Get(h);

			if (name == var->GetName())
			{
				var->OnVarChanged();
				return;
			}
		}
	}
}

bool DebugControlVM::IsCheatingEnabled() const
{
	return _cheatingEnabled;
}

bool DebugControlVM::IsValid() const
{
	return _owner && _owner->IsValid();
}

const Noesis::ObservableCollection<PropertyCategory>* DebugControlVM::GetCategories() const
{
	return _categories;
}

void DebugControlVM::AddProperty(ff::StringRef name)
{
	noAssertRet(name != s_didCheatName.GetString());

	ff::String categoryName;
	size_t dot = name.find('.');
	if (dot != ff::INVALID_SIZE)
	{
		categoryName = name.substr(0, dot);
	}

	Noesis::Ptr<PropertyCategory> categoryToAdd;
	for (int i = 0; i < _categories->Count(); i++)
	{
		PropertyCategory* category = _categories->Get(i);

		if (categoryName == category->GetName())
		{
			categoryToAdd.Reset(category);
			break;
		}
	}

	if (categoryToAdd == nullptr)
	{
		categoryToAdd = Noesis::MakePtr<PropertyCategory>(categoryName);
		_categories->Add(categoryToAdd);
	}

	ff::ValuePtr value = _owner->GetValue(name);
	Noesis::Ptr<VarHolderBase> prop;

	if (value->IsType<ff::HashValue>())
	{
		prop = *new EventHolder(_owner, name);
	}
	else if (value->IsType<ff::BoolValue>())
	{
		prop = *new BoolHolder(_owner, name);
	}
	else if (value)
	{
		prop = *new ValueHolder(_owner, name);
	}

	if (prop)
	{
		categoryToAdd->GetEditableProperties()->Add(prop);
	}
}

NS_IMPLEMENT_REFLECTION(DebugControl)
{
	NsMeta<Noesis::TypeId>("Jumper.DebugControl");
	NsProp("ViewModel", &DebugControl::GetViewModel);
}

DebugControl::DebugControl()
	: _appService(nullptr)
	, _vars(nullptr)
	, _viewModel(Noesis::MakePtr<DebugControlVM>(this))
	, _varChangedEvent(nullptr)
	, _varsDestroyedEvent(nullptr)
	, _varsResetEvent(nullptr)
{
	Loaded() += Noesis::MakeDelegate(this, &DebugControl::OnLoaded);
	Noesis::GUI::LoadComponent(this, "DebugControl.xaml");
}

DebugControl::~DebugControl()
{
	_viewModel->Destroy();
	SetVars(nullptr);
}

void DebugControl::Reset(Vars* vars)
{
	SetVars(vars);

	if (vars)
	{
		_dict = vars->Save();
	}
	else
	{
		_dict.Clear();
	}

	_viewModel->Reset();
}

DebugControlVM* DebugControl::GetViewModel() const
{
	return _viewModel;
}

bool DebugControl::IsValid() const
{
	return _vars != nullptr;
}

void DebugControl::PostEvent(ff::StringRef name)
{
	ff::hash_t eventId = ff::HashFunc(name);
	ff::ValuePtrT<ff::HashValue> value = GetValue(name);

	if (value && value.GetValue())
	{
		_appService->PostEvent(value.GetValue());
	}
}

bool DebugControl::SetValue(ff::StringRef name, const ff::Value* value)
{
	ff::ValuePtr currentValue = GetValue(name);
	assertRetVal(currentValue, false);

	ff::ValuePtr newValue = value->Convert(currentValue->GetTypeIndex());
	noAssertRetVal(newValue, false);

	_dict.SetValue(name, newValue);

	if (_vars)
	{
		_vars->Set(name, newValue);
	}

	return true;
}

ff::ValuePtr DebugControl::GetValue(ff::StringRef name) const
{
	return _dict.GetValue(name);
}

ff::Vector<ff::String> DebugControl::GetAllNames() const
{
	return _dict.GetAllNames(true);
}

bool DebugControl::ConnectEvent(Noesis::BaseComponent* source, const char* event, const char* handler)
{
	NS_CONNECT_EVENT(Noesis::Button, Click, OnCloseButtonClick);
	NS_CONNECT_EVENT(Noesis::Button, Click, OnClickEnableCheating);
	NS_CONNECT_EVENT(Noesis::Button, Click, OnResetButtonClick);
	return false;
}

void DebugControl::OnLoaded(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&)
{
	DebugPage* page = Noesis::DynamicCast<DebugPage*, Noesis::FrameworkElement*>(this->GetParent());
	assertRet(page);

	_appService = page->GetAppService();
}

void DebugControl::OnResetButtonClick(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&)
{
	_vars->Reset();
}

void DebugControl::OnCloseButtonClick(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&)
{
	if (_appService)
	{
		_appService->ShowDebugUI(false);
	}
}

void DebugControl::OnClickEnableCheating(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&)
{
	assertRet(_vars);
	_vars->SetDidCheat();
}

void DebugControl::OnVarChanged(ff::StringRef name)
{
	if (name == s_didCheatName.GetString())
	{
		Reset(_vars);
		return;
	}

	ff::ValuePtr value = _vars->Get(name);

	if (!_dict.GetValue(name)->Compare(value))
	{
		_dict.SetValue(name, value);
		_viewModel->OnVarChanged(name);
	}
}

void DebugControl::OnVarsDestroyed()
{
	Reset(nullptr);
}

void DebugControl::OnVarsReset()
{
	Reset(_vars);
}

void DebugControl::SetVars(Vars* vars)
{
	noAssertRet(_vars != vars);

	if (_vars)
	{
		_vars->VarChanged.Remove(_varChangedEvent);
		_vars->VarsDestroyed.Remove(_varsDestroyedEvent);
		_vars->VarsReset.Remove(_varsResetEvent);
	}

	_vars = vars;

	if (_vars)
	{
		_varChangedEvent = _vars->VarChanged.Add([this](const Vars*, ff::StringRef name)
			{
				OnVarChanged(name);
			});

		_varsDestroyedEvent = _vars->VarsDestroyed.Add([this](const Vars*)
			{
				OnVarsDestroyed();
			});

		_varsResetEvent = _vars->VarsReset.Add([this](const Vars*)
			{
				OnVarsReset();
			});
	}
}
