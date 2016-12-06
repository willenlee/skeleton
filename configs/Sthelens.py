#! /usr/bin/python

HOME_PATH = './'
CACHE_PATH = '/var/cache/obmc/'
FLASH_DOWNLOAD_PATH = "/tmp"
GPIO_BASE = 320
SYSTEM_NAME = "StHelens"


## System states
##   state can change to next state in 2 ways:
##   - a process emits a GotoSystemState signal with state name to goto
##   - objects specified in EXIT_STATE_DEPEND have started
SYSTEM_STATES = [
	'BASE_APPS',
	'BMC_STARTING',
	'BMC_STARTING2',
	'BMC_READY',
	'HOST_POWERING_ON',
	'HOST_POWERED_ON',
	'INVENTORY_UPLOADED',
	'HOST_BOOTING',
	'HOST_BOOTED',
	'HOST_POWERED_OFF',
]

EXIT_STATE_DEPEND = {
	'BASE_APPS' : {
		'/org/openbmc/sensors': 0,
	},
	'BMC_STARTING' : {
#		'/org/openbmc/control/power0' : 0,
#		'/org/openbmc/control/host0' : 0,
#		'/org/openbmc/control/flash/bios' : 0,
#		'/org/openbmc/sensors/speed/fan5': 0,
		'/org/openbmc/inventory/system/chassis/io_board' : 0,
	},
	'BMC_STARTING2' : {
#		'/org/openbmc/control/fans' : 0,
		'/org/openbmc/control/chassis0': 0,
	},
}

## method will be called when state is entered
ENTER_STATE_CALLBACK = {
	'INVENTORY_UPLOADED' : {
		'boot' : {
			'bus_name'    : 'org.openbmc.control.Host',
			'obj_name'    : '/org/openbmc/control/host0',
			'interface_name' : 'org.openbmc.control.Host',
		},
		'setOn' : {
			'bus_name'   : 'org.openbmc.control.led',
			'obj_name'   : '/org/openbmc/control/led/identify',
			'interface_name' : 'org.openbmc.Led',
		}
	},
	'HOST_POWERED_OFF' : {
		'setOff' : {
			'bus_name'   : 'org.openbmc.control.led',
			'obj_name'   : '/org/openbmc/control/led/identify',
			'interface_name' : 'org.openbmc.Led',
		}

	},
	'BMC_READY' : {
		'setBlinkSlow' : {
			'bus_name'   : 'org.openbmc.control.led',
			'obj_name'   : '/org/openbmc/control/led/heartbeat',
			'interface_name' : 'org.openbmc.Led',
		},
		'init' : {
			'bus_name'   : 'org.openbmc.control.Flash',
			'obj_name'   : '/org/openbmc/control/flash/bios',
			'interface_name' : 'org.openbmc.Flash',
		}
	}
}

