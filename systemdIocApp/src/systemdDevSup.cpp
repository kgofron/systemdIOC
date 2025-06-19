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

    // Get the unit path
    ret = sd_bus_call_method(bus, "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
                            "org.freedesktop.systemd1.Manager", "GetUnit",
                            &error, &reply, "s", "serval.service");
    if (ret < 0) {
        recGblSetSevr(psi, COMM_ALARM, INVALID_ALARM);
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return -1;
    }

    const char* unit_path;
    ret = sd_bus_message_read(reply, "o", &unit_path);
    sd_bus_message_unref(reply);
    sd_bus_error_free(&error);
    if (ret < 0) {
        recGblSetSevr(psi, COMM_ALARM, INVALID_ALARM);
        sd_bus_unref(bus);
        return -1;
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
    if (ret < 0) {
        recGblSetSevr(psi, COMM_ALARM, INVALID_ALARM);
        sd_bus_unref(bus);
        return -1;
    }

    // Handle empty or null active_state
    if (!active_state || strlen(active_state) == 0) {
        strncpy(psi->val, "unknown", sizeof(psi->val) - 1);
        psi->val[sizeof(psi->val) - 1] = '\0';
        sd_bus_unref(bus);
        return 0;
    }

    // Handle whitespace-only active_state
    int is_whitespace_only = 1;
    for (int i = 0; active_state[i] != '\0'; i++) {
        if (active_state[i] != ' ' && active_state[i] != '\t' && active_state[i] != '\n' && active_state[i] != '\r' && active_state[i] != '`') {
            is_whitespace_only = 0;
            break;
        }
    }
    if (is_whitespace_only) {
        strncpy(psi->val, "unknown", sizeof(psi->val) - 1);
        psi->val[sizeof(psi->val) - 1] = '\0';
        sd_bus_unref(bus);
        return 0;
    }

    // Get the unit's SubState for more detailed status
    sd_bus_error substate_error = SD_BUS_ERROR_NULL;
    sd_bus_message* substate_reply = nullptr;
    ret = sd_bus_call_method(bus, "org.freedesktop.systemd1", unit_path,
                            "org.freedesktop.DBus.Properties", "Get",
                            &substate_error, &substate_reply, "ss",
                            "org.freedesktop.systemd1.Unit", "SubState");
    if (ret < 0) {
        // Continue with just ActiveState if SubState fails
        sd_bus_error_free(&substate_error);
    } else {
        const char* sub_state;
        ret = sd_bus_message_read(substate_reply, "v", "s", &sub_state);
        sd_bus_message_unref(substate_reply);
        sd_bus_error_free(&substate_error);
        if (ret >= 0 && sub_state && strlen(sub_state) > 0) {
            // Apply the same status mapping logic to SubState
            if (strcmp(sub_state, "running") == 0) {
                strncpy(psi->val, "running", sizeof(psi->val) - 1);
                psi->val[sizeof(psi->val) - 1] = '\0';
            } else if (strcmp(sub_state, "failed") == 0) {
                strncpy(psi->val, "stopped", sizeof(psi->val) - 1);
                psi->val[sizeof(psi->val) - 1] = '\0';
            } else if (strcmp(sub_state, "dead") == 0) {
                strncpy(psi->val, "stopped", sizeof(psi->val) - 1);
                psi->val[sizeof(psi->val) - 1] = '\0';
            } else {
                // Use SubState as-is for other states
                strncpy(psi->val, sub_state, sizeof(psi->val) - 1);
                psi->val[sizeof(psi->val) - 1] = '\0';
            }
            sd_bus_unref(bus);
            return 0;
        }
    }

    // Fallback to ActiveState
    if (strcmp(active_state, "failed") == 0) {
        // Check if it's actually just stopped by looking at the exit code
        // For now, we'll assume it's stopped if it's "failed"
        strncpy(psi->val, "stopped", sizeof(psi->val) - 1);
        psi->val[sizeof(psi->val) - 1] = '\0';
    } else if (strcmp(active_state, "active") == 0) {
        strncpy(psi->val, "running", sizeof(psi->val) - 1);
        psi->val[sizeof(psi->val) - 1] = '\0';
    } else if (strcmp(active_state, "inactive") == 0) {
        strncpy(psi->val, "stopped", sizeof(psi->val) - 1);
        psi->val[sizeof(psi->val) - 1] = '\0';
    } else if (strcmp(active_state, "deactivating") == 0) {
        strncpy(psi->val, "stopping", sizeof(psi->val) - 1);
        psi->val[sizeof(psi->val) - 1] = '\0';
    } else if (strcmp(active_state, "activating") == 0) {
        strncpy(psi->val, "starting", sizeof(psi->val) - 1);
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
epicsExportAddress(dset, devStringinServal); 