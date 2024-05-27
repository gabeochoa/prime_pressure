class_name InputSwitchHandler

var previous: bool = false
var using_controller: bool = false

static func instance() -> InputSwitchHandler: 
	if Globals._input_switch_handler == null:
		Globals._input_switch_handler= InputSwitchHandler.new()
	return Globals._input_switch_handler

func update_if_switched_input_type(event): 
	previous = using_controller
	if event is InputEventJoypadButton:
		using_controller = true
		#print("using controller")
	if event is InputEventKey:
		using_controller = false
		#print("using keyboard")
func changed_since_last_input() -> bool:
	return previous != using_controller
