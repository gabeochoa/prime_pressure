class_name ActionLabel extends Control

@onready var richLabel: RichTextLabel = %RichTextLabel

var data: ActionData

func with_data(action_data: ActionData) -> ActionLabel :
	data = action_data
	return self

# Called when the node enters the scene tree for the first time.
func _ready():
	update_ui()

func update_ui():
	print("update action label ui ", data.name)
	set_custom_minimum_size(Vector2(200, 100))
	var image = data.get_image()
	richLabel.add_image(image, 100, 100)
	richLabel.add_text(data.name)
	#if !data.hide_label:
		#richLabel.add_text(data.name)

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	pass
