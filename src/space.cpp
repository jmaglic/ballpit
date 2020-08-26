
#include "space.h"
#include "atom.h"
#include "voxel.h"
#include "atomtree.h"
#include <cmath>
#include <cassert>

/////////////////
// VOLUME COMP //
/////////////////

void Space::placeAtomsInGrid(const std::vector<Atom> &atoms, const AtomTree& atomtree){ // TODO: remove atoms argument. no longer necessary
  // calculate position of first voxel
  std::array<double,3> vxl_origin = getOrigin();
  
  // calculate side length of top level voxel
  double vxl_dist = grid_size * pow(2,max_depth);

  // origin of the cell has to be offset by half the grid size
  for(int dim = 0; dim < 3; dim++){
    vxl_origin[dim] += vxl_dist/2;
  }
  
  for(size_t x = 0; x < n_gridsteps[0]; x++){
    for(size_t y = 0; y < n_gridsteps[1]; y++){
      for(size_t z = 0; z < n_gridsteps[2]; z++){
        Voxel& vxl = getElement(x,y,z);
        std::array<double,3> vxl_pos =
          {vxl_origin[0] + vxl_dist * x, vxl_origin[1] + vxl_dist * y, vxl_origin[2] + vxl_dist * z};
        vxl.determineType(vxl_pos, grid_size, max_depth, atomtree);
      }
    }
  }      
  return;
}

double Space::getVolume(){
  // calc Volume
  size_t total = 0;
  for(size_t i = 0; i < n_gridsteps[0] * n_gridsteps[1] * n_gridsteps[2]; i++){ // loop through all top level voxels
    // tally bottom level voxels
    total += getElement(i).tallyVoxelsOfType('a',max_depth);
  }
  double unit_volume = pow(grid_size,3);
  return unit_volume * total;
}

//////////////////////
// ACCESS FUNCTIONS //
//////////////////////

std::array<double,3> Space::getMin(){
  return cart_min;
}

std::array<double,3> Space::getOrigin(){
  return getMin();
}

std::array<double,3> Space::getMax(){
  return cart_max;
}

std::array<double,3> Space::getSize(){
  std::array<double,3> size;
  for(int dim = 0; dim < 3; dim++){
    size[dim] = cart_max[dim] - cart_min[dim];
  }
  return size;
}

void Space::treetomatrix(std::vector<char> &matrix, Voxel& toplevel, int offx, int offy, int offz, int dimx, int dimy, int dimz){
	for (int i= 0;i<8;++i){
		short xhalf = i%2;
		short yhalf = i<=1||i==4||i==5;
		short zhalf = i<4;
		auto type = toplevel.getType();
		if (type=='m'){
			auto element = toplevel.get(xhalf,yhalf,zhalf);
			treetomatrix(matrix,
						 element,
						 offx+dimx*xhalf,
						 offy+dimy*yhalf,
						 offz+dimz*zhalf,
						 dimx/2, dimy/2, dimz/2);
		} else if(type=='e'){
		   int dim = this->getResolution()[0];
		   for (int z=offz; z<dimz; ++z){
			   for (int y=offy;y<dimy; ++y){
				   for (int x=offx; x<dimx; ++x){
					   matrix[z*dim*dim+y*dim+x] = 0;
				   }
			   }
		   }
		} else {
			int dim = this->getResolution()[0];
			for (int z=offz;z<dimz;++z){
				for (int y=offy;y<dimy;++y){
					for (int x=offx;x<dimx;++x){
						matrix[z*dim*dim+y*dim+x] = 1;
					}
				}
			}
		}
	}
}


std::vector<char> Space::getMatrix(){
	int dim = this->getResolution()[0];//assume uniform size
	//target matrix for tree to vector conversion
	std::vector<char> gridmatrix;
	gridmatrix.resize(pow(dim,3));
	
	//TODO positions needs to changed per voxel like in space::placeAtomsInGrid
	for(auto griditem : grid){
		treetomatrix(gridmatrix, griditem, 0, 0, 0, dim, dim, dim);
	}
	
	//set some fake data
	int i=0;
	for (auto iter = gridmatrix.begin();iter != gridmatrix.end();++iter){
		int x = i % dim-dim/2;
		int y = (i % (dim*dim))/dim-dim/2;
		int z = i / (dim*dim)-dim/2;
		if ((x<20 && x>2 && y<10 && y>4 && z<20 && z>3)){
			*iter = 1;
		}
		++i;
	}
	
	return gridmatrix;
}

//the number of voxels at lowest tree level in one dimension
std::array<double,3> Space::getResolution(){
	std::array<double,3> size;
	for(int dim = 0; dim < 3; dim++){
		size[dim] = n_gridsteps[dim]*pow(2,max_depth);
	}
  return size;
}

Voxel& Space::getElement(const size_t &i){
  assert(i < n_gridsteps[0] * n_gridsteps[1] * n_gridsteps[2]);
  return grid[i];
}

