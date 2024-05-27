class_name ActionManager

var action_index = 0;
var actions: Array[String] = [
	"action_one",
	"action_two",
	"action_three",
	"action_four",
]

static func instance() -> ActionManager: 
	if Globals._action_manager == null:
		Globals._action_manager= ActionManager.new()
	return Globals._action_manager

func assign_and_increment(actionData: ActionData):
	assert(action_index < actions.size(), "Showing more actions on screen than we have actions for")
	actionData.gamepad = actions[action_index]
	action_index += 1

func reset_count():
	action_index = 0
	
