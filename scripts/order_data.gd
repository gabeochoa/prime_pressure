class_name OrderData 

enum State {
	New,
	Procure,
	Pack,
	Ship,
}

enum ItemType {
	Shampoo,
	Chips, 
	ToiletPaper
} 

static var items: Dictionary = {
	ItemType.Shampoo: {
		"en_name": "Shampoo",
		"keys": "SHAM"
	},
	ItemType.Chips: {
		"en_name": "Chips",
		"keys": "CHIP"
	},
	ItemType.ToiletPaper: {
		"en_name": "Toilet Paper",
		"keys": "TOIL"
	}
}

class Item:
	var type: ItemType
	var amount: int
	
	func set_type(t: ItemType):
		type = t
		return self
	
	func set_amount(amt: int):
		amount = amt
		return self
	
var queue_position: int = -1
var state: State = State.New
var ran_state: bool = false
var order_items: Array[Item]

func update_state(new_state: State):
	state = new_state
	return self

func get_items():
	return order_items
	
func set_items(items: Array[Item]) -> OrderData:
	order_items = items
	return self


static func make_example_order() -> Array[Item]:
	var example_order: Array[Item]= [
		Item.new().set_type(ItemType.Shampoo).set_amount(1),
		Item.new().set_type(ItemType.Chips).set_amount(2),
		Item.new().set_type(ItemType.ToiletPaper).set_amount(10)
	]
	example_order.shuffle()
	return example_order

static func make_new_order() -> OrderData: 
	return (
		OrderData.new()
		.set_items(OrderData.make_example_order())
		.update_state(State.New)
	)
