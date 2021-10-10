#pragma once

class MeterUnitConverter
{
public:
	MeterUnitConverter(float meterToUnit = 1.0f) : meterToUnitRatio(meterToUnit), invMeterToUnitRatio(1.0f / meterToUnit) {}

	template<class T> T ToUnits(T val) { return val * invMeterToUnitRatio; }
	template<class T> T ToMeters(T val) { return val * meterToUnitRatio; }
	float GetMeterUnitRatio() { return meterToUnitRatio; }
	float GetInvMeterUnitRatio() { return invMeterToUnitRatio; }
protected:
	float meterToUnitRatio;
	float invMeterToUnitRatio;
};
