class_name Screen

class CompleteAction extends ActionListener:
	func _init():
		super(
			ActionData.new()
			.set_name("Finish")
			.set_key(KEY_ENTER)
			.set_gamepad("action_finish")
		)
		
	func action(screen:Screen): 
		print("screen " + screen.name + " complete")
		screen.complete = true;

var num_items_per_row: int = 1
var complete: bool
var name: String
var config : ActionGroup
var active_action: ActionListener = ActionListener.get_empty()
var is_dirty: bool = true

func _init(n: String, c: ActionGroup):
	name = n
	config = c

func is_active() -> bool: 
	assert(false, "class " + name + " needs to implement is_active()")
	return false

func is_not_active() -> bool:
	return !is_active()

func is_complete() -> bool: 
	return complete
