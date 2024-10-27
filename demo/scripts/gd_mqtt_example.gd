extends Control

const MQTT_HOST_ADDRESS    : String = "MQTT_HOST_ADDRESS"
const MQTT_CLIENT_ID       : String = "MQTT_CLIENT_ID"
const MQTT_USERNAME        : String = "MQTT_USERNAME"
const MQTT_PASSWORD        : String = "MQTT_PASSWORD"
const MQTT_SERVER_CERT_PATH: String = "MQTT_SERVER_CERT_PATH"

#| BEGIN | vars |
###########################################################################################
@onready var gd_remote_com: GDMqttCom = null
###########################################################################################
#| END   | vars |

#| BEGIN | generic overrides |
###########################################################################################
# @override
func _ready()->void:
	pass

# @override
func _process(_delta: float)->void:
	pass
###########################################################################################
#| END   | generic overrides |

#| BEGIN | private |
###########################################################################################
static func _get_env_variable(env_var: String)->String:
	var env_path: String = "res://.env"
	var env_value: String = ""
	if not FileAccess.file_exists(env_path):
		printerr("No .env file found.")
		return env_value
	var env_file: FileAccess = FileAccess.open(env_path, FileAccess.READ)
	while env_file.get_position() < env_file.get_length():
		var env_str: String = env_file.get_line()
		if env_str.begins_with(env_var):
			env_value = env_str.get_slice("=", 1)
	return env_value
###########################################################################################
#| END   | private |

#| BEGIN | public |
###########################################################################################
###########################################################################################
#| END   | public |

#| BEGIN | Signal callbacks |
###########################################################################################
###########################################################################################
#| END   | signal callbacks |
