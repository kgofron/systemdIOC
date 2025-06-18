#!../../bin/linux-x86_64/systemdIoc

## You may have to change systemdIoc to something else
## everywhere it appears in this file

< envPaths

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/systemdIoc.dbd"
systemdIoc_registerRecordDeviceDriver pdbbase

## Load record instances
dbLoadRecords("db/systemd.db", "P=serval:,R=service:")

cd "${TOP}/iocBoot/${IOC}"
iocInit

## Start any sequence programs
#seq sncExample, "user=systemdIoc" 
