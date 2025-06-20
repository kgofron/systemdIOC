#include <systemd/sd-bus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <service_name>\n", argv[0]);
        return 1;
    }

    const char *service_name = argv[1];
    sd_bus *bus = NULL;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    char *active_state = NULL;
    int r;

    // Connect to the system bus
    r = sd_bus_default_system(&bus);
    if (r < 0) {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        return 1;
    }

    // Construct the D-Bus object path for the service
    char service_path[256];
    snprintf(service_path, sizeof(service_path), "/org/freedesktop/systemd1/unit/%s.service", service_name);

    // Get the "ActiveState" property of the service
    r = sd_bus_get_property_string(
        bus,                            // bus
        "org.freedesktop.systemd1",     // destination
        service_path,                   // path
        "org.freedesktop.systemd1.Unit",// interface
        "ActiveState",                  // member (property name)
        &error,                         // object to return error in
        &active_state);                 // return value on success

    if (r < 0) {
        fprintf(stderr, "Failed to get service state: %s\n", error.message);
        sd_bus_error_free(&error);
        goto finish;
    }

    printf("Service '%s' ActiveState: %s\n", service_name, active_state);

finish:
    free(active_state);
    sd_bus_error_free(&error);
    sd_bus_unref(bus);

    return 0;
}

