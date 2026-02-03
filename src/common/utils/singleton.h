#pragma once

template<typename T>
class Singleton
{
public:
	static T& Get()
	{
		static T Instance;
		return Instance;
	}
};