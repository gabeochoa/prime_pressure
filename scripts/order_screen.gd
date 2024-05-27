class_name OrderScreen extends SequentialScreen

class OrderLetter extends ActionListener:
	func _init(parent:String, key: Key):
		super(
			ActionData.new()
			.set_name("order_letter_%s_%s" % [parent, key])
			.set_key(key)
			.set_hide_label(true)
		)
		
	func action(screen:Screen): 
		screen.next()

func create_order_letter(item: String):
	var list: Array[ActionListener] = []
	for character in item: 
		var char_as_int: int = character.to_ascii_buffer()[0] - "A".to_ascii_buffer()[0]
		var key: Key = (char_as_int as Key) + KEY_A
		#print(" idk char %s %s %s key %s" % [character, character.to_ascii_buffer()[0], char_as_int, key])
		list.append(OrderLetter.new(item, key))
	return list

func _init(order: OrderData):
	num_items_per_row = 4
	var order_actions: Array[ActionListener] = []
	for item in order.get_items():
		var type = item.type
		var item_data = OrderData.items[type]
		order_actions.append_array(create_order_letter(item_data.keys))
		# TODO add a number action for how many items they are buying 
		# i tried just adding it again but typing TOIL 10 times is not fun
		
	order_actions.append(CompleteAction.new())
	var group = (
		ActionGroup.new()
		.add_actions(order_actions)
	)
	super("order", group)
	next(true)
