extends Node2D

class ScreenListener:
	var hide_label: bool
	var name: String
	var key: Key
	var gamepad: String

	func _init(n: String, k:Key):
		name = n
		key = k
		
	func is_active() -> bool: 
		return true
		
	func action(screen: Screen): 
		assert(false, "class " + name + " needs to implement action()")
		return false

	func is_not_active() -> bool:
		return !is_active()


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

class Screen: 
	class CompleteAction extends ScreenListener:
		func _init():
			super("Finish", KEY_ENTER)
			gamepad = "action_finish"
			
		func action(screen:Screen): 
			print("screen " + screen.name + " complete")
			screen.complete = true;
	
	var complete: bool
	var name: String
	var config : Array[ScreenListener]
	var is_dirty: bool = true
	
	func _init(n: String, c: Array[ScreenListener]):
		name = n
		config = c

	func is_active() -> bool: 
		assert(false, "class " + name + " needs to implement is_active()")
		return false

	func is_not_active() -> bool:
		return !is_active()

	
class BoxScreen extends Screen: 
	class BoxUnfold extends ScreenListener:
		func _init():
			super("unfold", KEY_U)
			
		func action(screen:Screen): 
			print(name)
			screen.next()

	class BoxBottom extends ScreenListener:
		func _init():
			super("bottom", KEY_B)
			
		func action(screen:Screen): 
			print(name)
			screen.next()

	class BoxTape extends ScreenListener:
		func _init():
			super("tape", KEY_T)
			
		func action(screen:Screen): 
			print(name)
			screen.next()
			
	var enabled: int
	
	func next():
		is_dirty = true
		enabled += 1

	func _init():
		super("box", [
			BoxUnfold.new(),
			BoxBottom.new(),
			BoxTape.new(),
			CompleteAction.new()
		])
		enabled = 0;
		
	func is_complete() -> bool: 
		return complete

	func is_active() -> bool: 
		return !is_complete()
		
	func is_active_action(action: ScreenListener) -> bool: 
		for i in range(0, config.size()): 
			if enabled != i: 
				continue
			var act = config[i]
			if action.name != act.name:
				#print("action names didnt match %s and %s" % [action.name, act.name])
				continue
			return act.is_active();
		return false
			
				

class OrderScreen extends Screen: 
	class OrderLetter extends ScreenListener:
		func _init(key: Key):
			super("order_letter_%s" % key, key)
			hide_label = true
			
		func action(screen:Screen): 
			print(name)
			screen.next()
	
	func create_order_letter(item: String):
		var list: Array[ScreenListener]
		for character in item: 
			var char: int = character.to_ascii_buffer()[0] - "A".to_ascii_buffer()[0]
			var key: Key = char + KEY_A
			#print(" idk char %s %s %s key %s" % [character, character.to_ascii_buffer()[0], char, key])
			list.append(OrderLetter.new(key))
		return list
	
	var enabled: int
	
	func next():
		is_dirty = true
		enabled += 1

	func _init():
		var actions = create_order_letter("SHAM")
		actions.append_array( [ CompleteAction.new() ] )
		super("order", actions)
		enabled = 0;
		
	func is_complete() -> bool: 
		return complete

	func is_active() -> bool: 
		return !is_complete()
		
	func is_active_action(action: ScreenListener) -> bool: 
		for i in range(0, config.size()): 
			if enabled != i: 
				continue
			var act = config[i]
			if action.name != act.name:
				#print("action names didnt match %s and %s" % [action.name, act.name])
				continue
			return act.is_active();
		return false
			
				


var action_index = 0;
var actions: Array[String] = [
	"action_one",
	"action_two",
	"action_three",
	"action_four",
]

var screens: Array = [
	OrderScreen.new(),
	BoxScreen.new()
]

var using_controller: bool = true

@onready var screen_container: GridContainer = $"../CanvasLayer/VBoxContainer/PanelContainer/MarginContainer/GridContainer"

# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.

func assign_inputs_for_action(screen:Screen, action:ScreenListener):
	if action.gamepad.length() != 0:
		return
		
	if !screen.is_active_action(action):
		return
		
	assert(action_index < actions.size(), "Showing more actions on screen than we have actions for")
	action.gamepad = actions[action_index]
	action_index += 1

func handle_input_for_action(screen:Screen, action: ScreenListener):
	if !screen.is_active_action(action):
		return
		
	assign_inputs_for_action(screen,action)
		
	if Input.is_key_pressed(action.key) or Input.is_action_just_pressed(action.gamepad):
		action.action(screen)
		get_viewport().set_input_as_handled()
		return


# this cant be Array[Screen] because godot deletes the inner type idk
func get_active_screens() -> Array:
	return screens.filter(func(screen): return screen.is_active())


func _unhandled_input(event):
	action_index = 0
	var old_controller = using_controller
	
	if event is InputEventJoypadButton:
		using_controller = true
		print("using controller")
		
	if event is InputEventKey:
		using_controller = false
		print("using keyboard")
	
	for screen in get_active_screens():
		for action in screen.config:
			if old_controller != using_controller:
				screen.is_dirty = true
			handle_input_for_action(screen, action)
	
	#if event.pressed and event.keycode == KEY_W:
	#get_viewport().set_input_as_handled()

func create_action_button(screen: Screen, action: ScreenListener) -> Control :
	var label = ActionLabel.new(
		action.name,
		action.key,
		action.gamepad,
		using_controller,
		screen.is_active_action(action)
	)
	if action.hide_label:
		label.hide_label = true
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
	for action in active_screen.config:
		assign_inputs_for_action(active_screen,action)
		var action_ui = create_action_button(active_screen, action)
		screen_container.add_child(action_ui)

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	render_actions()
	pass
