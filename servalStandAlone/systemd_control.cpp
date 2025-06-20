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
        if (!bus) {
            return "Unknown";
        }

        sd_bus_error error = SD_BUS_ERROR_NULL;
        sd_bus_message* reply = nullptr;
        int ret;

        // Use ListUnits to get all units and find our specific service
        ret = sd_bus_call_method(bus,
                               "org.freedesktop.systemd1",
                               "/org/freedesktop/systemd1",
                               "org.freedesktop.systemd1.Manager",
                               "ListUnits",
                               &error,
                               &reply,
                               "");

        if (ret < 0) {
            std::cerr << "Failed to list units: " << error.message << std::endl;
            sd_bus_error_free(&error);
            return "Unknown";
        }

        if (!reply) {
            return "Unknown";
        }

        // Parse the response to find our service
        ret = sd_bus_message_enter_container(reply, 'a', "(ssssssouso)");
        if (ret < 0) {
            sd_bus_message_unref(reply);
            return "Unknown";
        }

        std::string result = "not-found";
        
        while ((ret = sd_bus_message_enter_container(reply, 'r', "ssssssouso")) > 0) {
            const char *name = nullptr, *description = nullptr, *load_state = nullptr, *active_state = nullptr, *sub_state = nullptr, *following = nullptr, *unit_path = nullptr, *job_type = nullptr, *job_path = nullptr;
            uint32_t job_id = 0;

            ret = sd_bus_message_read(reply, "ssssssouso", &name, &description, &load_state, &active_state, &sub_state, &following, &unit_path, &job_id, &job_type, &job_path);
            if (ret < 0) {
                sd_bus_message_exit_container(reply);
                break;
            }

            if (name && std::string(name) == serviceName && active_state) {
                result = active_state;
                sd_bus_message_exit_container(reply);
                break;
            }

            sd_bus_message_exit_container(reply);
        }

        sd_bus_message_exit_container(reply);
        sd_bus_message_unref(reply);
        
        // If we found the service in the list, return its state
        if (result != "not-found") {
            return result;
        }
        
        // If we didn't find the service in the list, it might be in a failed state
        // Try to get it using GetUnit
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
            // Service is not loaded or doesn't exist
            return "not-found";
        }

        if (!reply) {
            return "Unknown";
        }

        const char* unit_path = nullptr;
        ret = sd_bus_message_read(reply, "o", &unit_path);
        sd_bus_message_unref(reply);

        if (ret < 0 || !unit_path) {
            return "Unknown";
        }

        // Get the ActiveState from the unit
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

        if (!reply) {
            return "Unknown";
        }

        const char* state = nullptr;
        ret = sd_bus_message_read(reply, "v", "s", &state);
        sd_bus_message_unref(reply);

        if (ret < 0 || !state) {
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

        // Wait for 3 seconds
        std::cout << "Waiting for 3 seconds..." << std::endl;
        sleep(3);

        std::cout << "Stopping serval.service..." << std::endl;
        if (controller.controlService(serviceName, "StopUnit")) {
            std::cout << "Service stopped successfully" << std::endl;
            std::cout << "Final service status: " << controller.getServiceStatus(serviceName) << std::endl;
        }

        // Infinite loop to monitor service status every 1 second
        std::cout << "Starting continuous monitoring of " << serviceName << " (Ctrl+C to stop)..." << std::endl;
        while (true) {
            std::string status = controller.getServiceStatus(serviceName);
            std::cout << "[" << time(nullptr) << "] Service status: " << status << std::endl;
            sleep(1);
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 