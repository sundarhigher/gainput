
#include <gainput/gainput.h>

#include "GainputInputDeviceKeyboardImpl.h"
#include "GainputKeyboardKeyNames.h"
#include "../GainputInputDeltaState.h"
#include "../GainputHelpers.h"
#include "../GainputLog.h"

#if defined(GAINPUT_PLATFORM_LINUX)
	#include "GainputInputDeviceKeyboardLinux.h"
	#include "GainputInputDeviceKeyboardEvdev.h"
#elif defined(GAINPUT_PLATFORM_WIN)
	#include "GainputInputDeviceKeyboardWin.h"
	#include "GainputInputDeviceKeyboardWinRaw.h"
#elif defined(GAINPUT_PLATFORM_ANDROID)
	#include "GainputInputDeviceKeyboardAndroid.h"
#elif defined(GAINPUT_PLATFORM_MAC)
	#include "GainputInputDeviceKeyboardMac.h"
#endif

#include "GainputInputDeviceKeyboardNull.h"


namespace gainput
{

InputDeviceKeyboard::InputDeviceKeyboard(InputManager& manager, DeviceId device, DeviceVariant variant) :
	InputDevice(manager, device, manager.GetDeviceCountByType(DT_KEYBOARD)),
	impl_(0),
	keyNames_(manager_.GetAllocator())
{
	state_ = manager.GetAllocator().New<InputState>(manager.GetAllocator(), KeyCount_);
	GAINPUT_ASSERT(state_);
	previousState_ = manager.GetAllocator().New<InputState>(manager.GetAllocator(), KeyCount_);
	GAINPUT_ASSERT(previousState_);

#if defined(GAINPUT_PLATFORM_LINUX)
	if (variant == DV_STANDARD)
	{
		impl_ = manager.GetAllocator().New<InputDeviceKeyboardImplLinux>(manager, device, *state_, *previousState_);
	}
	else if (variant == DV_RAW)
	{
		impl_ = manager.GetAllocator().New<InputDeviceKeyboardImplEvdev>(manager, device, *state_, *previousState_);
	}
#elif defined(GAINPUT_PLATFORM_WIN)
	if (variant == DV_STANDARD)
	{
		impl_ = manager.GetAllocator().New<InputDeviceKeyboardImplWin>(manager, device, *state_, *previousState_);
	}
	else if (variant == DV_RAW)
	{
		impl_ = manager.GetAllocator().New<InputDeviceKeyboardImplWinRaw>(manager, device, *state_, *previousState_);
	}
#elif defined(GAINPUT_PLATFORM_ANDROID)
	impl_ = manager.GetAllocator().New<InputDeviceKeyboardImplAndroid>(manager, device, *state_, *previousState_);
#elif defined(GAINPUT_PLATFORM_MAC)
	impl_ = manager.GetAllocator().New<InputDeviceKeyboardImplMac>(manager, device, *state_, *previousState_);
#endif

	if (!impl_)
	{
		impl_ = manager.GetAllocator().New<InputDeviceKeyboardImplNull>(manager, device);
	}

	GAINPUT_ASSERT(impl_);

	GetKeyboardKeyNames(keyNames_);
}

InputDeviceKeyboard::~InputDeviceKeyboard()
{
	manager_.GetAllocator().Delete(state_);
	manager_.GetAllocator().Delete(previousState_);
	manager_.GetAllocator().Delete(impl_);
}

void
InputDeviceKeyboard::InternalUpdate(InputDeltaState* delta)
{
	impl_->Update(delta);
}

InputDevice::DeviceState
InputDeviceKeyboard::InternalGetState() const
{
	return impl_->GetState();

}

InputDevice::DeviceVariant
InputDeviceKeyboard::GetVariant() const
{
	return impl_->GetVariant();
}

size_t
InputDeviceKeyboard::GetAnyButtonDown(DeviceButtonSpec* outButtons, size_t maxButtonCount) const
{
	GAINPUT_ASSERT(outButtons);
	GAINPUT_ASSERT(maxButtonCount > 0);
	return CheckAllButtonsDown(outButtons, maxButtonCount, 0, KeyCount_);
}

size_t
InputDeviceKeyboard::GetButtonName(DeviceButtonId deviceButton, char* buffer, size_t bufferLength) const
{
	GAINPUT_ASSERT(IsValidButtonId(deviceButton));
	GAINPUT_ASSERT(buffer);
	GAINPUT_ASSERT(bufferLength > 0);
	HashMap<Key, const char*>::const_iterator it = keyNames_.find(Key(deviceButton));
	if (it == keyNames_.end())
	{
		return 0;
	}
	strncpy(buffer, it->second, bufferLength);
	buffer[bufferLength-1] = 0;
	const size_t nameLen = strlen(it->second);
	return nameLen >= bufferLength ? bufferLength : nameLen+1;
}

ButtonType
InputDeviceKeyboard::GetButtonType(DeviceButtonId deviceButton) const
{
	GAINPUT_ASSERT(IsValidButtonId(deviceButton));
	return BT_BOOL;
}

DeviceButtonId
InputDeviceKeyboard::GetButtonByName(const char* name) const
{
	GAINPUT_ASSERT(name);
	for (HashMap<Key, const char*>::const_iterator it = keyNames_.begin();
			it != keyNames_.end();
			++it)
	{
		if (strcmp(name, it->second) == 0)
		{
			return it->first;
		}
	}
	return InvalidDeviceButtonId;
}

bool
InputDeviceKeyboard::IsTextInputEnabled() const
{
	return impl_->IsTextInputEnabled();
}

void
InputDeviceKeyboard::SetTextInputEnabled(bool enabled)
{
	impl_->SetTextInputEnabled(enabled);
}

char
InputDeviceKeyboard::GetNextCharacter()
{
	return impl_->GetNextCharacter();
}

}