APPS = {
	'startup_hacks' : {
		'system_state'    : 'BASE_APPS',
		'start_process'   : True,
		'monitor_process' : False,
		'process_name'    : 'startup_hacks.sh',
	},
	'inventory' : {
		'system_state'    : 'BMC_STARTING',
		'start_process'   : True,
		'monitor_process' : True,
		'process_name'    : 'inventory_items.py',
		'args'            : [ SYSTEM_NAME ]
	},
	'inventory_upload' : {
		'system_state'    : 'HOST_POWERED_ON',
		'start_process'   : True,
		'monitor_process' : False,
		'process_name'    : 'goto_system_state.py',
		'args'            : [ 'INVENTORY_UPLOADED', 'inventory_upload.py' ]
	},
	'pcie_present' : {
		'system_state'    : 'INVENTORY_UPLOADED',
		'start_process'   : True,
		'monitor_process' : False,
		'process_name'    : 'pcie_slot_present.exe',
	},
	'fan_control' : {
		'system_state'    : 'BMC_STARTING2',
		'start_process'   : True,
		'monitor_process' : True,
		'process_name'    : 'fan_control.py',
	},
	'hwmon' : {
		'system_state'    : 'BMC_STARTING',
		'start_process'   : True,
		'monitor_process' : True,
		'process_name'    : 'hwmon.py',
		'args'            : [ SYSTEM_NAME ]
	},
	'sensor_manager' : {
		'system_state'    : 'BASE_APPS',
		'start_process'   : True,
		'monitor_process' : True,
		'process_name'    : 'sensor_manager2.py',
		'args'            : [ SYSTEM_NAME ]
	},
	'host_watchdog' : {
		'system_state'    : 'BMC_STARTING',
		'start_process'   : True,
		'monitor_process' : True,
		'process_name'    : 'host_watchdog.exe',
	},
	'power_control' : {
		'system_state'    : 'BMC_STARTING',
		'start_process'   : True,
		'monitor_process' : True,
		'process_name' : 'power_control.exe',
		'args' : [ '3000', '10' ]
	},
	'power_button' : {
		'system_state'    : 'BMC_STARTING',
		'start_process'   : True,
		'monitor_process' : True,
		'process_name'    : 'button_power.exe',
	},
        'reset_button' : {
                'system_state'    : 'BMC_STARTING',
                'start_process'   : True,
                'monitor_process' : True,
                'process_name'    : 'button_reset.exe',
        },
	'host_checkstop' : {
		'system_state'    : 'BMC_STARTING',
		'start_process'   : True,
		'monitor_process' : True,
		'process_name'    : 'host_checkstop.exe',
	},
	'led_control' : {
		'system_state'    : 'BMC_STARTING',
		'start_process'   : True,
		'monitor_process' : True,
		'process_name'    : 'led_controller.exe',
	},
	'flash_control' : {
		'system_state'    : 'BMC_STARTING',
		'start_process'   : True,
		'monitor_process' : True,
		'process_name'    : 'flash_bios.exe',
	},
	'bmc_flash_control' : {
		'system_state'    : 'BMC_STARTING',
		'start_process'   : True,
		'monitor_process' : True,
		'process_name'    : 'bmc_update.py',
	},
	'download_manager' : {
		'system_state'    : 'BMC_STARTING',
		'start_process'   : True,
		'monitor_process' : True,
		'process_name'    : 'download_manager.py',
		'args'            : [ SYSTEM_NAME ]
	},
	'host_control' : {
		'system_state'    : 'BMC_STARTING',
		'start_process'   : True,
		'monitor_process' : True,
		'process_name'    : 'control_host.exe',
	},
	'chassis_control' : {
		'system_state'    : 'BMC_STARTING2',
		'start_process'   : True,
		'monitor_process' : True,
		'process_name'    : 'chassis_control.py',
	},
	'board_vpd' : {
		'system_state'    : 'BMC_STARTING2',
		'start_process'   : True,
		'monitor_process' : False,
		'process_name'    : 'phosphor-read-eeprom',
		'args'            : ['--eeprom','/sys/bus/i2c/devices/0-0050/eeprom','--fruid','64'],
	},
	'motherboard_vpd' : {
		'system_state'    : 'BMC_STARTING2',
		'start_process'   : True,
		'monitor_process' : False,
		'process_name'    : 'phosphor-read-eeprom',
		'args'            : ['--eeprom','/sys/bus/i2c/devices/4-0054/eeprom','--fruid','3'],
	},
	'exp_vpd' : {
		'system_state'    : 'BMC_STARTING2',
		'start_process'   : True,
		'monitor_process' : False,
		'process_name'    : 'phosphor-read-eeprom',
		'args'            : ['--eeprom','/sys/bus/i2c/devices/6-0051/eeprom','--fruid','65'],
	},
	'hdd_vpd' : {
		'system_state'    : 'BMC_STARTING2',
		'start_process'   : True,
		'monitor_process' : False,
		'process_name'    : 'phosphor-read-eeprom',
		'args'            : ['--eeprom','/sys/bus/i2c/devices/6-0055/eeprom','--fruid','66'],
	},
	'restore' : {
		'system_state'    : 'BMC_READY',
		'start_process'   : True,
		'monitor_process' : False,
		'process_name'    : 'discover_system_state.py',
	},
	'info' : {
                'system_state'    : 'BMC_STARTING2',
                'start_process'   : True,
                'monitor_process' : True,
                'process_name'    : 'info.exe',
	},
	'fan_algorithm' : {
                'system_state'    : 'BMC_READY',
                'start_process'   : True,
                'monitor_process' : True,
                'process_name'    : 'fan_algorithm.exe',
	},
	'bmc_control' : {
		'system_state'    : 'BMC_STARTING',
		'start_process'   : True,
		'monitor_process' : True,
		'process_name'    : 'control_bmc.exe',
	},
	'id_button' : {
		'system_state'    : 'BMC_STARTING',
		'start_process'   : True,
		'monitor_process' : True,
		'process_name'    : 'button_id.exe',
	},
	'cable_led' : {
		'system_state'    : 'BMC_STARTING',
		'start_process'   : True,
		'monitor_process' : True,
		'process_name'    : 'cable_led.exe',
	},
	'netman' : {
		'system_state'    : 'BMC_STARTING2',
		'start_process'   : True,
		'monitor_process' : True,
		'process_name'    : 'netman.py',
	},
	'sync_mac' : {
		'system_state'    : 'BMC_READY',
		'start_process'   : True,
		'monitor_process' : False,
		'process_name'    : 'sync_inventory_items.py',
		'args'            : ['-t','DAUGHTER_CARD','-n','io_board','-p','Custom Field 2','-s','mac'],
    },
	'fan_ctl' : {
		'system_state'    : 'BMC_STARTING',
		'start_process'   : True,
		'monitor_process' : True,
		'process_name'    : 'fan_generic_obj.exe',
	},
}

