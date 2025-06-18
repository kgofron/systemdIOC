#include <epicsExport.h>
#include <devSup.h>
#include <recGbl.h>
#include <boRecord.h>
#include <stringinRecord.h>
#include <alarm.h>  // For COMM_ALARM and INVALID_ALARM
#include <systemd/sd-bus.h>
#include <string>
#include <iostream>

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
    sd_bus* bus = nullptr;
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
        printf("GetUnit error: %s\n", error.message ? error.message : strerror(-ret));
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
        printf("Read unit path error\n");
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
        printf("Get ActiveState error: %s\n", state_error.message ? state_error.message : strerror(-ret));
        recGblSetSevr(psi, COMM_ALARM, INVALID_ALARM);
        sd_bus_error_free(&state_error);
        sd_bus_unref(bus);
        return -1;
    }

    const char* state;
    ret = sd_bus_message_read(state_reply, "v", "s", &state);
    sd_bus_message_unref(state_reply);
    sd_bus_error_free(&state_error);
    if (ret < 0) {
        printf("Read state value error\n");
        recGblSetSevr(psi, COMM_ALARM, INVALID_ALARM);
        sd_bus_unref(bus);
        return -1;
    }

    if (state && strlen(state) > 0) {
        strncpy(psi->val, state, sizeof(psi->val) - 1);
        psi->val[sizeof(psi->val) - 1] = '\0';
    } else {
        strncpy(psi->val, "unknown", sizeof(psi->val) - 1);
        psi->val[sizeof(psi->val) - 1] = '\0';
    }

    sd_bus_unref(bus);
    return 0;
}

static long write_bo(void* prec) {
    boRecord *pbo = (boRecord *)prec;
    sd_bus* bus = nullptr;
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