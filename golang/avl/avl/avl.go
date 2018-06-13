// +build linux

package avl

import "fmt"

// Node is the default instance of our struct
type Node struct {
	Value  int
	Height int
	Left   *Node
	Right  *Node
}

// New instantiates a new instance of an AVL node
func New() *Node {
	return &Node{}
}

// Insert the provided value into the AVL
func Insert(val int, n *Node) *Node {
	if n == nil {
		// insert a new root
		n = &Node{Value: val}
	} else if val < n.Value {
		n.Left = Insert(val, n.Left) // recurse...

		if height(n.Left)-height(n.Right) == 2 {
			if val < n.Left.Value {
				n = singleRotateLeft(n)
			} else {
				n = doubleRotateLeft(n)
			}
		}
	} else if val > n.Value {
		n.Right = Insert(val, n.Right)

		if height(n.Right)-height(n.Left) == 2 {
			if val > n.Right.Value {
				n = singleRotateRight(n)
			} else {
				n = doubleRotateRight(n)
			}
		}
	}

	n.Height = max(height(n.Left), height(n.Right)) + 1

	return n
}

// singleRotateLeft performs a rotation between n and its left child
// NOTE: call singleRotateLeft only if n has a left child
func singleRotateLeft(n *Node) *Node {
	t := &Node{}

	t = n.Left
	n.Left = t.Right
	t.Right = n

	n.Height = max(height(n.Left), height(n.Right)) + 1
	t.Height = max(height(t.Left), n.Height) + 1

	return t
}

// singleRotateRight performs a rotation between n and its right child
// NOTE: call singleRotateRight only if n has a right child
func singleRotateRight(n *Node) *Node {
	t := &Node{}

	t = n.Right
	n.Right = t.Left
	t.Left = n

	n.Height = max(height(n.Left), height(n.Right)) + 1
	t.Height = max(height(t.Right), n.Height) + 1

	return t
}

// doubleRotateLeft handles the AVL left-right double rotation
func doubleRotateLeft(n *Node) *Node {
	n.Left = singleRotateRight(n.Left)
	return singleRotateLeft(n)
}

// doubleRotateRight handles the AVL right-left double rotation
func doubleRotateRight(n *Node) *Node {
	n.Right = singleRotateLeft(n.Right)
	return singleRotateRight(n)
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

// DumpAVL will dump out the contents of the tree
func (n *Node) DumpAVL() {
	if n == nil {
		return
	}

	fmt.Printf("%d\n", n.Value)
	if n.Left != nil {
		fmt.Printf("(L:%d)\n", n.Left.Value)
	}
	if n.Right != nil {
		fmt.Printf("(R:%d)\n", n.Right.Value)
	}
	fmt.Printf("(h: %d)\n", n.Height)

	n.Left.DumpAVL()
	n.Right.DumpAVL()
}
