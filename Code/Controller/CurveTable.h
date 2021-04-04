class CurveTable
{
private:
	int _elements;
	float *_pInputValues;
	float *_pOutputValues;

	float Interpolate(float x, float x0, float x1, float y0, float y1)
	{
		if (x <= x0)
		{
			return y0;
		}

		if (x >= x1)
		{
			return y1;
		}

		float numerator = (y0 * (x1 - x)) + (y1 * (x - x0));
		float denominator = x1 - x0;

		return numerator / denominator;
	}

public:
	static CurveTable * CreateExhaustCamTable();
	static CurveTable * CreateRpmFilterTable();
	static CurveTable* CreateGainTable();

	CurveTable(
		int elements,
		float *pInputValues,
		float *pOutputValues)
	{
		_elements = elements;
		_pInputValues = pInputValues;
		_pOutputValues = pOutputValues;
	}

	float GetValue(float input)
	{
		if (input <= _pInputValues[0])
		{
			return _pOutputValues[0];
		}

		for (int element = 1; element <= _elements; element++)
		{
			if (input < _pInputValues[element])
			{
				return Interpolate(
					input,
					_pInputValues[element - 1],
					_pInputValues[element],
					_pOutputValues[element - 1],
					_pOutputValues[element]);
			}
		}

		return _pOutputValues[_elements - 1];
	}
};

void SelfTestCurveTable();