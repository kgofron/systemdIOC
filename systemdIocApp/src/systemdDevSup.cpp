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
    ret = sd_bus_call_method(bus,
                           "org.freedesktop.systemd1",
                           "/org/freedesktop/systemd1",
                           "org.freedesktop.systemd1.Manager",
                           "GetUnit",
                           &error,
                           &reply,
                           "s",
                           "serval.service");

    if (ret < 0) {
        recGblSetSevr(psi, COMM_ALARM, INVALID_ALARM);
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return -1;
    }

    const char* unit_path;
    ret = sd_bus_message_read(reply, "o", &unit_path);
    sd_bus_message_unref(reply);

    if (ret < 0) {
        recGblSetSevr(psi, COMM_ALARM, INVALID_ALARM);
        sd_bus_unref(bus);
        return -1;
    }

    // Get the unit's ActiveState
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
        recGblSetSevr(psi, COMM_ALARM, INVALID_ALARM);
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return -1;
    }

    const char* state;
    ret = sd_bus_message_read(reply, "v", "s", &state);
    sd_bus_message_unref(reply);

    if (ret < 0) {
        recGblSetSevr(psi, COMM_ALARM, INVALID_ALARM);
        sd_bus_unref(bus);
        return -1;
    }

    // Copy the state to the record's value
    strncpy(psi->val, state, sizeof(psi->val) - 1);
    psi->val[sizeof(psi->val) - 1] = '\0';

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