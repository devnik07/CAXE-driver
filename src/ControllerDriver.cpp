#include "ControllerDriver.h"

EVRInitError ControllerDriver::Activate(uint32_t unObjectId)
{
	isActive = true;

	driverId = unObjectId;
	PropertyContainerHandle_t props = VRProperties()->TrackedDeviceToPropertyContainer(driverId); // stores all the information about our driver

	VRProperties()->SetStringProperty(props, Prop_InputProfilePath_String, "{caxe}/resources/input/caxe_controller_profile.json");
	VRProperties()->SetBoolProperty(props, Prop_WillDriftInYaw_Bool, true);

	poseUpdateThread = std::thread(&ControllerDriver::PoseUpdateThread, this);

	return VRInitError_None;
}

DriverPose_t ControllerDriver::GetPose()
{
	DriverPose_t pose = { 0 };
	pose.qWorldFromDriverRotation.w = 1.f;
	pose.qDriverFromHeadRotation.w = 1.f;

	// TODO: update pose by getting data from the Arduino

	pose.poseIsValid = true;
	pose.deviceIsConnected = true;
	pose.result = TrackingResult_Running_OK;

	return pose;
}

void ControllerDriver::Deactivate()
{
	if (isActive.exchange(false)) {
		poseUpdateThread.join();
	}

	// unassign our controller index (we don't want to be calling vrserver anymore after Deactivate() has been called.
	driverId = k_unTrackedDeviceIndexInvalid;
}

void ControllerDriver::EnterStandby() {}

/**
* If this was a HMD driver, an implementation of vr::IVRDisplayComponent, vr::IVRVirtualDisplay 
* or vr::IVRDirectModeComponent would be returned.
**/
void* ControllerDriver::GetComponent(const char* pchComponentNameAndVersion)
{
	return nullptr;
}

void ControllerDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
	if (unResponseBufferSize >= 1) {
		pchResponseBuffer[0] = 0;
	}
}

/**
* Updates the component states of the controller.
* Since we don't have any components we won't update anything.
**/
void ControllerDriver::RunFrame()
{
	return;
}

void ControllerDriver::PoseUpdateThread()
{
	while (isActive) {

		// Inform the vr server that our tracked device's pose has updated, giving it the pose returned by our GetPose().
		VRServerDriverHost()->TrackedDevicePoseUpdated(driverId, GetPose(), sizeof(DriverPose_t));

		// Update our pose every five milliseconds.
		// In reality, you should update the pose whenever you have new data from your device.
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}
