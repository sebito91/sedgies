// +build linux

package avl

// Node is the default instance of our struct
type Node struct {
	Value   int
	Height  int
	Left    *Node
	Right   *Node
	Sibling *Node
}

// New instantiates a new instance of an AVL node
func New() *Node {
	return &Node{}
}

// Insert the provided value into the AVL
func Insert(val int, n *Node) (*Node, error) {
	if n == nil {
		n = &Node{Value: val}
	}

	n.Height = max(height(n.Left), height(n.Right)) + 1

	return nil, nil
}

// height is a helper that returns the current AVL distance
func height(n *Node) int {
	if n == nil {
		return -1
	}
	return n.Height
}

// max is a helper to return the max difference between values
// NOTE the math stdlib contains a function to compare floats, but not ints :/
func max(l, r int) int {
	if l > r {
		return l
	}
	return r
}