Voxel& Space::getElement(const size_t &x, const size_t &y, const size_t &z){
  // check if element is out of bounds
  assert(x < n_gridsteps[0]);
  assert(y < n_gridsteps[1]);
  assert(z < n_gridsteps[2]);
  return grid[z * n_gridsteps[0] * n_gridsteps[1] + y * n_gridsteps[0] + x];
}

// displays voxel grid types as matrix in the terminal. useful for debugging
void Space::printGrid(){
  char usr_inp = '\0';
  int x_min = 0;
  int y_min = 0;
  
  auto x_max = ((n_gridsteps[0] >= 25)? 25: n_gridsteps[0]);
  auto y_max = ((n_gridsteps[1] >= 25)? 25: n_gridsteps[1]);

  size_t z = 0;
  std::cout << "Enter 'q' to quit; 'w', 'a', 's', 'd' for directional input; 'c' to continue in z direction; 'r' to go back in z direction" << std::endl;
  while (usr_inp != 'q'){

    // x and y coordinates
    if (usr_inp == 'a'){x_min--; x_max--;}
    else if (usr_inp == 'd'){x_min++; x_max++;}
    else if (usr_inp == 's'){y_min++; y_max++;}
    else if (usr_inp == 'w'){y_min--; y_max--;}
    
    if (x_min < 0){x_min++; x_max++;}
    if (y_min < 0){y_min++; y_max++;}
    if (x_max > n_gridsteps[0]){x_min--; x_max--;}
    if (y_max > n_gridsteps[1]){y_min--; y_max--;}
    
    // z coordinate
    if (usr_inp == 'r'){
      if (z==0){
        std::cout << "z position at min. ";
      }
      else{z--;}
    }
    else if (usr_inp == 'c'){
      z++;
      if (z >= n_gridsteps[2]){
        z--;
        std::cout << "z position at max. ";
      }
    }
    
    // print matrix
    for(size_t y = y_min; y < y_max; y++){
      for(size_t x = x_min; x < x_max; x++){
        std::cout << getElement(x,y,z).getType();
      }
      std::cout << std::endl;
    }
    
    // get user input
    std::cout << std::endl;
    std::cout << "INPUT: ";
    std::cin >> usr_inp;
  }
  return;
}


/////////////////
// CONSTRUCTOR //
/////////////////

Space::Space(std::vector<Atom> &atoms, const double& bottom_level_voxel_dist, const int& depth)
  :grid_size(bottom_level_voxel_dist), max_depth(depth){
  setBoundaries(atoms);
  setGrid();
}

///////////////////////////////
// FUNCTIONS FOR CONSTRUCTOR //
///////////////////////////////

// member function loops through all atoms (previously extracted
// from file) and saves the minimum and maximum positions in all
// three cartesian directions out of all atoms. Also finds max
// radius of all atoms and sets the space boundaries slightly so
// that all atoms fit the space.
//void Space::findMinMaxFromAtoms
void Space::setBoundaries
  (std::vector<Atom> &atoms)
{
  double max_radius = 0;
  for(int at = 0; at < atoms.size(); at++){
    std::array<double,3> atom_pos = {atoms[at].pos_x, atoms[at].pos_y, atoms[at].pos_z}; // atom positions are correct
  
    // if we are at the first atom, then the atom position is min/max by default
    if(at == 0){
      cart_min = atom_pos;
      cart_max = atom_pos;
      max_radius = atoms[at].rad;
    }
    
    // after the first atom, begin comparing the atom positions to min/max and save if exceeds previously saved values
    else{
      for(int dim = 0; dim < 3; dim++){ 
        if(atom_pos[dim] > cart_max[dim]){ 
          cart_max[dim] = atom_pos[dim];
        }
        if(atom_pos[dim] < cart_min[dim]){
          cart_min[dim] = atom_pos[dim];
        }
        if(atoms[at].rad > max_radius){
          max_radius = atoms[at].rad;
        }
      }
    }
  }
  // expand boundaries by a little more than the largest atom found
  for(int dim = 0; dim < 3; dim++){ 
    cart_min[dim] -= (1.1 * max_radius);
    cart_max[dim] += (1.1 * max_radius);    
  }
  return;
}

// based on the grid step and the octree max_depth, this function produces a
// 3D grid (in form of a 1D vector) that contains all top level voxels.
void Space::setGrid(){
  size_t n_voxels = 1;
  
  for(int dim = 0; dim < 3; dim++){
    n_gridsteps[dim] = std::ceil (std::ceil( (getSize())[dim] / grid_size ) / std::pow(2,max_depth) );
    n_voxels *= n_gridsteps[dim];
  }
  
  for(size_t i = 0; i < n_voxels; i++){
    grid.push_back(Voxel());
  }
  
  return;
}