CACHED_INTERFACES = {
		"org.openbmc.InventoryItem" : True,
		"org.openbmc.control.Chassis" : True,
	}
INVENTORY_ROOT = '/org/openbmc/inventory'

FRU_INSTANCES = {
	        '<inventory_root>/system' : { 'fru_type' : 'SYSTEM','is_fru' : True, 'present' : "True" },
        	'<inventory_root>/system/misc' : { 'fru_type' : 'SYSTEM','is_fru' : False, },

        	'<inventory_root>/system/chassis' : { 'fru_type' : 'SYSTEM','is_fru' : True, 'present' : "True" },

	        '<inventory_root>/system/chassis/motherboard' : { 'fru_type' : 'MAIN_PLANAR','is_fru' : True, },

        	'<inventory_root>/system/systemevent'                  : { 'fru_type' : 'SYSTEM_EVENT', 'is_fru' : False, },
        	'<inventory_root>/system/chassis/motherboard/refclock' : { 'fru_type' : 'MAIN_PLANAR', 'is_fru' : False, },
        	'<inventory_root>/system/chassis/motherboard/pcieclock': { 'fru_type' : 'MAIN_PLANAR', 'is_fru' : False, },
        	'<inventory_root>/system/chassis/motherboard/todclock' : { 'fru_type' : 'MAIN_PLANAR', 'is_fru' : False, },
        	'<inventory_root>/system/chassis/motherboard/apss'     : { 'fru_type' : 'MAIN_PLANAR', 'is_fru' : False, },
	
	        '<inventory_root>/system/chassis/motherboard/bmc' : { 'fru_type' : 'BMC','is_fru' : False, 'manufacturer' : 'ASPEED' },
	        '<inventory_root>/system/chassis/io_board' : { 'fru_type' : 'DAUGHTER_CARD','is_fru' : True,'Custom Field 2': '00:00:00:00:00:00',},
}

ID_LOOKUP = {
	'FRU' : {
		0x40 : '<inventory_root>/system/chassis/io_board',
	},
	'FRU_STR' : {
		'BOARD_100'  : '<inventory_root>/system/chassis/io_board',
	},
	'SENSOR' : {
	},
	'GPIO_PRESENT' : {
	}
}

GPIO_CONFIG = {}
GPIO_CONFIG['POWER_BUTTON'] = { 'gpio_pin': 'F1', 'direction': 'both' }
GPIO_CONFIG['IDBTN']       = { 'gpio_pin': 'F3', 'direction': 'both' }

def convertGpio(name):
	name = name.upper()
	c = name[0:1]
	offset = int(name[1:])
	a = ord(c)-65
	base = a*8+GPIO_BASE
	return base+offset

