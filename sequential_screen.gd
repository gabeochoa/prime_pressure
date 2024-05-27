class_name SequentialScreen extends Screen
	
class SequentialAction extends ActionListener: 
	func _init(data: ActionData):
		super(data)
		
	func action(screen:Screen): 
		print("on action triggered: ", data.name)
		screen.next()
		
func next(first_run: bool = false):
	is_dirty = true
	
	if !first_run:
		active_action.data.set_active(false)
		config.increment_action_index()
		
	active_action = config.get_current_action()
	active_action.data.set_active(true)
	
	print("next_iter, current action is ", active_action.data.name)

func is_active() -> bool: 
	return !is_complete()
	
func is_active_action(action: ActionListener) -> bool: 
	return action.data.name == active_action.data.name 

