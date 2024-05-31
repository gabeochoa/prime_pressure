extends Control

@export var percentage: float = 0

func update_pct(pct: float):
	percentage = pct
	queue_redraw()

func draw_filled_arc(center, radius, angle_from, angle_to, color):
	var nb_points = 32
	var points_arc = PackedVector2Array()
	points_arc.push_back(center)
	var colors = PackedColorArray([color])

	for i in range(nb_points + 1):
		var angle_point = deg_to_rad(angle_from + i * (angle_to - angle_from) / nb_points - 90)
		points_arc.push_back(center + Vector2(cos(angle_point), sin(angle_point)) * radius)
	draw_polygon(points_arc, colors)

func _draw(): 
	var center = Vector2(100, 75)
	var radius = 25
	var angle_from = 0
	var angle_to = 360 * percentage
	var color =  Color.RED
	draw_circle(center, radius, Color.WHITE)
	draw_filled_arc(center, radius, angle_from, angle_to, color)
