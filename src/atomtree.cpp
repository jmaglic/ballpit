
#include "atomtree.h"
#include "atom.h"

///////////////////
// AUX FUNCTIONS //
///////////////////

double findMaxRad(std::vector<Atom>& list_of_atoms);
void findAdjacentRecursive(std::vector<Atom*>&, const Atom&, const double& shell_to_shell_dist, const double& min_distance, const AtomNode* node, int dim);

//////////////
// ATOMNODE //
//////////////

// CONSTRUCTOR

AtomNode::AtomNode(int atom_id, AtomNode* left_node, AtomNode* right_node) 
    : _atom_id(atom_id), left_child(left_node), right_child(right_node) {}

// ACCESS

void AtomNode::setAtomList(const std::vector<Atom>& atom_list){
  AtomNode::_atom_list = atom_list;
}

std::vector<Atom>& AtomNode::getAtomList(){
  return _atom_list;
}

Atom& AtomNode::getAtom() const {
  return AtomNode::getAtom(_atom_id);
}

Atom& AtomNode::getAtom(const int atom_id){
  return _atom_list[atom_id];
}

int AtomNode::getAtomId() const {
  return _atom_id;
}

// OTHER
// for testing
void AtomNode::print(){
  std::cout << getAtom().symbol << "("
    << getAtom().getCoordinate(0) << "," 
    << getAtom().getCoordinate(1) << "," 
    << getAtom().getCoordinate(2) << ")";

  std::cout << "(-";
  if(left_child!=NULL){
    left_child->print();
  }
  std::cout << " +";
  if(right_child!=NULL){
    right_child->print();
  }
  std::cout << ")";
  return;
}

//////////////
// ATOMTREE //
//////////////

// CONSTRUCTOR

AtomTree::AtomTree(){
  _root = NULL;
  _max_rad = 0;
}

AtomTree::AtomTree(std::vector<Atom>& list_of_atoms){
  AtomNode::setAtomList(list_of_atoms);
  _root = buildTree(0, list_of_atoms.size(), 0);
  // ideally, the maximum radius would be the largest radius among all children of a node.
  // this, however, may require running an algorithm for every tree node, increasing the
  // complexity of the operation. it is much simpler to use the maximum radius among all
  // atoms instead, sacrificing optimisation.
  _max_rad = findMaxRad(list_of_atoms);
}

// FUNCTIONS USED BY CONSTRUCTOR

// recursive function to generate a 3-d tree from a list of atoms
// rather than copying the list of atoms in every recursion, or saving the partitioned
// vectors, the original list of atoms is passed by reference and only the vector limits
// are passed by value. any sorting during the tree building occurs in the original list
AtomNode* AtomTree::buildTree(
    int vec_first, // index of first vector element (default 0)
    int vec_end, // vector size (index of last element + 1)
    char dim){
  // if list of atoms has no atoms left
  if((vec_end-vec_first)==0){
    return NULL;
  }
  // if list of atoms has exactly one atom left
  else if((vec_end-vec_first)==1){
    return new AtomNode(vec_first,NULL,NULL);
  }

  else{
    quicksort(AtomNode::getAtomList(), vec_first, vec_end, dim);
    int median = vec_first + (vec_end-vec_first)/2; // operation rounds down
    return new AtomNode(
//        AtomNode::getAtomList()[median],
        median,
        buildTree(vec_first, median, (dim+1)%3), 
        buildTree(median+1, vec_end, (dim+1)%3)); 
  }
}

double findMaxRad(std::vector<Atom>& list_of_atoms){
  double max_rad = 0;
  for (int atom = 0; atom < list_of_atoms.size(); atom++){
    if (list_of_atoms[atom].getRad() > max_rad){
      max_rad = list_of_atoms[atom].getRad();
    }
  }
  return max_rad;
}
 
void AtomTree::quicksort(std::vector<Atom>& list_of_atoms, const int& vec_first, const int& vec_end, const char& dim){
  
  if(vec_first >= vec_end-1){
    return;
  }
    
  double pivot = list_of_atoms[vec_end-1].getCoordinate(dim);
  
  int cntr = vec_first;

  for(int i = vec_first; i < vec_end; i++){
    if(list_of_atoms[i].getCoordinate(dim) <= pivot){
      swap(list_of_atoms[cntr], list_of_atoms[i]);
      cntr++;
    }
  }
  quicksort(list_of_atoms, vec_first, cntr-1, dim);
  quicksort(list_of_atoms, cntr, vec_end, dim);

  return;
}

void AtomTree::swap(Atom& a, Atom& b){
  Atom temp = a;
  a = b;
  b = temp;
  return;
}

// for testing
void AtomTree::print() const {
  std::cout << "Printing Tree" << std::endl;
  if(_root == NULL){
    std::cout << "Tree empty" << std::endl;
  }
  else{
    _root->print();
  }
  std::cout << std::endl;
  return;
}

// ACCESS

const double AtomTree::getMaxRad() const {
  return _max_rad;
}

const AtomNode* AtomTree::getRoot() const {
  return _root;
}

// NEIGHBOUR LIST

// changes to AtomNode and AtomTree neccessitate a reevaluation of this function
std::vector<Atom*> AtomTree::findAdjacent(const Atom& at, const double& shell_to_shell_dist){
  std::vector<Atom*> list_of_adjacent;
  int dim = 0;
  double min_distance = at.rad + _max_rad + shell_to_shell_dist;

  findAdjacentRecursive(list_of_adjacent, at, shell_to_shell_dist, min_distance, _root, dim);
  
  return list_of_adjacent;
}

// changes to AtomNode and AtomTree neccessitate a reevaluation of this function
void findAdjacentRecursive(
    std::vector<Atom*>& list_of_adjacent, 
    const Atom& atom,
    const double& shell_to_shell_dist,
    const double& min_distance,
    const AtomNode* node, 
    int dim){
  
  if (node == NULL) {return;}
  Atom* test_atom = &node->getAtom(); // for easier access
  double dist1D = distance(test_atom->getPos(), atom.getPos(), dim);
  
  if (abs(dist1D) > min_distance){ // if atom is very far from test atom
    findAdjacentRecursive(
        list_of_adjacent,
        atom,
        shell_to_shell_dist,
        min_distance,
        dist1D < 0 ? node->left_child : node->right_child,
        (dim+1)%3);
  }
  else{ // if atom is close enough to test atom that is could be adjacent
    double dist_at_at = distance(atom.getPos(), test_atom->getPos());
    if (dist_at_at < (atom.rad + test_atom->rad + shell_to_shell_dist)){ // if test atom is adjacent
      if (&atom != test_atom){ // if its not the same atom
        list_of_adjacent.push_back(test_atom);
      }
    }
    for (AtomNode* child : {node->left_child, node->right_child}){
      findAdjacentRecursive(list_of_adjacent, atom, shell_to_shell_dist, min_distance, child, (dim+1)%3);
    }
  }
}

