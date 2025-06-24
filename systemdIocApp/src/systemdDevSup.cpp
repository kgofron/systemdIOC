#include <epicsExport.h>
#include <devSup.h>
#include <recGbl.h>
#include <boRecord.h>
#include <stringinRecord.h>
#include <alarm.h>  // For COMM_ALARM and INVALID_ALARM
#include <systemd/sd-bus.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <errno.h>

static long init_record_bo(void* prec) {
    boRecord *pbo = (boRecord *)prec;
    pbo->udf = FALSE;
    return 0;
}

static long init_record_stringin(void* prec) {
    stringinRecord *psi = (stringinRecord *)prec;
    psi->udf = FALSE;
    return 0;
}

static long read_stringin(void* prec) {
    stringinRecord *psi = (stringinRecord *)prec;
    
    // Drop privileges to avoid password prompts
    uid_t current_uid = getuid();
    uid_t effective_uid = geteuid();
    
    if (effective_uid != current_uid) {
        if (seteuid(current_uid) != 0) {
            recGblSetSevr(psi, COMM_ALARM, INVALID_ALARM);
            return -1;
        }
    }
    
    sd_bus* bus = nullptr;
    // Connect to system bus for systemd services
    int ret = sd_bus_default_system(&bus);
    if (ret < 0) {
        recGblSetSevr(psi, COMM_ALARM, INVALID_ALARM);
        return -1;
    }

    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;

    // Use ListUnits to get all units and find our specific service
    ret = sd_bus_call_method(bus, "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
                            "org.freedesktop.systemd1.Manager", "ListUnits",
                            &error, &reply, "");
    if (ret < 0) {
        recGblSetSevr(psi, COMM_ALARM, INVALID_ALARM);
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return -1;
    }

    if (!reply) {
        recGblSetSevr(psi, COMM_ALARM, INVALID_ALARM);
        sd_bus_unref(bus);
        return -1;
    }

    // Parse the response to find our service
    ret = sd_bus_message_enter_container(reply, 'a', "(ssssssouso)");
    if (ret < 0) {
        sd_bus_message_unref(reply);
        recGblSetSevr(psi, COMM_ALARM, INVALID_ALARM);
        sd_bus_unref(bus);
        return -1;
    }

    std::string result = "not-found";

    while ((ret = sd_bus_message_enter_container(reply, 'r', "ssssssouso")) > 0) {
        const char *name = nullptr, *description = nullptr, *load_state = nullptr,
                   *active_state = nullptr, *sub_state = nullptr, *following = nullptr,
                   *unit_path = nullptr, *job_type = nullptr, *job_path = nullptr;
        uint32_t job_id = 0;

        ret = sd_bus_message_read(reply, "ssssssouso", &name, &description, &load_state,
                                 &active_state, &sub_state, &following, &unit_path,
                                 &job_id, &job_type, &job_path);
        if (ret < 0) {
            sd_bus_message_exit_container(reply);
            break;
        }

        if (name && strcmp(name, "serval.service") == 0 && active_state) {
            result = active_state;
            sd_bus_message_exit_container(reply);
            break;
        }

        sd_bus_message_exit_container(reply);
    }

    sd_bus_message_exit_container(reply);
    sd_bus_message_unref(reply);

    // If we found the service in the list, use its state
    if (result != "not-found") {
        // Map the state to our simplified status
        if (strcmp(result.c_str(), "active") == 0) {
            strncpy(psi->val, "running", sizeof(psi->val) - 1);
            psi->val[sizeof(psi->val) - 1] = '\0';
        } else if (strcmp(result.c_str(), "inactive") == 0) {
            strncpy(psi->val, "stopped", sizeof(psi->val) - 1);
            psi->val[sizeof(psi->val) - 1] = '\0';
        } else if (strcmp(result.c_str(), "failed") == 0) {
            strncpy(psi->val, "stopped", sizeof(psi->val) - 1);
            psi->val[sizeof(psi->val) - 1] = '\0';
        } else if (strcmp(result.c_str(), "activating") == 0) {
            strncpy(psi->val, "starting", sizeof(psi->val) - 1);
            psi->val[sizeof(psi->val) - 1] = '\0';
        } else if (strcmp(result.c_str(), "deactivating") == 0) {
            strncpy(psi->val, "stopping", sizeof(psi->val) - 1);
            psi->val[sizeof(psi->val) - 1] = '\0';
        } else {
            // For any other state, use it as-is
            strncpy(psi->val, result.c_str(), sizeof(psi->val) - 1);
            psi->val[sizeof(psi->val) - 1] = '\0';
        }
        sd_bus_unref(bus);
        return 0;
    }

    // If we didn't find the service in the list, it might be in a failed state
    // Try to get it using GetUnit as fallback
    sd_bus_error fallback_error = SD_BUS_ERROR_NULL;
    sd_bus_message* fallback_reply = nullptr;
    ret = sd_bus_call_method(bus, "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
                            "org.freedesktop.systemd1.Manager", "GetUnit",
                            &fallback_error, &fallback_reply, "s", "serval.service");
    if (ret < 0) {
        // Service is not loaded or doesn't exist
        strncpy(psi->val, "not-found", sizeof(psi->val) - 1);
        psi->val[sizeof(psi->val) - 1] = '\0';
        sd_bus_error_free(&fallback_error);
        sd_bus_unref(bus);
        return 0;
    }

    if (!fallback_reply) {
        strncpy(psi->val, "unknown", sizeof(psi->val) - 1);
        psi->val[sizeof(psi->val) - 1] = '\0';
        sd_bus_unref(bus);
        return 0;
    }

    const char* unit_path;
    ret = sd_bus_message_read(fallback_reply, "o", &unit_path);
    sd_bus_message_unref(fallback_reply);
    sd_bus_error_free(&fallback_error);
    if (ret < 0 || !unit_path) {
        strncpy(psi->val, "unknown", sizeof(psi->val) - 1);
        psi->val[sizeof(psi->val) - 1] = '\0';
        sd_bus_unref(bus);
        return 0;
    }

    // Get the unit's ActiveState
    sd_bus_error state_error = SD_BUS_ERROR_NULL;
    sd_bus_message* state_reply = nullptr;
    ret = sd_bus_call_method(bus, "org.freedesktop.systemd1", unit_path,
                            "org.freedesktop.DBus.Properties", "Get",
                            &state_error, &state_reply, "ss",
                            "org.freedesktop.systemd1.Unit", "ActiveState");
    if (ret < 0) {
        recGblSetSevr(psi, COMM_ALARM, INVALID_ALARM);
        sd_bus_error_free(&state_error);
        sd_bus_unref(bus);
        return -1;
    }

    const char* active_state;
    ret = sd_bus_message_read(state_reply, "v", "s", &active_state);
    sd_bus_message_unref(state_reply);
    sd_bus_error_free(&state_error);
    if (ret < 0 || !active_state) {
        strncpy(psi->val, "unknown", sizeof(psi->val) - 1);
        psi->val[sizeof(psi->val) - 1] = '\0';
        sd_bus_unref(bus);
        return 0;
    }

    // Map the fallback state to our simplified status
    if (strcmp(active_state, "active") == 0) {
        strncpy(psi->val, "running", sizeof(psi->val) - 1);
        psi->val[sizeof(psi->val) - 1] = '\0';
    } else if (strcmp(active_state, "inactive") == 0) {
        strncpy(psi->val, "stopped", sizeof(psi->val) - 1);
        psi->val[sizeof(psi->val) - 1] = '\0';
    } else if (strcmp(active_state, "failed") == 0) {
        strncpy(psi->val, "stopped", sizeof(psi->val) - 1);
        psi->val[sizeof(psi->val) - 1] = '\0';
    } else if (strcmp(active_state, "activating") == 0) {
        strncpy(psi->val, "starting", sizeof(psi->val) - 1);
        psi->val[sizeof(psi->val) - 1] = '\0';
    } else if (strcmp(active_state, "deactivating") == 0) {
        strncpy(psi->val, "stopping", sizeof(psi->val) - 1);
        psi->val[sizeof(psi->val) - 1] = '\0';
    } else {
        // For any other state, use it as-is
        strncpy(psi->val, active_state, sizeof(psi->val) - 1);
        psi->val[sizeof(psi->val) - 1] = '\0';
    }

    sd_bus_unref(bus);
    return 0;
}

