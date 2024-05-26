class_name ActionLabel extends RichTextLabel

func get_image_for_key(key: Key)-> Texture2D:
	const prefix = "res://graphics/controls/keyboard_default/"
	var make_keyboard = func(name): return prefix +  "keyboard_%s.png"%name
	
	match key:
		var letter when key >= KEY_A and key <= KEY_Z:
			var pos :int = key - KEY_A
			var character: int= 65 + pos
			var keyfile = make_keyboard.call(String.chr(character))
			print(keyfile)
			return load(keyfile)
		KEY_ENTER:
			return load(make_keyboard.call("enter"))
		var other_key:
			assert(false, "Missing image for key %s" % other_key)
	return load("res://graphics/character_efb_mug.png")
	
func get_image_for_gamepad(gamepad: String)-> Texture2D:
	const prefix = "res://graphics/controls/xbox_default/"
	var make_keyboard = func(name): return prefix +  "xbox_button_%s.png"%name
	match gamepad:
		"action_one":
			return load(make_keyboard.call("a"))
		"action_two":
			return load(make_keyboard.call("x"))
		"action_three":
			return load(make_keyboard.call("b"))
		"action_four":
			return load(make_keyboard.call("y"))
	return load("res://graphics/character_efb_mug.png")
	
var key: Key
var gamepad: String

var use_controller: bool

func _init(n: String, k: Key, g: String, is_controller: bool):
	name = n
	key = k
	gamepad = g
	use_controller = is_controller

# Called when the node enters the scene tree for the first time.
func _ready():
	print("create action label")
	update_ui()
	pass # Replace with function body.

func update_ui():
	set_custom_minimum_size(Vector2(200, 100))
	var image = get_image_for_gamepad(gamepad) if use_controller else get_image_for_key(key)
	add_image(image, 100, 100)
	add_text(name)

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass
