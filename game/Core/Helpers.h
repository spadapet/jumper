#pragma once

#include "Entity/Entity.h"

namespace Helpers
{
	// Conversions
	inline ff::PointInt PointToCell(ff::PointFixedInt pos, ff::PointFixedInt cellSize);
	inline float DegreesToRadians(ff::FixedInt angle);
	template<typename T> DirectX::XMVECTOR PointToXMVECTOR(const ff::PointType<T>& point);

	// Enum flags
	template<typename T> bool HasAllFlags(T state, T check);
	template<typename T> bool HasAnyFlags(T state, T check);
	template<typename T> T SetFlags(T state, T set, bool value);
	template<typename T> T SetFlags(T state, T set);
	template<typename T> T ClearFlags(T state, T clear);
	template<typename T> T CombineFlags(T f1, T f2);
	template<typename T> T CombineFlags(T f1, T f2, T f3);
	template<typename T> T CombineFlags(T f1, T f2, T f3, T f4);

	// Entity
	void RequestDeleteEntity(ff::Entity entity);
}

ff::PointInt Helpers::PointToCell(ff::PointFixedInt pos, ff::PointFixedInt cellSize)
{
	ff::PointFixedInt cell = pos / cellSize;
	return cell.ToType<int>();
}

float Helpers::DegreesToRadians(ff::FixedInt angle)
{
	return (float)angle * ff::PI2_F / -360.0f;
}

template<typename T>
DirectX::XMVECTOR Helpers::PointToXMVECTOR(const ff::PointType<T>& point)
{
	ff::PointFloat pointF = point.ToType<float>();
	DirectX::XMFLOAT2 pointDX(pointF.x, pointF.y);
	return DirectX::XMLoadFloat2(&pointDX);
}

template<typename T>
bool Helpers::HasAllFlags(T state, T check)
{
	typedef std::underlying_type_t<T> TI;
	return ((TI)state & (TI)check) == (TI)check;
}

template<typename T>
bool Helpers::HasAnyFlags(T state, T check)
{
	typedef std::underlying_type_t<T> TI;
	return ((TI)state & (TI)check) != (TI)0;
}

template<typename T>
T Helpers::SetFlags(T state, T set, bool value)
{
	typedef std::underlying_type_t<T> TI;
	const TI istate = (TI)state;
	const TI iset = (TI)set;
	const TI ival = (TI)value;

	return (T)((istate & ~iset) | (iset * ival));
}

template<typename T>
T Helpers::SetFlags(T state, T set)
{
	typedef std::underlying_type_t<T> TI;
	return (T)((TI)state | (TI)set);
}

template<typename T>
T Helpers::ClearFlags(T state, T clear)
{
	typedef std::underlying_type_t<T> TI;
	return (T)((TI)state & ~(TI)clear);
}

template<typename T>
T Helpers::CombineFlags(T f1, T f2)
{
	typedef std::underlying_type_t<T> TI;
	return (T)((TI)f1 | (TI)f2);
}

template<typename T>
T Helpers::CombineFlags(T f1, T f2, T f3)
{
	typedef std::underlying_type_t<T> TI;
	return (T)((TI)f1 | (TI)f2 | (TI)f3);
}

template<typename T>
T Helpers::CombineFlags(T f1, T f2, T f3, T f4)
{
	typedef std::underlying_type_t<T> TI;
	return (T)((TI)f1 | (TI)f2 | (TI)f3 | (TI)f4);
}
