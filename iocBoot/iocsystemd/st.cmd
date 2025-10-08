#!../../bin/linux-x86_64/systemdIoc

## You may have to change systemdIoc to something else
## everywhere it appears in this file

< envPaths

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/systemdIoc.dbd"
systemdIoc_registerRecordDeviceDriver pdbbase

## Load record instances for multiple systemd services
## Each dbLoadRecords creates a new set of PVs for a different service
## P=PV prefix, R=record suffix, SERVICE=systemd service name

## Example 1: Control serval.service
dbLoadRecords("db/systemd.db", "P=serval:,R=service:,SERVICE=serval.service")

## Example 2: Control another service (uncomment to enable)
#dbLoadRecords("db/systemd.db", "P=emulator:,R=service:,SERVICE=emulator.service")

## Example 3: Control ssh service (uncomment to enable)
#dbLoadRecords("db/systemd.db", "P=system:,R=ssh:,SERVICE=ssh.service")

## Example 4: Control apache2 service (uncomment to enable)
#dbLoadRecords("db/systemd.db", "P=web:,R=server:,SERVICE=apache2.service")

cd "${TOP}/iocBoot/${IOC}"
iocInit

## Start any sequence programs
#seq sncExample, "user=systemdIoc" 
