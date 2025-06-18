#include <epicsExport.h>
#include <devSup.h>
#include <recGbl.h>
#include <boRecord.h>
#include <alarm.h>  // For COMM_ALARM and INVALID_ALARM
#include <systemd/sd-bus.h>
#include <string>
#include <iostream>

static long init_record(void* prec) {
    boRecord *pbo = (boRecord *)prec;
    // Initialize any record-specific fields here if needed
    pbo->udf = FALSE;  // Mark the record as defined
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
    init_record,
    NULL,
    write_bo
};

epicsExportAddress(dset, devBoServal); 