static long write_bo(void* prec) {
    boRecord *pbo = (boRecord *)prec;
    
    // Drop privileges to avoid password prompts
    uid_t current_uid = getuid();
    uid_t effective_uid = geteuid();
    
    if (effective_uid != current_uid) {
        if (seteuid(current_uid) != 0) {
            recGblSetSevr(pbo, COMM_ALARM, INVALID_ALARM);
            return -1;
        }
    }
    
    sd_bus* bus = nullptr;
    // Connect to system bus for systemd services
    int ret = sd_bus_default_system(&bus);
    if (ret < 0) {
        recGblSetSevr(pbo, COMM_ALARM, INVALID_ALARM);
        return -1;
    }

    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;

    // Check if this is the ResetFailed record or the Start/Stop record
    if (strstr(pbo->name, "ResetFailed") != nullptr) {
        // This is the ResetFailed record - perform ResetFailedUnit action
        ret = sd_bus_call_method(bus,
                               "org.freedesktop.systemd1",
                               "/org/freedesktop/systemd1",
                               "org.freedesktop.systemd1.Manager",
                               "ResetFailedUnit",
                               &error,
                               &reply,
                               "s",
                               "serval.service");
    } else {
        // This is the Start/Stop record - perform StartUnit or StopUnit action
        const char* action = pbo->val ? "StartUnit" : "StopUnit";
        ret = sd_bus_call_method(bus,
                               "org.freedesktop.systemd1",
                               "/org/freedesktop/systemd1",
                               "org.freedesktop.systemd1.Manager",
                               action,
                               &error,
                               &reply,
                               "ss",
                               "serval.service",
                               "replace");
    }

    if (ret < 0) {
        recGblSetSevr(pbo, COMM_ALARM, INVALID_ALARM);
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return -1;
    }

    sd_bus_message_unref(reply);
    sd_bus_unref(bus);
    return 0;
}

struct {
    long number;
    DEVSUPFUN report;
    DEVSUPFUN init;
    DEVSUPFUN init_record;
    DEVSUPFUN get_ioint_info;
    DEVSUPFUN write_bo;
} devBoServal = {
    5,
    NULL,
    NULL,
    init_record_bo,
    NULL,
    write_bo
};

struct {
    long number;
    DEVSUPFUN report;
    DEVSUPFUN init;
    DEVSUPFUN init_record;
    DEVSUPFUN get_ioint_info;
    DEVSUPFUN write_bo;
} devBoServalReset = {
    5,
    NULL,
    NULL,
    init_record_bo,
    NULL,
    write_bo
};

struct {
    long number;
    DEVSUPFUN report;
    DEVSUPFUN init;
    DEVSUPFUN init_record;
    DEVSUPFUN get_ioint_info;
    DEVSUPFUN read_stringin;
} devStringinServal = {
    5,
    NULL,
    NULL,
    init_record_stringin,
    NULL,
    read_stringin
};

epicsExportAddress(dset, devBoServal);
epicsExportAddress(dset, devBoServalReset);
epicsExportAddress(dset, devStringinServal);