HWMON_CONFIG = {
	'11-0048' :  {
		'names' : {
			'temp1_input' : { 'object_path' : 'temperature/TMP1','poll_interval' : 5000,'scale' : 1000,'units' : 'C',
					'critical_upper' : 40, 'critical_lower' : -100, 'warning_upper' : 36, 'warning_lower' : -99, 'emergency_enabled' : True },
		}
	},
	'11-0049' :  {
		'names' : {
			'temp1_input' : { 'object_path' : 'temperature/TMP2','poll_interval' : 5000,'scale' : 1000,'units' : 'C',
					'critical_upper' : 40, 'critical_lower' : -100, 'warning_upper' : 36, 'warning_lower' : -99, 'emergency_enabled' : True },
		}
	},
	'11-004a' :  {
		'names' : {
			'temp1_input' : { 'object_path' : 'temperature/TMP3','poll_interval' : 5000,'scale' : 1000,'units' : 'C',
					'critical_upper' : 40, 'critical_lower' : -100, 'warning_upper' : 36, 'warning_lower' : -99, 'emergency_enabled' : True },
		}
	},
	'11-004b' :  {
		'names' : {
			'temp1_input' : { 'object_path' : 'temperature/TMP4','poll_interval' : 5000,'scale' : 1000,'units' : 'C',
					'critical_upper' : 40, 'critical_lower' : -100, 'warning_upper' : 36, 'warning_lower' : -99, 'emergency_enabled' : True },
		}
	},
	'11-004c' :  {
		'names' : {
			'temp1_input' : { 'object_path' : 'temperature/TMP5','poll_interval' : 5000,'scale' : 1000,'units' : 'C',
					'critical_upper' : 40, 'critical_lower' : -100, 'warning_upper' : 36, 'warning_lower' : -99, 'emergency_enabled' : True },
		}
	},
	'11-004d' :  {
		'names' : {
			'temp1_input' : { 'object_path' : 'temperature/TMP6','poll_interval' : 5000,'scale' : 1000,'units' : 'C',
					'critical_upper' : 40, 'critical_lower' : -100, 'warning_upper' : 36, 'warning_lower' : -99, 'emergency_enabled' : True },
		}
	},
	'11-004e' :  {
		'names' : {
			'temp1_input' : { 'object_path' : 'temperature/TMP7','poll_interval' : 5000,'scale' : 1000,'units' : 'C',
					'critical_upper' : 40, 'critical_lower' : -100, 'warning_upper' : 36, 'warning_lower' : -99, 'emergency_enabled' : True },
		}
	},
	'11-004f' :  {
		'names' : {
			'temp1_input' : { 'object_path' : 'temperature/TMP8','poll_interval' : 5000,'scale' : 1000,'units' : 'C',
					'critical_upper' : 40, 'critical_lower' : -100, 'warning_upper' : 36, 'warning_lower' : -99, 'emergency_enabled' : True },
		}
	},
}

# Miscellaneous non-poll sensor with system specific properties.
# The sensor id is the same as those defined in ID_LOOKUP['SENSOR'].
MISC_SENSORS = {
}

