extends Sprite2D

var pos: Vector2 = Vector2.ZERO
const speed: int = 10

func _unhandled_input(event):
	if event is InputEventKey:
		if event.pressed and event.keycode == KEY_W:
			pos.y += 10


# Called when the node enters the scene tree for the first time.
func _ready():
	pos = Vector2(100, 200);


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	position = pos
