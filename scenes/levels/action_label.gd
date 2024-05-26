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
	
var key: Key
var gamepad: String

func _init(n: String, k: Key, g: String):
	name = n
	key = k
	gamepad = g
	
# Called when the node enters the scene tree for the first time.
func _ready():
	print("create action label")
	set_custom_minimum_size(Vector2(200, 100))
	add_image(get_image_for_key(key), 100, 100)
	add_text(name)
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass
