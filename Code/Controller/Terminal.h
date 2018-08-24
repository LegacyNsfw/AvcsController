#pragma once


class ITerminal
{
public:
	static ITerminal* GetInstance();

	virtual void Initialize() = 0;
	virtual void Update() = 0;
};