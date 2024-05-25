extends Node2D

class Shop extends ScreenListener:
	func _init():
		super("shop", KEY_S)
		
	func is_active() -> bool: 
		return true
		
	func action(): 
		print("Shop action")

class Shop2 extends Shop:
	func action(): 
		print("two action")
		
class Shop3 extends Shop:
	func action(): 
		print("three action")
	
		
var config: Array[ScreenListener]= [
	Shop.new(),
	Shop2.new(),
	Shop3.new(),
]

var action_index = 0;
var actions: Array[String] = [
	"action_one",
	"action_two",
	"action_three",
	"action_four",
]

# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.

func _unhandled_input(event):
	action_index = 0
	
	for screen in config:
		if screen.is_not_active():
			continue
			
		assert(action_index < actions.size(), "Showing more actions on screen than we have actions for")
		
		screen.gamepad = actions[action_index]
		action_index += 1
		
			
		if Input.is_key_pressed(screen.key) or Input.is_action_just_pressed(screen.gamepad):
			screen.action()
			get_viewport().set_input_as_handled()
			break;
	
	if event is InputEventKey:
		if event.pressed and event.keycode == KEY_W:
			get_viewport().set_input_as_handled()

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass
