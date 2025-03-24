#pragma once

#include <openvr_driver.h>
#include <serial/serial.h>
#include <thread>
#include <atomic>

using namespace vr;

const std::string PORT = "COM3";
const uint32_t BAUD_RATE = 115200;
const uint32_t TIMEOUT = 1000;

/**
* This class controls the behavior of the controller.
**/
class ControllerDriver : public ITrackedDeviceServerDriver {
public:

	/**
	* Initialization of the controller. Setting up handles to inform
	* OpenVR when the controller state changes.
	**/
	EVRInitError Activate(uint32_t unObjectId) override;
	void Deactivate() override;
	void EnterStandby() override;
	void* GetComponent(const char* pchComponentNameAndVersion) override;
	void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) override;
	DriverPose_t GetPose() override;

	/**
	* Retrieve controller orientation and update OpenVR if anything has changed.
	**/
	void RunFrame();
	void PoseUpdateThread();
	HmdQuaternion_t HmdQuaternion_FromEulerAngles(double roll, double pitch, double yaw);

private:
	uint32_t driverId;
	std::atomic<bool> isActive;
	std::thread poseUpdateThread;
	serial::Serial serial_con;
};