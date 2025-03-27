#include "DeviceProvider.h"

EVRInitError DeviceProvider::Init(IVRDriverContext* pDriverContext)
{
	VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);

	VRDriverLog()->Log("Initializing CAXE Controller");

	controllerDriver = new ControllerDriver();
	VRServerDriverHost()->TrackedDeviceAdded("caxe_controller", TrackedDeviceClass_Controller, controllerDriver);
	
	return VRInitError_None;
}

void DeviceProvider::Cleanup()
{
	delete controllerDriver;
	controllerDriver = NULL;
}

const char* const* DeviceProvider::GetInterfaceVersions()
{
	return k_InterfaceVersions;
}

void DeviceProvider::RunFrame()
{
	if (controllerDriver != nullptr) {
		controllerDriver->RunFrame();
	}
}

bool DeviceProvider::ShouldBlockStandbyMode()
{
	return false;
}

void DeviceProvider::EnterStandby() {}

void DeviceProvider::LeaveStandby() {}

