extends Node2D

class InputSwitchHandler : 
	var previous: bool = true
	var using_controller: bool = true
	func update_if_switched_input_type(event): 
		previous = using_controller
		if event is InputEventJoypadButton:
			using_controller = true
			#print("using controller")
		if event is InputEventKey:
			using_controller = false
			#print("using keyboard")
	func changed_since_last_input() -> bool:
		return previous != using_controller
	
var controller_handler: InputSwitchHandler = InputSwitchHandler.new()

var screens: Array = [
	BoxScreen.new(),
	OrderScreen.new(),
]

@export var action_label_scene: PackedScene
@onready var screen_container: GridContainer =  %GridContainer

# this cant be Array[Screen] because godot deletes the inner type idk
func get_active_screens() -> Array:
	return screens.filter(func(screen): return screen.is_active())

func _unhandled_input(event):
	ActionManager.instance().reset_count()
	
	controller_handler.update_if_switched_input_type(event)
	
	for screen in get_active_screens():
		var action = screen.active_action
		if action.is_active():
			if controller_handler.changed_since_last_input():
				screen.is_dirty = true
			if !screen.is_active_action(action):
				continue
				
			if Input.is_key_pressed(action.data.key) or Input.is_action_just_pressed(action.data.gamepad):
				print("action ", action.data.name, action.data.gamepad)
				action.action(screen)
				get_viewport().set_input_as_handled()
				return
	
func create_action_button(screen: Screen, action: ActionListener) -> Control :
	ActionManager.instance().assign_input_if_missing(action)

	var label = action_label_scene.instantiate().with_data(
		action.data
		.set_is_controller(controller_handler.using_controller)
		.set_active(screen.is_active_action(action))
	)
	
	if action.data.hide_label:
		label.data.hide_label = true
		
	var wrapper = MarginContainer.new()
	wrapper.add_child(label)
	wrapper.add_theme_constant_override("theme_override_constants/margin_left", 100)
	return wrapper
		
func render_actions():
	var children = screen_container.get_children();
	
	var actives = get_active_screens()
	if actives.size() == 0:
		for child in children:
			child.queue_free()
		return
		
	var active_screen = actives[0]
	if !active_screen.is_dirty:
		return
	active_screen.is_dirty = false
	
	for child in children:
		child.queue_free()
	
	screen_container.columns = active_screen.num_items_per_row

	active_screen.config.for_each_active(
		func (action): 
			screen_container.add_child(
				create_action_button(active_screen, action)
			)
	)

func _process(_delta):
	render_actions()
	pass

class BoxScreen extends SequentialScreen: 
	class BoxUnfold extends SequentialAction:
		func _init():
			super(
				ActionData.new()
				.set_name("unfold")
				.set_key(KEY_U)
			)

	class BoxBottom extends SequentialAction:
		func _init():
			super(
				ActionData.new()
				.set_name("bottom")
				.set_key(KEY_B)
			)

	class BoxTape extends SequentialAction:
		func _init():
			super(
				ActionData.new()
				.set_name("tape")
				.set_key(KEY_T)
			)
			
	func _init():
		super("box", ActionGroup.new().add_actions([
			BoxUnfold.new(),
			BoxBottom.new(),
			BoxTape.new(),
			CompleteAction.new()
		]))
		next(true)
		

class OrderScreen extends SequentialScreen: 
	class OrderLetter extends ActionListener:
		func _init(key: Key):
			super(
				ActionData.new()
				.set_name("order_letter_%s" % key)
				.set_key(key)
				.set_hide_label(true)
			)
			
		func action(screen:Screen): 
			print(data.name)
			screen.next()
	
	func create_order_letter(item: String):
		var list: Array[ActionListener] = []
		for character in item: 
			var char_as_int: int = character.to_ascii_buffer()[0] - "A".to_ascii_buffer()[0]
			var key: Key = (char_as_int as Key) + KEY_A
			print(" idk char %s %s %s key %s" % [character, character.to_ascii_buffer()[0], char_as_int, key])
			list.append(OrderLetter.new(key))
		return list
	
	func _init():
		num_items_per_row = 1
		var orders = create_order_letter("SHAM")
		orders.append(CompleteAction.new())
		var group = (
			ActionGroup.new()
			.add_actions(orders)
		)
		super("order", group)
		next(true)