# Set fan_algorithm config
#   FAN_DBUS_INTF_LOOKUP: {keys:[dbus bus name, dbus interface name]}
#   CHASSIS_POWER_STATE: set chassis power state object path
#   FAN_INPUT_OBJ: set fan input object path
#   FAN_OUTPUT_OBJ: set fan out object path , eg: hwmon control fan speed or directly use pwm control pwm
#   OPEN_LOOP_PARAM: set openloop parameter
#       - paramA
#       - paramB
#       - paramC
#       - Low_Amb
#       - Up_Amb
#       - Low_Speed
#       - High_Speed
#   OPEN_LOOP_GROUPS_1: set openloop object path
#   CLOSE_LOOP_PARAM_1: set closeloop parameter
#       - g_Kp
#       - g_Ki
#       - g_Kd
#       - g_CPUVariable
#       - g_DIMMVariable
#       - sensor_tracking
#       - highest warning temp
#   CLOSE_LOOP_GROUPS_1: set closeloop  group001 object path, eg: CPU0/CPU1
#   CLOSE_LOOP_PARAM_2: set closeloop parameter
#       - g_Kp
#       - g_Ki
#       - g_Kd
#       - g_CPUVariable
#       - g_DIMMVariable
#       - sensor_tracking
#       - highest warning temp
#   CLOSE_LOOP_GROUPS_2: set closeloop  group002 object path, eg: DIMM
#   FAN_LED_OFF: set fan led command: off
#   FAN_LED_PORT0_ALL_BLUE: set fan led port0 command: all blue
#   FAN_LED_PORT1_ALL_BLUE: set fan led port1 command: all blue
#   FAN_LED_PORT0_ALL_RED: set fan led port0 command: all red
#   FAN_LED_PORT1_ALL_RED: set fan led port1 command: all red
#   PORT0_FAN_LED_RED_MASK: set fan led port0 register mask with red
#   PORT0_FAN_LED_BLUE_MASK: set fan led port0 register mask with blue
#   PORT1_FAN_LED_RED_MASK: set fan led port1 register mask with red
#   PORT1_FAN_LED_BLUE_MASK: set fan led port1 register mask with blue
#   FAN_LED_I2C_BUS: set fan led i2c bus
#   FAN_LED_I2C_SLAVE_ADDRESS: set fan led i2c slave address
FAN_ALGORITHM_CONFIG = {
    'FAN_DBUS_INTF_LOOKUP':
    {
        'CHASSIS_POWER_STATE': ['org.openbmc.control.Chassis', 'org.openbmc.control.Chassis'],
        'FAN_INPUT_OBJ' : ['org.openbmc.control.fan', 'org.openbmc.Fan'],
        'FAN_OUTPUT_OBJ' : ['org.openbmc.control.fan', 'org.openbmc.Fan'],
        'OPEN_LOOP_GROUPS_1' : [],
        'CLOSE_LOOP_GROUPS_1' : [],
        'CLOSE_LOOP_GROUPS_2' : [],
    },

    'CHASSIS_POWER_STATE': ['/org/openbmc/control/chassis0'],
    'FAN_INPUT_OBJ':
        [
            "/org/openbmc/control/fan/fan_tacho0",
            "/org/openbmc/control/fan/fan_tacho1",
            "/org/openbmc/control/fan/fan_tacho2",
            "/org/openbmc/control/fan/fan_tacho3",
            "/org/openbmc/control/fan/fan_tacho4",
            "/org/openbmc/control/fan/fan_tacho5",
            "/org/openbmc/control/fan/fan_tacho6",
            "/org/openbmc/control/fan/fan_tacho7",
            "/org/openbmc/control/fan/fan_tacho8",
            "/org/openbmc/control/fan/fan_tacho9",
            "/org/openbmc/control/fan/fan_tacho10",
            "/org/openbmc/control/fan/fan_tacho11",
        ],
    'FAN_OUTPUT_OBJ':
        [
            "/org/openbmc/control/fan/fan0",
            "/org/openbmc/control/fan/fan1",
            "/org/openbmc/control/fan/fan2",
            "/org/openbmc/control/fan/fan3",
            "/org/openbmc/control/fan/fan4",
            "/org/openbmc/control/fan/fan5",
        ],
    'OPEN_LOOP_PARAM':
        [
            '0',
            '2',
            '0',
            '20',
            '38',
            '40',
            '100',
        ],
    'OPEN_LOOP_GROUPS_1':[],
    'CLOSE_LOOP_PARAM_1' :
        [
            '0.45',
            '-0.017',
            '0.3',
            '80',
            '85',
        ],
    'CLOSE_LOOP_GROUPS_1':[],
    'CLOSE_LOOP_PARAM_2' :
        [
            '0.45',
            '-0.017',
            '0.3',
            '75',
            '85',
        ],
    'CLOSE_LOOP_GROUPS_2':[],

    'FAN_LED_OFF': ["0xFF"],
    'FAN_LED_PORT0_ALL_BLUE': ["0xAA"],
    'FAN_LED_PORT1_ALL_BLUE': ["0x55"],
    'FAN_LED_PORT0_ALL_RED': ["0x55"],
    'FAN_LED_PORT1_ALL_RED': ["0xAA"],
    'PORT0_FAN_LED_RED_MASK': ["0x02"],
    'PORT0_FAN_LED_BLUE_MASK': ["0x01"],
    'PORT1_FAN_LED_RED_MASK': ["0x40"],
    'PORT1_FAN_LED_BLUE_MASK': ["0x80"],
    'FAN_LED_SPEED_LIMIT': ["30"],

    'FAN_LED_I2C_BUS': ["/dev/i2c-6"],
    'FAN_LED_I2C_SLAVE_ADDRESS': ["0x20"],
}

