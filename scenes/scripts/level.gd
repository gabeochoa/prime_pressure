extends Node2D

func _unhandled_input(event):
	if event.is_action_pressed("ui_cancel") or Input.is_action_pressed("menu_back"): 
		get_tree().quit()

# Called when the node enters the scene tree for the first time.
func _ready():
	pass

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	if $logo.position.x > 1920: 
		$logo.pos.x = 0

	# Input.is_action_just_pressed("key") 294 to see if it was just pressed,
	# Input.is_action_pressed("key") 237 to see if it is still pressed.
