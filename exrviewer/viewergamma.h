#pragma once

#include <ImfArray.h>
#include <ImfHeader.h>
#include <ImathFun.h>
#include <ImathLimits.h>
#include "namespaceAlias.h"

#define max(a,b)            (((a) > (b)) ? (a) : (b))

inline float POW2f(float v)
{
	return IMATH::Math<float>::pow(2, v);
}

inline float Knee(double x, double f)
{
	return float(IMATH::Math<double>::log(x * f + 1) / f);
}

inline float FindKneeF(float x, float y)
{
	float f0 = 0;
	float f1 = 1;

	while (Knee(x, f1) > y)
	{
		f0 = f1;
		f1 = f1 * 2;
	}

	for (int i = 0; i < 30; ++i)
	{
		float f2 = (f0 + f1) / 2;
		float y2 = Knee(x, f2);

		if (y2 < y)
			f1 = f2;
		else
			f0 = f2;
	}

	return (f0 + f1) / 2;
}

class GammaLutCalculator
{
public:
	GammaLutCalculator(float gamma,
		float exposure,
		float defog,
		float kneeLow,
		float kneeHigh,
		float* lut = nullptr,
		half domainMin = -HALF_MAX,
		half domainMax = HALF_MAX,
		float defaultValue = 0.f,
		float posInfValue = 255.f,
		float negInfValue = 0.f,
		float nanValue = 0.f)
	{
		if (lut)
		{
			m_lut = lut;
			m_bLutAllocated = false;
		}
		else
		{
			m_lut = new float[1 << 16];
			m_bLutAllocated = true;
		}
		memset(m_lut, 0, sizeof(float)* (1 << 16));

		InitGammaValue(gamma, exposure, defog, kneeLow, kneeHigh);
		InitRangeValue(domainMin, domainMax, defaultValue, posInfValue, negInfValue, nanValue);
		InitLut();
	}
	~GammaLutCalculator() {	if (m_bLutAllocated) SAFEDELETEARRAY(m_lut); }

	inline float calc(half x) const
	{
		return m_lut[x.bits()];
	}

protected:
	void InitGammaValue(float gamma,
		float exposure,
		float defog,
		float kneeLow,
		float kneeHigh)
	{
		m_gamma = gamma;
		m_m = POW2f(exposure + 2.47393);
		m_defog = defog;
		m_kneeLowPow2 = POW2f(kneeLow);
		m_f = FindKneeF(POW2f(kneeHigh) - m_kneeLowPow2, POW2f(3.5) - m_kneeLowPow2);
		m_scale = 255.0 * POW2f(-3.5 * m_gamma);
	}

	void InitRangeValue(
		half domainMin = -HALF_MAX,
		half domainMax = HALF_MAX,
		float defaultValue = 0.f,
		float posInfValue = 255.f,
		float negInfValue = 0.f,
		float nanValue = 0.f)
	{
		m_domainMin = domainMin;
		m_domainMax = domainMax;
		m_defaultValue = defaultValue;
		m_posInfValue = posInfValue;
		m_negInfValue = negInfValue;
		m_nanValue = nanValue;
	}

	void InitLut()
	{
		for (int i = 0; i < (1 << 16); i++)
		{
			half x;
			x.setBits(i);

			if (x.isNan())
				m_lut[i] = m_nanValue;
			else if (x.isInfinity())
				m_lut[i] = x.isNegative() ? m_negInfValue : m_posInfValue;
			else if (x < m_domainMin || x > m_domainMax)
				m_lut[i] = m_defaultValue;
			else
				m_lut[i] = GammaInitFunc(x);
		}
	}

	float GammaInitFunc(half h)
	{
		float x = max(0.f, (h - m_defog)); // Defog
		x *= m_m; // Exposure
	  
		if (x > m_kneeLowPow2)
		{
			// Knee
			x = m_kneeLowPow2 + Knee(x - m_kneeLowPow2, m_f);
		}

		x = IMATH::Math<float>::pow(x, m_gamma); // Gamma
		return IMATH_NAMESPACE::clamp(x * m_scale, 0.f, 255.f); // Scale and clamp
	}

protected:
	//float m_lut[1 << 16];
	float *m_lut;
	bool m_bLutAllocated;
	half  m_domainMin;
	half  m_domainMax;
	float m_defaultValue;
	float m_posInfValue;
	float m_negInfValue;
	float m_nanValue;

	// gamma values
	float m_gamma;
	float m_m, m_defog;
	float m_kneeLowPow2;
	float m_f, m_scale;
};
