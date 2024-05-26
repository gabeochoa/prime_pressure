class_name ActionLabel extends RichTextLabel

var key: Key
var gamepad: String

func _init(n: String, k: Key, g: String):
	name = n
	key = k
	gamepad = g
	
# Called when the node enters the scene tree for the first time.
func _ready():
	print("create action label")
	set_custom_minimum_size(Vector2(100, 100))
	set_text(name)
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass
