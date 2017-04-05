#include <stdio.h>
#include <openbmc_intf.h>
#include <gpio.h>
#include <openbmc.h>

/* ------------------------------------------------------------------------- */
static const gchar* dbus_object_path = "/org/openbmc/buttons";
static const gchar* instance_name = "power0";
static const gchar* dbus_name = "org.openbmc.buttons.Power";
static const int LONG_PRESS_SECONDS = 3;
static GDBusObjectManagerServer *manager = NULL;

//This object will use these GPIOs
GPIO gpio_button = (GPIO){ "POWER_BUTTON" };

static gboolean
on_is_on(Button *btn,
		GDBusMethodInvocation *invocation,
		gpointer user_data)
{
	gboolean btn_state=button_get_state(btn);
	button_complete_is_on(btn,invocation,btn_state);
	return TRUE;
}

static gboolean
on_button_press(Button *btn,
		GDBusMethodInvocation *invocation,
		gpointer user_data)
{
	button_emit_pressed(btn);
	button_complete_sim_press(btn,invocation);
	return TRUE;
}

static gboolean
on_button_interrupt( GIOChannel *channel,
		GIOCondition condition,
		gpointer user_data )
{
	GError *error = 0;
	gsize bytes_read = 0;
	gchar buf[2];
	buf[1] = '\0';
	g_io_channel_seek_position( channel, 0, G_SEEK_SET, 0 );
	g_io_channel_read_chars(channel,
			buf, 1,
			&bytes_read,
			&error );
	printf("%s\n",buf);

	time_t current_time = time(NULL);
	if(gpio_button.irq_inited)
	{
		Button* button = object_get_button((Object*)user_data);
		if(buf[0] == '0')
		{
			printf("Power Button pressed\n");
			button_emit_pressed(button);
			button_set_timer(button,(long)current_time);
		}
		else
		{
			long press_time = current_time-button_get_timer(button);
			printf("Power Button released, held for %ld seconds\n",press_time);
			if(press_time > LONG_PRESS_SECONDS)
			{
				button_emit_pressed_long(button);
			} else {
				button_emit_released(button);
			}
		}
	}
	else { gpio_button.irq_inited = true; }

	return TRUE;
}

static void
on_bus_acquired(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data)
{
	ObjectSkeleton *object;
	//g_print ("Acquired a message bus connection: %s\n",name);
	manager = g_dbus_object_manager_server_new(dbus_object_path);
	gchar *s;
	s = g_strdup_printf("%s/%s",dbus_object_path,instance_name);
	object = object_skeleton_new(s);
	g_free(s);

	Button* button = button_skeleton_new();
	object_skeleton_set_button(object, button);
	g_object_unref(button);

	//define method callbacks
	g_signal_connect(button,
			"handle-is-on",
			G_CALLBACK(on_is_on),
			NULL); /* user_data */
	g_signal_connect(button,
			"handle-sim-press",
			G_CALLBACK(on_button_press),
			NULL); /* user_data */


	/* Export the object (@manager takes its own reference to @object) */
	g_dbus_object_manager_server_set_connection(manager, connection);
	g_dbus_object_manager_server_export(manager, G_DBUS_OBJECT_SKELETON(object));
	g_object_unref(object);

	// get gpio device paths
	int rc = GPIO_OK;
	do {
		rc = gpio_init(connection,&gpio_button);
		if(rc != GPIO_OK) { break; }
		rc = gpio_open_interrupt(&gpio_button,on_button_interrupt,object);
		if(rc != GPIO_OK) { break; }
	} while(0);
	if(rc != GPIO_OK)
	{
		printf("ERROR PowerButton: GPIO setup (rc=%d)\n",rc);
	}
}

static void
on_name_acquired(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data)
{
}

static void
on_name_lost(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data)
{
}

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "i2c-dev.h"



#define FAN_LED_OFF     0xFF
#define FAN_LED_PORT0_ALL_BLUE  0xAA
#define FAN_LED_PORT1_ALL_BLUE  0x55
#define FAN_LED_PORT0_ALL_RED   0x55
#define FAN_LED_PORT1_ALL_RED   0xAA
#define PORT0_FAN_LED_RED_MASK  0x02
#define PORT0_FAN_LED_BLUE_MASK	0x01
#define PORT1_FAN_LED_RED_MASK  0x40
#define PORT1_FAN_LED_BLUE_MASK	0x80




static int i2c_open(int bus)
{
	int rc = 0, fd = -1;
	char fn[32];

	snprintf(fn, sizeof(fn), "/dev/i2c-%d", bus);
	fd = open(fn, O_RDWR);
	if (fd == -1) {
		printf("--> Set Fan: Failed to open i2c device %s", fn);
		return -1;
	}
	return fd;
}


#define CMD_OUTPUT_PORT_0 2
#define PCA9535_ADDR 0x20
static int SetFanLed(int fd, uint8_t port0, uint8_t port1)
{
	struct i2c_rdwr_ioctl_data data;
	struct i2c_msg msg[1];
	int rc = 0, use_pec = 0;
	uint8_t write_bytes[3];

//	fprintf(stderr,"SetFanLed: port0 = %02X,port1 = %02X\n",port0,port1);

	memset(&msg, 0, sizeof(msg));

	write_bytes[0] = CMD_OUTPUT_PORT_0;
	write_bytes[1] = port0;
	write_bytes[2] = port1;
  
	msg[0].addr = PCA9535_ADDR;
	msg[0].flags = (use_pec) ? I2C_CLIENT_PEC : 0;
	msg[0].len = sizeof(write_bytes);
	msg[0].buf = write_bytes;

	data.msgs = msg;
	data.nmsgs = 1;
	rc = ioctl(fd, I2C_RDWR, &data);
	if (rc < 0) {
		printf("SetFanLed: Failed to do raw io");
		//close(fd);
		return -1;
	}

	return 0;
}


gint
main(gint argc, gchar *argv[])
{
	GMainLoop *loop;

	cmdline cmd;
	cmd.argc = argc;
	cmd.argv = argv;

	int rc = 0;
	int fd = -1;

	fd = i2c_open(9);
	if (fd != -1) {
		printf("SetFanLed====\n");
		SetFanLed(fd,FAN_LED_PORT0_ALL_BLUE,FAN_LED_PORT1_ALL_BLUE);
		//close(fd);
	}

	guint id;
	loop = g_main_loop_new(NULL, FALSE);

	id = g_bus_own_name(DBUS_TYPE,
			dbus_name,
			G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT |
			G_BUS_NAME_OWNER_FLAGS_REPLACE,
			on_bus_acquired,
			on_name_acquired,
			on_name_lost,
			&cmd,
			NULL);

	g_main_loop_run(loop);

	g_bus_unown_name(id);
	g_main_loop_unref(loop);
	return 0;
}
