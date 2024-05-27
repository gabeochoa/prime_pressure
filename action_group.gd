class_name ActionGroup

var children: Array[ActionGroup]
var actions: Array[ActionListener]
var is_vertical: bool = true

var action_index: int = 0
var children_index: int =0 

func add_actions(acts: Array[ActionListener]) -> ActionGroup:
	actions = acts
	return self
	
func add_child(child: ActionGroup) -> ActionGroup:
	children.append(child)
	return self

func set_vertical(v: bool) -> ActionGroup:
	is_vertical = v
	return self

func has_next() -> bool: 
	if action_index < actions.size(): return true
	return children_index < children.size() and children[children_index].has_next()
	
func for_each(cb: Callable, from_beginning: bool = false, active_only: bool = false) -> void: 
	var _old_action_index = action_index
	var _old_children_index = children_index
	
	if from_beginning:
		action_index = 0
		children_index = 0
	
	# this would be a Array[Array[int]] but nested types arent allowed 
	var children_indexes = []
	for child in children: 
		children_indexes.append(
			[
				child.action_index,
				child.children_index
			]
		)
	
	while(true):
		var action = get_current_action()
		increment_action_index()
		if action == null or action.is_empty:
			break
		if active_only and action.is_not_active(): 
			continue
		cb.call(action)
	
	action_index = _old_action_index
	children_index = _old_children_index
	
	var i = 0
	for child in children: 
		child.action_index = children_indexes[i][0]
		child.children_index= children_indexes[i][1]
		i+=1
	
func for_all(cb: Callable) -> void: 
	for_each(cb, true, false)
	
func for_each_active(cb: Callable) -> void: 
	for_each(cb, false, true)

func get_current_child_action() -> ActionListener: 
	if !children.size(): return ActionListener.get_empty()
	if children_index >= children.size(): return ActionListener.get_empty()
	return children[children_index].get_current_action()
	
func get_current_action() -> ActionListener: 
	if action_index >= actions.size():
		return get_current_child_action()
	return actions[action_index]
	
func increment_action_index(): 
	action_index += 1
	if action_index < actions.size(): return
	if !children.size(): return 
	
	var child = children[children_index]
	if child.has_next(): 
		child.increment_action_index()
		return
	children_index += 1
