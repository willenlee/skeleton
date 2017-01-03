#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <systemd/sd-bus.h>

typedef enum {
	EM_PWM_IDX_0 = 0,
	EM_PWM_IDX_1,
	EM_PWM_IDX_2,
	EM_PWM_IDX_3,
	EM_PWM_IDX_4,
	EM_PWM_IDX_5,
	EM_PWM_IDX_6,
	EM_PWM_IDX_7,
} EM_PWM_IDX;

typedef enum {
	EM_PWM_CMD_EN = 0, //set pwm enable
	EM_PWM_CMD_FALLING, //set pwm duty falling
	EM_PWM_CMD_RISING, //set pwm duty rising
	EM_PWM_CMD_TYPE, //set pwm type (M/N/O)
} EM_PWM_NODE_CMD;

#define FAN_DBUS_OBJ_ROOT "/org/openbmc/control/fan"
#define FAN_DBUS_OBJ_SUBDIR "fan"
#define FAN_DBUS "org.openbmc.control.fan"
#define FAN_INTFERFACE "org.openbmc.Fan"

#define PWM_MAX_UNIT (255)
#define MAX_FAN_NUM (6)
#define SYS_PWM_PATH "/sys/devices/platform/ast_pwm_tacho.0/"

static EM_PWM_IDX g_fan_map_pwm_tab[] = {
	EM_PWM_IDX_0,  //fan0 map to pwm0
	EM_PWM_IDX_1,  //fan1 map to pwm1
	EM_PWM_IDX_2,  //fan2 map to pwm2
	EM_PWM_IDX_3,  //fan3 map to pwm3
	EM_PWM_IDX_4,  //fan4 map to pwm4
	EM_PWM_IDX_5,  //fan5 map to pwm5
};

static char g_pwm_cmd_map_sys_tab[][20] = {
	"_en",  //EM_PWM_CMD_EN --> "pwmX_en", X :mean pwm index eg: "pwm0_en"
	"_falling",  //EM_PWM_CMD_FALLING --> "pwmX_falling", X :mean pwm index eg: "pwm0_falling"
	"_rising",  //EM_PWM_CMD_RISING --> "pwmX_rising", X :mean pwm index eg: "pwm0_rising"
	"_type",  //EM_PWM_CMD_TYPE --> "pwmX_type", X :mean pwm index eg: "pwm0_type"
};

/*
 * ----------------------------------------------------------------
 * Router function for any FAN operations that come via dbus
 *----------------------------------------------------------------
 */
char *
get_fan_name(const char *dbus_path)
{
	char *fan_name = NULL;

	fan_name = strrchr(dbus_path, '/');
	if(fan_name) {
		fan_name++;
	}
	return fan_name;

}

int
get_fan_index(const char *fan_name)
{
	int fan_index;
	char format_s[20];
	sprintf(format_s, "%s%%d", FAN_DBUS_OBJ_SUBDIR);
	sscanf(fan_name, format_s, &fan_index);
	return fan_index;
}

static void
sys_pwm_write(EM_PWM_IDX pwm_idex, EM_PWM_NODE_CMD pwm_cmd, int write_value)
{
	char sys_pwm_path[100];
	char sys_cmd[100];
	int rc;

	if (write_value > PWM_MAX_UNIT)
		write_value = PWM_MAX_UNIT;
	else if (write_value < 0)
		write_value = 0;

	rc = sprintf(sys_pwm_path , "%spwm%d%s", SYS_PWM_PATH, pwm_idex, g_pwm_cmd_map_sys_tab[pwm_cmd]);
	sprintf(sys_cmd, "echo %d > %s", write_value, sys_pwm_path);
	system(sys_cmd);
}

static int
sys_pwm_read(EM_PWM_IDX pwm_idex, EM_PWM_NODE_CMD pwm_cmd)
{
	char sys_pwm_path[100];
	FILE *fp;
	int retValue = -1;
	char buf[100] = {0};
	int ret_len;

	sprintf(sys_pwm_path , "%spwm%d%s", SYS_PWM_PATH, pwm_idex, g_pwm_cmd_map_sys_tab[pwm_cmd]);

	fp = fopen(sys_pwm_path,"r");
	if(fp == NULL) {
		fprintf(stderr,"Error:[%s] opening:[%s]\n",strerror(errno),sys_pwm_path);
		return -1;
	}

	ret_len = fread(buf, 1, 100, fp);
	sscanf(buf, "%x", &retValue);

	fclose(fp);

	return retValue;
}

