extends Node2D

enum WindowType {
	Order,
	Box,
	Packing
}

class ScreenListener:
	var name: String
	var key: Key
	var gamepad: String

	func _init(n: String, k:Key ):
		name = n
		key = k
			
	func is_active() -> bool: 
		assert(false, "class " + name + " needs to implement is_active()")
		return false
		
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
	var name: String
	var config : Array[ScreenListener]

	func _init(n: String, c: Array[ScreenListener]):
		name = n
		config = c

	func is_active() -> bool: 
		assert(false, "class " + name + " needs to implement is_active()")
		return false

	func is_not_active() -> bool:
		return !is_active()


class BoxScreen extends Screen: 
	var enabled: Dictionary
	
	class BoxUnfold extends ScreenListener:
		func _init():
			super("unfold", KEY_U)
			
		func is_active() -> bool: 
			return true
			
		func action(screen:Screen): 
			print("unfold")

	class BoxBottom extends ScreenListener:
		func _init():
			super("bottom", KEY_B)
			
		func is_active() -> bool: 
			return true
			
		func action(screen:Screen): 
			print("bottom")

	class BoxTape extends ScreenListener:
		func _init():
			super("tape", KEY_T)
			
		func is_active() -> bool: 
			return true
			
		func action(screen:Screen): 
			print("tape")

	func _init():
		super("box", [
			BoxUnfold.new(),
			BoxBottom.new(),
			BoxTape.new(),
		])

	func is_active() -> bool: 
		return true

var action_index = 0;
var actions: Array[String] = [
	"action_one",
	"action_two",
	"action_three",
	"action_four",
]

var screens: Array = [
	BoxScreen.new()
]

# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.

func handle_input_for_action(screen:Screen, action: ScreenListener):
	assert(action_index < actions.size(), "Showing more actions on screen than we have actions for")
	
	action.gamepad = actions[action_index]
	action_index += 1
		
	if Input.is_key_pressed(action.key) or Input.is_action_just_pressed(action.gamepad):
		action.action(screen)
		get_viewport().set_input_as_handled()
		return


func _unhandled_input(event):
	action_index = 0
	
	for screen in screens:
		if screen.is_not_active():
			continue
			
		for action in screen.config:
			handle_input_for_action(screen, action)
	
	if event is InputEventKey:
		if event.pressed and event.keycode == KEY_W:
			get_viewport().set_input_as_handled()

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass
