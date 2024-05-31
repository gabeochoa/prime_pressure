class_name OrderQueueItem extends Control

@onready var action_label = %ActionLabel
@onready var state_icon = %StateIcon


var label: String = "" 
var icon: Texture2D= null

func with_data(new_name: String, new_label: String, new_icon: Texture2D) -> OrderQueueItem :
	self.name = new_name
	self.label = new_label
	self.icon= new_icon
	return self

# Called when the node enters the scene tree for the first time.
func _ready():
	update_ui()

func update_ui():
	#print("update action label ui ", data.name)
	set_custom_minimum_size(Vector2(200, 100))
	if icon != null:
		state_icon.add_image(icon, 100, 100)
	action_label.add_text(label)

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	pass
