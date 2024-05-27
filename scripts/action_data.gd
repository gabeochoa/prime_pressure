class_name ActionData 

var name: String
var key: Key
var gamepad: String
var is_active: bool
var hide_label: bool

static func with_all(n: String, k: Key, g: String, active: bool):
	return (
		ActionData.new()
		.set_name(n)
		.set_key(k)
		.set_gamepad(g)
		.set_is_active(active)
	)

func set_name(n: String) -> ActionData : 
	name = n
	return self
	
func set_key(k: Key) -> ActionData: 
	key = k
	return self
	
func set_gamepad(g: String) -> ActionData: 
	gamepad = g
	return self
	
func set_active(ic: bool) -> ActionData: 
	is_active = ic
	return self
	
func set_hide_label(hl: bool) -> ActionData: 
	hide_label = hl
	return self

func get_image() -> Texture2D: 
	var image
	if InputSwitchHandler.instance().using_controller:
		image = get_image_for_gamepad() 
	else:
		image = get_image_for_key()
	return image

func get_image_for_key()-> Texture2D:
	const prefix = "res://graphics/controls/keyboard_default/"
	var make_keyboard = func(key_name): return prefix +  "keyboard_%s%s.png"% [key_name, "" if is_active else "_outline"]
	
	match key:
		var letter when key >= KEY_A and key <= KEY_Z:
			var pos :int = letter - KEY_A
			var character: int = 65 + pos
			var keyfile = make_keyboard.call(String.chr(character))
			#print(keyfile)
			return load(keyfile)
		KEY_ENTER:
			return load(make_keyboard.call("enter"))
		var other_key:
			assert(false, "Missing image for key %s" % other_key)
	return load("res://graphics/character_efb_mug.png")
	
func get_image_for_gamepad()-> Texture2D:
	const prefix = "res://graphics/controls/xbox_default/"
	var make_button= func(button_name): return prefix +  "xbox_button_%s%s.png"%[ button_name, "" if is_active else "_outline"]
	var make_= func(button_name): return prefix +  "xbox_%s%s.png"% [button_name, "" if is_active else "_outline"]
	match gamepad:
		"action_one":
			return load(make_button.call("a"))
		"action_two":
			return load(make_button.call("b"))
		"action_three":
			return load(make_button.call("x"))
		"action_four":
			return load(make_button.call("y"))
		"action_finish":
			return load(make_.call("rb"))
		var other_key:
			assert(false, "Missing image for gamepad %s action %s" % [name, other_key])
	return load("res://graphics/character_efb_mug.png")
