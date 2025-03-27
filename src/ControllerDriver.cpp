#include "ControllerDriver.h"

#include "serial/serial.h"

EVRInitError ControllerDriver::Activate(uint32_t unObjectId)
{
	isActive = true;

	driverId = unObjectId;
	PropertyContainerHandle_t props = VRProperties()->TrackedDeviceToPropertyContainer(driverId); // stores all the information about our driver

	VRProperties()->SetStringProperty(props, Prop_InputProfilePath_String, "{caxe}/input/caxe_controller_profile.json");
	VRProperties()->SetBoolProperty(props, Prop_WillDriftInYaw_Bool, true);


	serial::Timeout timeout = serial::Timeout::simpleTimeout(TIMEOUT);
	serial_con.setPort(PORT);
	serial_con.setBaudrate(BAUD_RATE);
	serial_con.setTimeout(timeout);
	serial_con.open();

	poseUpdateThread = std::thread(&ControllerDriver::PoseUpdateThread, this);

	if (serial_con.isOpen()) {
		VRDriverLog()->Log("Successfully opened serial port");
	}
	else {
		VRDriverLog()->Log("Failed to open serial port");
		return VRInitError_Driver_Failed;
	}

	return VRInitError_None;
}

DriverPose_t ControllerDriver::GetPose()
{
	DriverPose_t pose = { 0 };
	HmdQuaternion_t orientation; 
	float roll, pitch, yaw;

	pose.qWorldFromDriverRotation.w = 1.f;
	pose.qDriverFromHeadRotation.w = 1.f;

	std::string measurements = serial_con.readline();
	sscanf(measurements.c_str(), "%f,%f,%f", &roll, &pitch, &yaw);

	orientation = HmdQuaternion_FromEulerAngles(roll, pitch, yaw);
	pose.qRotation = orientation;
	pose.vecPosition[1] = 1.0f;
	pose.vecPosition[2] = -1.0f;

	VRDriverLog()->Log(measurements.c_str());
	
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

	serial_con.close();

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
		VRDriverLog()->Log("Orientation updated");

		// Update our pose every five milliseconds.
		// In reality, you should update the pose whenever you have new data from your device.
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}

HmdQuaternion_t ControllerDriver::HmdQuaternion_FromEulerAngles(double roll, double pitch, double yaw) {
	double cr = cos(roll * 0.5);
	double sr = sin(roll * 0.5);
	double cp = cos(pitch * 0.5);
	double sp = sin(pitch * 0.5);
	double cy = cos(yaw * 0.5);
	double sy = sin(yaw * 0.5);

	vr::HmdQuaternion_t q;
	q.w = cr * cp * cy + sr * sp * sy;
	q.x = cr * sp * cy + sr * cp * sy;
	q.y = cr * cp * sy - sr * sp * cy;
	q.z = sr * cp * cy - cr * sp * sy;

	return q;
}
