class_name ActionListener

var data: ActionData
var is_empty: bool = false
var is_complete: bool = false

func _init(action_data: ActionData):
	data = action_data
	
func is_active() -> bool: 
	return !is_empty and !is_complete
	
func action(_screen: Screen): 
	assert(false, "class " + data.name + " needs to implement action()")
	return false

func is_not_active() -> bool:
	return !is_active()

func set_empty(b: bool):
	is_empty = b 
	
static func get_empty() -> ActionListener :
	return ActionListener.new(
			ActionData.new().set_active(false)
		).set_empty(true)
