#include <systemd/sd-bus.h>
#include <iostream>
#include <string>
#include <memory>
#include <unistd.h>

class SystemdController {
private:
    sd_bus* bus = nullptr;

public:
    SystemdController() {
        // Using system bus to control system-wide services
        // Note: Access control should be managed through polkit rules
        int ret = sd_bus_default_system(&bus);
        if (ret < 0) {
            throw std::runtime_error("Failed to connect to system bus: " + std::string(strerror(-ret)));
        }

        // Drop privileges after getting the bus connection
        if (setuid(getuid()) != 0) {
            throw std::runtime_error("Failed to drop privileges");
        }
    }

    ~SystemdController() {
        if (bus) {
            sd_bus_unref(bus);
        }
    }

    bool controlService(const std::string& serviceName, const std::string& action) {
        sd_bus_error error = SD_BUS_ERROR_NULL;
        sd_bus_message* reply = nullptr;
        int ret;

        ret = sd_bus_call_method(bus,
                               "org.freedesktop.systemd1",
                               "/org/freedesktop/systemd1",
                               "org.freedesktop.systemd1.Manager",
                               action.c_str(),
                               &error,
                               &reply,
                               "ss",
                               serviceName.c_str(),
                               "replace");

        if (ret < 0) {
            std::cerr << "Failed to " << (action == "StartUnit" ? "start" : "stop") 
                     << " service: " << error.message << std::endl;
            sd_bus_error_free(&error);
            return false;
        }

        sd_bus_message_unref(reply);
        return true;
    }

    std::string getServiceStatus(const std::string& serviceName) {
        sd_bus_error error = SD_BUS_ERROR_NULL;
        sd_bus_message* reply = nullptr;
        int ret;

        // First, get the unit path
        ret = sd_bus_call_method(bus,
                               "org.freedesktop.systemd1",
                               "/org/freedesktop/systemd1",
                               "org.freedesktop.systemd1.Manager",
                               "GetUnit",
                               &error,
                               &reply,
                               "s",
                               serviceName.c_str());

        if (ret < 0) {
            std::cerr << "Failed to get unit path: " << error.message << std::endl;
            sd_bus_error_free(&error);
            return "Unknown";
        }

        const char* unit_path;
        ret = sd_bus_message_read(reply, "o", &unit_path);
        sd_bus_message_unref(reply);

        if (ret < 0) {
            return "Unknown";
        }

        // Now get the unit's ActiveState
        ret = sd_bus_call_method(bus,
                               "org.freedesktop.systemd1",
                               unit_path,
                               "org.freedesktop.DBus.Properties",
                               "Get",
                               &error,
                               &reply,
                               "ss",
                               "org.freedesktop.systemd1.Unit",
                               "ActiveState");

        if (ret < 0) {
            std::cerr << "Failed to get service status: " << error.message << std::endl;
            sd_bus_error_free(&error);
            return "Unknown";
        }

        const char* state;
        ret = sd_bus_message_read(reply, "v", "s", &state);
        sd_bus_message_unref(reply);

        if (ret < 0) {
            return "Unknown";
        }

        return state;
    }
};

int main() {
    try {
        SystemdController controller;
        const std::string serviceName = "serval.service";

        // Check initial status
        std::cout << "Initial service status: " << controller.getServiceStatus(serviceName) << std::endl;

        // Equivalent to: systemctl reset-failed serval.service
        // controller.controlService(serviceName, "ResetFailedUnit");

        std::cout << "Starting serval.service..." << std::endl;
        if (controller.controlService(serviceName, "StartUnit")) {
            std::cout << "Service started successfully" << std::endl;
            std::cout << "Current service status: " << controller.getServiceStatus(serviceName) << std::endl;
        }

        // Wait for 10 seconds
        std::cout << "Waiting for 10 seconds..." << std::endl;
        sleep(10);

        std::cout << "Stopping serval.service..." << std::endl;
        if (controller.controlService(serviceName, "StopUnit")) {
            std::cout << "Service stopped successfully" << std::endl;
            std::cout << "Final service status: " << controller.getServiceStatus(serviceName) << std::endl;
        }

        // Wait for 5 seconds after stopping
        std::cout << "Waiting for 5 seconds after stopping..." << std::endl;
        sleep(5);
        std::cout << "Service status after 5 seconds: " << controller.getServiceStatus(serviceName) << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 