#!../../bin/linux-x86_64/systemdIoc

## You may have to change systemdIoc to something else
## everywhere it appears in this file

< envPaths

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/systemdIoc.dbd"
systemdIoc_registerRecordDeviceDriver pdbbase

## Set the service name to control (default: serval.service)
## Change this to control a different systemd service
setServiceName("serval.service")

## Load record instances
## P=prefix, R=record suffix, SERVICE=service name for descriptions
dbLoadRecords("db/systemd.db", "P=serval:,R=service:,SERVICE=serval.service")

cd "${TOP}/iocBoot/${IOC}"
iocInit

## Start any sequence programs
#seq sncExample, "user=systemdIoc" 
