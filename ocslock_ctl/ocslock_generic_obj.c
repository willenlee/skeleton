#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <systemd/sd-bus.h>
#include "../libocs_semlock/ocslock.h"


#define OCSLOCK_DBUS_OBJ_ROOT "/org/openbmc/ocslock"
#define OCSLOCK_INTFERFACE "org.openbmc.Control"
#define OCSLOCK_DBUS "org.openbmc.Ocslock"

static int g_ocslock_flag[NUM_OCSLOCKS];

static char *
get_ocslock_name(const char *dbus_path)
{
	char *fan_name = NULL;

	fan_name = strrchr(dbus_path, '/');
	if(fan_name) {
		fan_name++;
	}
	return fan_name;

}

static int
ocslock_function_router(sd_bus_message *msg, void *user_data,
			sd_bus_error *ret_error)
{
	/* Generic error reporter. */
	int rc = -1;
	/* Extract the fan name from the full dbus path */
	const char *ocslock_path = sd_bus_message_get_path(msg);

	if(ocslock_path == NULL) {
		fprintf(stderr, "Error. OCSLOCK path is empty");
		return sd_bus_reply_method_return(msg, "i", rc);
	}

	char *ocslock_name = get_ocslock_name(ocslock_path);
	if(ocslock_name == NULL) {
		fprintf(stderr, "Invalid ocslock_name name for path\n");
		return sd_bus_reply_method_return(msg, "i", rc);
	}

	int ocslock_id = atoi(ocslock_name);
	if (ocslock_id >= NUM_OCSLOCKS) {
		fprintf(stderr, "Invalid ocslock_id  for path\n");
		return sd_bus_reply_method_return(msg, "i", rc);
	}

	/* Now that we have the FAN name, get the Operation. */
	const char *ocslock_function = sd_bus_message_get_member(msg);
	if(ocslock_function == NULL) {
		fprintf(stderr, "Null Ocslock function specificed: ocslock_function\n");
		return sd_bus_reply_method_return(msg, "i", rc);
	}

	if(strcmp(ocslock_function, "ocs_lock") == 0) {
		if (g_ocslock_flag[ocslock_id]!=0)
			return sd_bus_reply_method_return(msg, "i", -1);

		g_ocslock_flag[ocslock_id] = 1;
		rc = ocs_lock(ocslock_id);
		if (rc < 0)
			g_ocslock_flag[ocslock_id] = 0;
	} else if(strcmp(ocslock_function, "ocs_unlock") == 0) {
		rc = ocs_unlock(ocslock_id);
		if (rc >= 0)
			g_ocslock_flag[ocslock_id] = 0;
	} else
		fprintf(stderr,"Invalid OCSLOCK function:[%s]\n",ocslock_function);

	return sd_bus_reply_method_return(msg, "i", rc);
}

/*
 * -----------------------------------------------
 * Dbus Services offered by this PWM controller
 * -----------------------------------------------
 */
static const sd_bus_vtable ocslock_control_vtable[] = {
	SD_BUS_VTABLE_START(0),
	SD_BUS_METHOD("ocs_lock", "", "i", &ocslock_function_router, SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("ocs_unlock", "", "i", &ocslock_function_router, SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_VTABLE_END,
};

static int
register_ocslock_services(sd_bus *bus_type, sd_bus_slot *ocslock_slot, char *ocslock_object)
{
	int rc = -1;
	/* Install the object */
	rc = sd_bus_add_object_vtable(bus_type,
				      &ocslock_slot,
				      ocslock_object, /* object path */
				      OCSLOCK_INTFERFACE, /* interface name */
				      ocslock_control_vtable,
				      NULL);
	if(rc < 0) {
		fprintf(stderr, "Failed to add object to dbus: %s\n", strerror(-rc));
		return rc;
	}

	rc = sd_bus_emit_object_added(bus_type, ocslock_object);
	if(rc < 0) {
		fprintf(stderr, "Failed to emit InterfacesAdded "
			"signal: %s\n", strerror(-rc));
		return rc;
	}

	return rc;
}

int
start_ocslock_services()
{
	const char *ocslock_dbus_root = OCSLOCK_DBUS_OBJ_ROOT;
	/* Generic error reporter. */
	int rc = -1;

	/* Bus and slot where we are offering the ocslock dbus service. */
	sd_bus *bus_type = NULL;
	sd_bus_slot *ocslock_slot = NULL;

	/* Get a hook onto system bus. */
	rc = sd_bus_open_system(&bus_type);
	if(rc < 0) {
		fprintf(stderr,"Error opening system bus.\n");
		return rc;
	}

	/* Install a freedesktop object manager */
	rc = sd_bus_add_object_manager(bus_type, NULL, ocslock_dbus_root);
	if(rc < 0) {
		fprintf(stderr, "Failed to add object to dbus: %s\n",
			strerror(-rc));
		sd_bus_slot_unref(ocslock_slot);
		sd_bus_flush_close_unref(bus_type);
		return rc;
	}

	int i;
	char ocs_objpath[100];
	for (i = 0; i<NUM_OCSLOCKS; i++) {
		sprintf(ocs_objpath, "%s/%d", OCSLOCK_DBUS_OBJ_ROOT, i);
		register_ocslock_services(bus_type, ocslock_slot, ocs_objpath);
		ocslock_init(i); //ocslock init
		g_ocslock_flag[i] = 0;
	}

	printf("start_ocslock_services Starting!!!\n");

	/* If we had success in adding the providers, request for a bus name. */
	if(rc >= 0) {
		/* Take one in OpenBmc */
		rc = sd_bus_request_name(bus_type, OCSLOCK_DBUS, 0);
		if(rc < 0) {
			fprintf(stderr, "Failed to acquire service name: %s\n", strerror(-rc));
		} else {
			for(;;) {
				/* Process requests */
				rc = sd_bus_process(bus_type, NULL);
				if(rc < 0) {
					fprintf(stderr, "Failed to process bus: %s\n", strerror(-rc));
					break;
				}

				if(rc > 0) {
					continue;
				}

				rc = sd_bus_wait(bus_type, (uint64_t) - 1);
				if(rc < 0) {
					fprintf(stderr, "Failed to wait on bus: %s\n", strerror(-rc));
					break;
				}
			}
		}
	}

	sd_bus_slot_unref(ocslock_slot);
	sd_bus_flush_close_unref(bus_type);
	return rc;
}

int
main(void)
{
	int rc = 0;

	/* This call is not supposed to return. If it does, then an error */
	rc = start_ocslock_services();
	if(rc < 0) {
		fprintf(stderr, "Error starting OCSLock Services. Exiting");
	}
	return rc;
}

