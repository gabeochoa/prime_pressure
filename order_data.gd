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

const items: Dictionary = {
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
	
var order_items: Array[Item]


var example_order = [
	Item.new().set_name(ItemType.Shampoo).set_amount(1),
	Item.new().set_name(ItemType.Chips).set_amount(2),
	Item.new().set_name(ItemType.ToiletPaper).set_amount(10)
]
