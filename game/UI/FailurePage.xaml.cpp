﻿#include "pch.h"
#include "Globals/AppGlobals.h"
#include "String/StringUtil.h"
#include "UI/FailurePage.xaml.h"
#include "Windows/FileUtil.h"

Jumper::FailurePage::FailurePage(ff::AppGlobals* globals)
{
	ff::String errorText;
	if (ff::ReadWholeFile(globals->GetLogFile(), errorText))
	{
		ff::ReplaceAll(errorText, ff::String(L"\r\n"), ff::String(L"\n"));
		ff::ReplaceAll(errorText, ff::String(L"\n"), ff::String(L"\r\n"));
		_error = errorText.pstring();
	}

	InitializeComponent();
}

Platform::String^ Jumper::FailurePage::ErrorText::get()
{
	return _error ? _error : "";
}
