#pragma once

#include <assert.h>
#include <vector>

typedef void (*HotkeyCallbackFunc)(void);

static const int kIdStartRangeExe = 0x0000;
static const int kIdStartRangeDll = 0xC000;


class HotkeyManager
{
public:
	HotkeyManager(int _startingId) 
	: mHotkeyCounter(_startingId)
	{ }

	~HotkeyManager()
	{
	}
	
	void AddHotkey(HWND _hwnd, UINT _fsModifiers, UINT _vk, HotkeyCallbackFunc _func)
	{
		if (!RegisterHotKey(_hwnd, AllocateHotkeyId(), _fsModifiers, _vk)) {
			throw 2;
		}

		mHotkeyFuncs.push_back(_func);
	}

	void OnWmHotkey(const MSG& _msg)
	{
		assert(_msg.wParam >= 0 && _msg.wParam < mHotkeyFuncs.size());
		assert(mHotkeyFuncs[_msg.wParam]);

		mHotkeyFuncs[_msg.wParam]();
	}

private:
	int AllocateHotkeyId()
	{
		return mHotkeyCounter++;
	}

	int mHotkeyCounter;

	std::vector<HotkeyCallbackFunc> mHotkeyFuncs;
};

