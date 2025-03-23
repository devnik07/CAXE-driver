#pragma once

#include "ControllerDriver.h"

#include <openvr_driver.h>

using namespace vr;

/**
* This class instantiates the device driver and inform OpenVR about the device.
**/
class DeviceProvider : public IServerTrackedDeviceProvider
{
public:

	/**
	* Initiailze and add your drivers to OpenVR here.
	**/
	EVRInitError Init(IVRDriverContext* pDriverContext) override;

	/**
	* Called right before your driver is unloaded.
	**/
	void Cleanup() override;

	/**
	* Returns version of the openVR interface this driver works with.
	**/
	const char* const* GetInterfaceVersions() override;

	/**
	* Called every frame. Update your drivers here.
	**/
	void RunFrame() override;

	/**
	* Return true if standby mode should be blocked. False otherwise.
	**/
	bool ShouldBlockStandbyMode() override;

	/**
	* Called when OpenVR goes into stand-by mode, so you can tell your devices to go into stand-by mode
	**/
	void EnterStandby() override;

	/**
	* Called when OpenVR leaves stand-by mode.
	**/
	void LeaveStandby() override;

private:
	ControllerDriver* controllerDriver; //this provider only provides this one controller driver.
};