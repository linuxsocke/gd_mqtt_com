extends Control

const MQTT_HOST_ADDRESS    : String = "MQTT_HOST_ADDRESS"
const MQTT_CLIENT_ID       : String = "MQTT_CLIENT_ID"
const MQTT_USERNAME        : String = "MQTT_USERNAME"
const MQTT_PASSWORD        : String = "MQTT_PASSWORD"
const MQTT_SERVER_CERT_PATH: String = "MQTT_SERVER_CERT_PATH"

#| BEGIN | vars |
###########################################################################################
@onready var gd_mqtt_com: GDMqttCom = null
###########################################################################################
#| END   | vars |

#| BEGIN | generic overrides |
###########################################################################################
# @override
func _ready()->void:
	var host_address: String = SaveData.get_env_variable(SaveData.MQTT_HOST_ADDRESS)
	if host_address=="":
		printerr("Mqtt host_address not defined.")
		return
	var client_id: String = SaveData.get_env_variable(SaveData.MQTT_CLIENT_ID)
	if client_id=="":
		printerr("Mqtt client_id not defined.")
		return
	var username: String = SaveData.get_env_variable(SaveData.MQTT_USERNAME)
	if username=="":
		printerr("Mqtt username not defined.")
		return
	var password: String = SaveData.get_env_variable(SaveData.MQTT_PASSWORD)
	if password=="":
		printerr("Mqtt password not defined.")
		return
	var server_cert_path: String = SaveData.get_env_variable(SaveData.MQTT_SERVER_CERT_PATH)
	if server_cert_path=="":
		printerr("Mqtt server_cert_path not defined.")
		return
	print("Mqtt host address: %s, ClientID: %s, Username: %s, cert path: %s" % 
		[host_address, client_id, username, server_cert_path])
	# TODO if cert path use user pw auth? Or print error in load screen?
	gd_mqtt_com = GDRemoteCom.new()
	var res: int = gd_mqtt_com.setup_remote_com(
		host_address, client_id, username, password, server_cert_path)
	if res!=GDRemoteCom.GD_SUCCESS:
		gd_mqtt_com = null
		printerr("GDRemoteCom: Failed setup mqtt com with error code: ", res)
		return
	res = gd_mqtt_com.connect()
	if res!=GDRemoteCom.GD_SUCCESS:
		printerr("GDRemoteCom: Failed connect mqtt com with error code: ", res)
		return
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

func _await_connected_signal()->void:
	await gd_mqtt_com.connected
	var res: int = _gd_remote_com.subscribe_to_topic(
		EXAMPLE_TOPIC_NAME, 
		_example_cb
	)
	if res!=GDRemoteCom.GD_SUCCESS:
		printerr("GDRemoteCom: Failed subscribe heartbeat failed with error code: ", res)
		return

func _example_cb(json_str: String)->void:
	print("Received %s" % json_str)
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
