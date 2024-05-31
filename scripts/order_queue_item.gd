class_name OrderQueueItem extends Control

@onready var action_label = %ActionLabel
@onready var state_icon = %StateIcon
@onready var clock = $Clock


var label: String = "" 
var icon: Texture2D= null
var percentage: float = 0

func with_data(new_name: String, new_label: String, new_icon: Texture2D, new_pct: float ) -> OrderQueueItem :
	self.name = new_name
	self.label = new_label
	self.icon= new_icon
	self.percentage = new_pct
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
	clock.percentage = percentage

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	pass
