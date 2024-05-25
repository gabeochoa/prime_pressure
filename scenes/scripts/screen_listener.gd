class_name ScreenListener 
extends Node

var key: Key
var gamepad: String

func _init(n: String, k:Key):
	name = n
	key = k
		
func is_active() -> bool: 
	assert(false, "class " + name + " needs to implement is_active()")
	return false
	
func action(): 
	assert(false, "class " + name + " needs to implement action()")

func is_not_active() -> bool:
	return !is_active()


	
# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass
