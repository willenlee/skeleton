#!/usr/bin/python -u

import sys
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
from obmc.dbuslib.bindings import get_dbus, DbusProperties, DbusObjectManager
import time

class OCSLOCK_NAME(object):
	PRU_CHARDEV = 0
	PRU_SEQNUM = 1
	TELEMETRY_DAEMON = 2
	OCSGPIOACCESS = 3
	I2C0_CHARDEV = 4
	I2C1_CHARDEV = 5
	PRU_PERSIST = 6
	OCSLOG_SHM = 7
	NVDIMM_DAEMON = 8
	USR_ACCNT = 9
	NET_CONFIG = 10
	OCSFILE_WRITE = 11
	I2C16_CHARDEV = 12
	I2C17_CHARDEV = 13
	I2C18_CHARDEV = 14
	I2C19_CHARDEV = 15
	NUM_OCSLOCKS = 16
	

OCSLOCK_DBUS="org.openbmc.Ocslock"
OCSLOCK_OBJ_ROOT="/org/openbmc/ocslock/"
OCSLOCK_INTF="org.openbmc.Control"

def ocs_lock(ocslockid):
	if ocslockid>=OCSLOCK_NAME.NUM_OCSLOCKS:
		print "ocslockid exceeds NUM_OCSLOCKS:"+str(ocslockid)
		return

	ocs_bus = get_dbus()
	ocs_obj= ocs_bus.get_object("org.openbmc.Ocslock", "/org/openbmc/ocslock/"+str(ocslockid))
	ocs_interface = dbus.Interface(ocs_obj, "org.openbmc.Control")
	while ocs_interface.ocs_lock() < 0:
		time.sleep(1)
	
def ocs_unlock(ocslockid):
	if ocslockid>=OCSLOCK_NAME.NUM_OCSLOCKS:
		print "ocslockid exceeds NUM_OCSLOCKS:"+str(ocslockid)
		return

	ocs_bus = get_dbus()
	ocs_obj= ocs_bus.get_object("org.openbmc.Ocslock", "/org/openbmc/ocslock/"+str(ocslockid))
	ocs_interface = dbus.Interface(ocs_obj, "org.openbmc.Control")
	while ocs_interface.ocs_unlock() < 0:
		time.sleep(1)
