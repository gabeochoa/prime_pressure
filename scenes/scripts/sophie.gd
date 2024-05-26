extends Node2D


'''
	open the order
		order the items  (type the first four letters, or four button presses)
	wait for the order to be fetched 
	grab box 
		unfold
		bottom
		tape
	place items 
		order the items  (type the first four letters, or four button presses)
		add paper 
		top
		tape
		
	1
		sham
		chip
		toil
	...
	g
		u
		f
		t
	p
		sham
		chip
		toil
		p
		t
'''


	
class BoxScreen extends Screen: 
	class BoxUnfold extends ActionListener:
		func _init():
			super(
				ActionData.new()
				.set_name("unfold")
				.set_key(KEY_U)
			)
			
		func action(screen:Screen): 
			print("on action triggered: ", data.name)
			screen.next()

	class BoxBottom extends ActionListener:
		func _init():
			super(
				ActionData.new()
				.set_name("bottom")
				.set_key(KEY_B)
			)
			
		func action(screen:Screen): 
			print("on action triggered: ", data.name)
			screen.next()

	class BoxTape extends ActionListener:
		func _init():
			super(
				ActionData.new()
				.set_name("tape")
				.set_key(KEY_T)
			)
			
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

	func _init():
		super("box", ActionGroup.new().add_actions([
			BoxUnfold.new(),
			BoxBottom.new(),
			BoxTape.new(),
			CompleteAction.new()
		]))
		next(true)
		
	func is_complete() -> bool: 
		return complete

	func is_active() -> bool: 
		return !is_complete()
		
	func is_active_action(action: ActionListener) -> bool: 
		return action.data.name == active_action.data.name 

class OrderScreen extends Screen: 
	class OrderLetter extends ActionListener:
		func _init(key: Key):
			super(
				ActionData.new()
				.set_name("order_letter_%s" % key)
				.set_key(key)
				.set_hide_label(true)
			)
			
		func action(screen:Screen): 
			print(data.name)
			screen.next()
	
	func create_order_letter(item: String):
		var list: Array[ActionListener] = []
		for character in item: 
			var char_as_int: int = character.to_ascii_buffer()[0] - "A".to_ascii_buffer()[0]
			var key: Key = (char_as_int as Key) + KEY_A
			#print(" idk char %s %s %s key %s" % [character, character.to_ascii_buffer()[0], char, key])
			list.append(OrderLetter.new(key))
		return list
	
	func next():
		is_dirty = true
		active_action = config.get_current_action()

	func _init():
		var group = (
			ActionGroup.new()
			.add_child(
				ActionGroup.new().add_actions(
						create_order_letter("SHAM")
					)
			)
			.add_actions(
				[CompleteAction.new()]
			))
		super("order", group)
		next()
		
	func is_complete() -> bool: 
		return complete

	func is_active() -> bool: 
		return !is_complete()
		
	func is_active_action(action: ActionListener) -> bool: 
		return action.data.name == active_action.data.name and action.is_active()
			

var action_index = 0;
var actions: Array[String] = [
	"action_one",
	"action_two",
	"action_three",
	"action_four",
]

var screens: Array = [
	BoxScreen.new(),
	#OrderScreen.new(),
]

var using_controller: bool = true
@export var action_label_scene: PackedScene
@onready var screen_container: GridContainer =  %GridContainer

func assign_inputs_for_action(screen:Screen, action:ActionListener):
	if action.data.gamepad.length() != 0:
		return
		
	assert(action_index < actions.size(), "Showing more actions on screen than we have actions for")
	action.data.gamepad = actions[action_index]
	action_index += 1

# this cant be Array[Screen] because godot deletes the inner type idk
func get_active_screens() -> Array:
	return screens.filter(func(screen): return screen.is_active())

func _unhandled_input(event):
	action_index = 0
	var old_controller = using_controller
	
	if event is InputEventJoypadButton:
		using_controller = true
		#print("using controller")
		
	if event is InputEventKey:
		using_controller = false
		#print("using keyboard")
	
	for screen in get_active_screens():
		var action = screen.active_action
		if action.is_active():
			if old_controller != using_controller:
				screen.is_dirty = true
			if !screen.is_active_action(action):
				continue
				
			if Input.is_key_pressed(action.data.key) or Input.is_action_just_pressed(action.data.gamepad):
				print("action ", action.data.name, action.data.gamepad)
				action.action(screen)
				get_viewport().set_input_as_handled()
				return
	
	#if event.pressed and event.keycode == KEY_W:
	#get_viewport().set_input_as_handled()

func create_action_button(screen: Screen, action: ActionListener) -> Control :
	assign_inputs_for_action(screen, action)

	var label = action_label_scene.instantiate().with_data(
		action.data
		.set_is_controller(using_controller)
		.set_active(screen.is_active_action(action))
	)
	
	if action.data.hide_label:
		label.data.hide_label = true
		
	var wrapper = MarginContainer.new()
	wrapper.add_child(label)
	wrapper.add_theme_constant_override("theme_override_constants/margin_left", 100)
	return wrapper
		
func render_actions():
	var children = screen_container.get_children();
	
	var actives = get_active_screens()
	if actives.size() == 0:
		print("no active screens")
		for child in children:
			child.queue_free()
		return
		
	var active_screen = actives[0]
	if !active_screen.is_dirty:
		return
	active_screen.is_dirty = false
	
	for child in children:
		child.queue_free()

	active_screen.config.for_each_active(
		func (action): 
			screen_container.add_child(
				create_action_button(active_screen, action)
			)
	)

func _process(_delta):
	render_actions()
	pass
