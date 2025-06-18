TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS BELOW HERE

# Use typed rset structure (see 3.16.1 release notes)
USR_CPPFLAGS += -DUSE_TYPED_RSET

# Build an IOC support library
LIBRARY_IOC += systemdIocSupport

# Make the support library shared and disable static library
SHARED_LIBRARIES = YES
STATIC_BUILD = NO

USR_LDFLAGS += -lsystemd

LIB_SYS_LIBS += systemd

# Link locally-provided code into the support library,
# rather than directly into the IOC application, that
# causes problems on Windows DLL builds
systemdIocSupport_SRCS += systemdDevSup.cpp
systemdIocSupport_SRCS += devsystemdIocVersion.c

# Add systemd to the support library's dependencies
systemdIocSupport_LIBS += $(EPICS_BASE_IOC_LIBS)

# Auto-generate a header file containing a version string.
# Version comes from the VCS if available, else date+time.
GENVERSION = systemdIocVersion.h
# Macro name
GENVERSIONMACRO = systemdIocVERSION

# Build the IOC application
PROD_IOC = systemdIoc

# systemdIoc.dbd will be created and installed
DBD += systemdIoc.dbd

# systemdIoc.dbd will include these files:
systemdIoc_DBD += base.dbd
systemdIoc_DBD += devsystemdIocVersion.dbd
systemdIoc_DBD += serval.dbd

# systemdIoc_registerRecordDeviceDriver.cpp derives from systemdIoc.dbd
systemdIoc_SRCS += systemdIoc_registerRecordDeviceDriver.cpp

# Build the main IOC entry point where needed
systemdIoc_SRCS_DEFAULT += systemdIocMain.cpp
systemdIoc_SRCS_vxWorks += -nil-

# Link in the code from our support library
systemdIoc_LIBS += systemdIocSupport

# Link QSRV (pvAccess Server) if available
ifdef EPICS_QSRV_MAJOR_VERSION
    systemdIoc_LIBS += qsrv
    systemdIoc_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)
    systemdIoc_DBD += PVAServerRegister.dbd
    systemdIoc_DBD += qsrv.dbd
endif

# Finally link IOC to the EPICS Base libraries
systemdIoc_LIBS += $(EPICS_BASE_IOC_LIBS)

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD EXTRA GNUMAKE RULES BELOW HERE

# Explicit dependency needed for generated header file
devsystemdIocVersion$(DEP): $(COMMON_DIR)/$(GENVERSION)