static int
pwm_fan_function_router(sd_bus_message *msg, void *user_data,
			sd_bus_error *ret_error)
{
	/* Generic error reporter. */
	int rc = -1;
	/* Extract the fan name from the full dbus path */
	const char *fan_path = sd_bus_message_get_path(msg);

	if(fan_path == NULL) {
		fprintf(stderr, "Error. FAN path is empty");
		return sd_bus_reply_method_return(msg, "i", rc);
	}

	char *fan_name = get_fan_name(fan_path);
	if(fan_name == NULL) {
		fprintf(stderr, "Invalid FAN name for path :[%s]\n",fan_path);
		return sd_bus_reply_method_return(msg, "i", rc);
	}

	/* Now that we have the FAN name, get the Operation. */
	const char *fan_function = sd_bus_message_get_member(msg);
	if(fan_function == NULL) {
		fprintf(stderr, "Null FAN function specificed for : [%s]\n",fan_name);
		return sd_bus_reply_method_return(msg, "i", rc);
	}

	int fan_index = get_fan_index(fan_name);
	int pwm_idex = g_fan_map_pwm_tab[fan_index];

	/* Route the user action to appropriate handlers. */
	if(strcmp(fan_function, "getValue") == 0) {
		//int pwm_falling =  sys_pwm_read(pwm_idex, EM_PWM_CMD_FALLING);
		int pwm_rising =  sys_pwm_read(pwm_idex, EM_PWM_CMD_RISING);
		return sd_bus_reply_method_return(msg, "i", pwm_rising);
	} else if(strcmp(fan_function, "setValue") == 0) {
		int fan_speed = 0;

		/* Extract values into 'ss' ( string, string) */
		rc = sd_bus_message_read(msg, "i", &fan_speed);
		sys_pwm_write(pwm_idex, EM_PWM_CMD_FALLING, 0x00);
		sys_pwm_write(pwm_idex, EM_PWM_CMD_RISING, fan_speed);
	} else {
		fprintf(stderr,"Invalid FAN function:[%s]\n",fan_function);
	}

	return sd_bus_reply_method_return(msg, "i", rc);
}

/*
 * -----------------------------------------------
 * Dbus Services offered by this PWM controller
 * -----------------------------------------------
 */
static const sd_bus_vtable fan_control_vtable[] = {
	SD_BUS_VTABLE_START(0),
	SD_BUS_METHOD("getValue", "", "i", &pwm_fan_function_router, SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("setValue", "i", "i", &pwm_fan_function_router, SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_VTABLE_END,
};

int
start_fan_services()
{
	static const char *fan_dbus_root = FAN_DBUS_OBJ_ROOT;
	/* Generic error reporter. */
	int rc = -1;
	int num_fans = 0;

	/* Bus and slot where we are offering the Fan dbus service. */
	sd_bus *bus_type = NULL;
	sd_bus_slot *fan_slot = NULL;

	/* Get a hook onto system bus. */
	rc = sd_bus_open_system(&bus_type);
	if(rc < 0) {
		fprintf(stderr,"Error opening system bus.\n");
		return rc;
	}

	/* Install a freedesktop object manager */
	rc = sd_bus_add_object_manager(bus_type, NULL, fan_dbus_root);
	if(rc < 0) {
		fprintf(stderr, "Failed to add object to dbus: %s\n",
			strerror(-rc));
		sd_bus_slot_unref(fan_slot);
		sd_bus_unref(bus_type);
		return rc;
	}

	/* Fully qualified Dbus object for a particular Fan */
	char fan_object[128] = {0};
	int len = 0;

	num_fans = 0;
	/* For each fan present, announce the service on dbus. */
	while(num_fans < MAX_FAN_NUM) {
		memset(fan_object, 0x0, sizeof(fan_object));
		len = snprintf(fan_object, sizeof(fan_object), "%s%s%s%d",
			       fan_dbus_root, "/" ,FAN_DBUS_OBJ_SUBDIR ,num_fans);

		if(len >= sizeof(fan_object)) {
			fprintf(stderr, "Error. FAN object is too long:[%d]\n",len);
			rc = -1;
			break;
		}

		/* Install the object */
		rc = sd_bus_add_object_vtable(bus_type,
					      &fan_slot,
					      fan_object, /* object path */
					      FAN_INTFERFACE, /* interface name */
					      fan_control_vtable,
					      NULL);

		if(rc < 0) {
			fprintf(stderr, "Failed to add object to dbus: %s\n", strerror(-rc));
			break;
		}

		rc = sd_bus_emit_object_added(bus_type, fan_object);
		if(rc < 0) {
			fprintf(stderr, "Failed to emit InterfacesAdded "
				"signal: %s\n", strerror(-rc));
			break;
		}
		num_fans++;
	}

	/* If we had success in adding the providers, request for a bus name. */
	if(rc >= 0) {
		/* Take one in OpenBmc */
		rc = sd_bus_request_name(bus_type, FAN_DBUS, 0);
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

	sd_bus_slot_unref(fan_slot);
	sd_bus_unref(bus_type);
	return rc;
}

int
main(void)
{
	int rc = 0;

	/* This call is not supposed to return. If it does, then an error */
	rc = start_fan_services();
	if(rc < 0) {
		fprintf(stderr, "Error starting FAN Services. Exiting");
	}
	return rc;
}

