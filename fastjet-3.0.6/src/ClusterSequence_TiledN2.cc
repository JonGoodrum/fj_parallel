//STARTHEADER
// $Id: ClusterSequence_TiledN2.cc 3213 2013-09-16 06:59:06Z soyez $
//
// Copyright (c) 2005-2011, Matteo Cacciari, Gavin P. Salam and Gregory Soyez
//
//----------------------------------------------------------------------
// This file is part of FastJet.
//
// FastJet is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// The algorithms that underlie FastJet have required considerable
// development and are described in hep-ph/0512210. If you use
// FastJet as part of work towards a scientific publication, please
// include a citation to the FastJet paper.
//
// FastJet is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FastJet. If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------
//ENDHEADER


// The plain N^2 part of the ClusterSequence class -- separated out
// from the rest of the class implementation so as to speed up
// compilation of this particular part while it is under test.

#include<iostream>
#include<vector>
#include<cmath>
#include<algorithm>
#include "fastjet/PseudoJet.hh"
#include "fastjet/ClusterSequence.hh"
#include "fastjet/internal/MinHeap.hh"

FASTJET_BEGIN_NAMESPACE // defined in fastjet/internal/base.hh

using namespace std;

//----------------------------------------------------------------------
void ClusterSequence::_bj_remove_from_tiles(TiledJet * const jet) {
  Tile * tile = & ((*_tiles)[jet->tile_index]); //**//

  if (jet->previous == NULL) {
    // we are at head of the tile, so reset it.
    // If this was the only jet on the tile then tile->head will now be NULL
    tile->head = jet->next;
  } else {
    // adjust link from previous jet in this tile
    jet->previous->next = jet->next;
  }
  if (jet->next != NULL) {
    // adjust backwards-link from next jet in this tile
    jet->next->previous = jet->previous;
  }
}

//----------------------------------------------------------------------
/// Set up the tiles:
/// - decide the range in eta
/// - allocate the tiles
/// - set up the cross-referencing info between tiles
///
/// The neighbourhood of a tile is set up as follows
///
/// LRR
/// LXR
/// LLR
///
/// such that tiles is an array containing XLLLLRRRR with pointers
/// | \ RH_tiles
/// \ surrounding_tiles
///
/// with appropriate precautions when close to the edge of the tiled
/// region.
///
void ClusterSequence::_initialise_tiles() {

 int baba = 0;
 //std::cout << "1\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
 if (tman->getFirst()) { //**//
 //std::cout <<  "2\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
  // first decide tile sizes (with a lower bound to avoid huge memory use with
  // very small R)
  double default_size = max(0.1,_Rparam);
 //std::cout <<  "3\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
  _tile_size_eta = default_size;
 //std::cout <<  "4\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
  // it makes no sense to go below 3 tiles in phi -- 3 tiles is
  // sufficient to make sure all pair-wise combinations up to pi in
  // phi are possible
  _n_tiles_phi = max(3,int(floor(twopi/default_size)));
 //std::cout <<  "5\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
  _tile_size_phi = twopi / _n_tiles_phi; // >= _Rparam and fits in 2pi
 //std::cout <<  "6\n"; baba++;/////////////////////////////////////////////////////////////////////////////////

  // always include zero rapidity in the tiling region
  _tiles_eta_min = 0.0;
 //std::cout <<  "7\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
  _tiles_eta_max = 0.0;
 //std::cout <<  "8\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
  // but go no further than following
  const double maxrap = 7.0;
 //std::cout <<  "9\n"; baba++;/////////////////////////////////////////////////////////////////////////////////

  // and find out how much further one should go
  for(unsigned int i = 0; i < _jets.size(); i++) {
 //std::cout <<  "10\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
    double eta = _jets[i].rap();
 //std::cout <<  "11\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
    // first check if eta is in range -- to avoid taking into account
    // very spurious rapidities due to particles with near-zero kt.
    if (abs(eta) < maxrap) {
 //std::cout <<  "12\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
      if (eta < _tiles_eta_min) {_tiles_eta_min = eta;}
 //std::cout <<  "13\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
      if (eta > _tiles_eta_max) {_tiles_eta_max = eta;}
 //std::cout <<  "14\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
    }
  }

  // now adjust the values
  _tiles_ieta_min = int(floor(_tiles_eta_min/_tile_size_eta));
 //std::cout <<  "15\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
  _tiles_ieta_max = int(floor( _tiles_eta_max/_tile_size_eta));
 //std::cout <<  "16\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
  _tiles_eta_min = _tiles_ieta_min * _tile_size_eta;
 //std::cout <<  "17\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
  _tiles_eta_max = _tiles_ieta_max * _tile_size_eta;
 //std::cout <<  "18\n"; baba++;/////////////////////////////////////////////////////////////////////////////////

  // allocate the tiles
  _tiles->resize((_tiles_ieta_max-_tiles_ieta_min+1)*_n_tiles_phi); //**//
 //std::cout <<  "19\n"; baba++;/////////////////////////////////////////////////////////////////////////////////

  // now set up the cross-referencing between tiles
  for (int ieta = _tiles_ieta_min; ieta <= _tiles_ieta_max; ieta++) {
 //std::cout <<  "20\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
    for (int iphi = 0; iphi < _n_tiles_phi; iphi++) {
 //std::cout <<  "21\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
      Tile * tile = &((*_tiles)[_tile_index(ieta,iphi)]); //**//
 //std::cout <<  "22\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
      // no jets in this tile yet
      tile->head = NULL; // first element of tiles points to itself
 //std::cout <<  "23\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
      tile->begin_tiles[0] = tile;
 //std::cout <<  "24\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
      Tile ** pptile = & (tile->begin_tiles[0]);
 //std::cout <<  "25\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
      pptile++;
 //std::cout <<  "26\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
      //
      // set up L's in column to the left of X
      tile->surrounding_tiles = pptile;
 //std::cout <<  "27\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
      if (ieta > _tiles_ieta_min) {
 //std::cout <<  "28\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
// with the itile subroutine, we can safely run tiles from
// idphi=-1 to idphi=+1, because it takes care of
// negative and positive boundaries
for (int idphi = -1; idphi <=+1; idphi++) {
 //std::cout <<  "29\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
*pptile = &((*_tiles)[_tile_index(ieta-1,iphi+idphi)]); //**//
 //std::cout <<  "30\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
pptile++;
 //std::cout <<  "31\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
}	
      }
      // now set up last L (below X)
      *pptile = &((*_tiles)[_tile_index(ieta,iphi-1)]); //**//
 //std::cout <<  "32\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
      pptile++;
 //std::cout <<  "33\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
      // set up first R (above X)
      tile->RH_tiles = pptile; //**//
 //std::cout <<  "34\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
      *pptile = &((*_tiles)[_tile_index(ieta,iphi+1)]); //**//
 //std::cout <<  "35\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
      pptile++;
 //std::cout <<  "36\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
      // set up remaining R's, to the right of X
      if (ieta < _tiles_ieta_max) {
 //std::cout <<  "37\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
for (int idphi = -1; idphi <= +1; idphi++) {
 //std::cout <<  "38\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
*pptile = &((*_tiles)[_tile_index(ieta+1,iphi+idphi)]); //**//
 //std::cout <<  "39\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
pptile++;
 //std::cout <<  "40\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
}	
      }
      // now put semaphore for end tile
      tile->end_tiles = pptile; //**//
 //std::cout <<  "41\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
      // finally make sure tiles are untagged
      tile->tagged = false; //**//
 //std::cout <<  "42\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
    }
  }
  if(tman->getMulti()){tman->resetFlag( _tiles ); s_CREATE_THREADS(_tiles , tman);} //**//
 //std::cout << "done\n"; baba++;/////////////////////////////////////////////////////////////////////////////////
  }
}

//----------------------------------------------------------------------
/// return the tile index corresponding to the given eta,phi point
int ClusterSequence::_tile_index(const double & eta, const double & phi) const {
  int ieta, iphi;
  if (eta <= _tiles_eta_min) {ieta = 0;}
  else if (eta >= _tiles_eta_max) {ieta = _tiles_ieta_max-_tiles_ieta_min;}
  else {
    //ieta = int(floor((eta - _tiles_eta_min) / _tile_size_eta));
    ieta = int(((eta - _tiles_eta_min) / _tile_size_eta));
    // following needed in case of rare but nasty rounding errors
    if (ieta > _tiles_ieta_max-_tiles_ieta_min) {
      ieta = _tiles_ieta_max-_tiles_ieta_min;}
  }
  // allow for some extent of being beyond range in calculation of phi
  // as well
  //iphi = (int(floor(phi/_tile_size_phi)) + _n_tiles_phi) % _n_tiles_phi;
  // with just int and no floor, things run faster but beware
  iphi = int((phi+twopi)/_tile_size_phi) % _n_tiles_phi;
  return (iphi + ieta * _n_tiles_phi);
}


//----------------------------------------------------------------------
// overloaded version which additionally sets up information regarding the
// tiling
inline void ClusterSequence::_tj_set_jetinfo( TiledJet * const jet,
const int _jets_index) {
  // first call the generic setup
  _bj_set_jetinfo<>(jet, _jets_index);

  // Then do the setup specific to the tiled case.

  // Find out which tile it belonds to
  jet->tile_index = _tile_index(jet->eta, jet->phi);

  // Insert it into the tile's linked list of jets
  Tile * tile = &((*_tiles)[jet->tile_index]); //**//
  jet->previous = NULL;
  jet->next = tile->head;
  if (jet->next != NULL) {jet->next->previous = jet;}
  tile->head = jet;
}


//----------------------------------------------------------------------
/// output the contents of the tiles
void ClusterSequence::_print_tiles(TiledJet * briefjets ) const {
  for (vector<Tile>::const_iterator tile = _tiles->begin(); //**//
       tile < _tiles->end(); tile++) { //**//
    cout << "Tile " << tile - _tiles->begin()<<" = "; //**//
    vector<int> list;
    for (TiledJet * jetI = tile->head; jetI != NULL; jetI = jetI->next) {
      list.push_back(jetI-briefjets);
      //cout <<" "<<jetI-briefjets;
    }
    sort(list.begin(),list.end());
    for (unsigned int i = 0; i < list.size(); i++) {cout <<" "<<list[i];}
    cout <<"\n";
  }
}


//----------------------------------------------------------------------
/// Add to the vector tile_union the tiles that are in the neighbourhood
/// of the specified tile_index, including itself -- start adding
/// from position n_near_tiles-1, and increase n_near_tiles as
/// you go along (could have done it more C++ like with vector with reserved
/// space, but fear is that it would have been slower, e.g. checking
/// for end of vector at each stage to decide whether to resize it)
void ClusterSequence::_add_neighbours_to_tile_union(const int tile_index,
vector<int> & tile_union, int & n_near_tiles) const {
  for (Tile * const * near_tile = ((*_tiles)[tile_index]).begin_tiles; //**//
       near_tile != ((*_tiles)[tile_index]).end_tiles; near_tile++){ //**//
    // get the tile number
    tile_union[n_near_tiles] = *near_tile - & ((*_tiles)[0]); //**//
    n_near_tiles++;
  }
}


//----------------------------------------------------------------------
/// Like _add_neighbours_to_tile_union, but only adds neighbours if
/// their "tagged" status is false; when a neighbour is added its
/// tagged status is set to true.
inline void ClusterSequence::_add_untagged_neighbours_to_tile_union(
               const int tile_index,
vector<int> & tile_union, int & n_near_tiles) {
  for (Tile ** near_tile = ((*_tiles)[tile_index]).begin_tiles; //**//
       near_tile != ((*_tiles)[tile_index]).end_tiles; near_tile++){ //**//
    if (! (*near_tile)->tagged) {
      (*near_tile)->tagged = true;
      // get the tile number
      tile_union[n_near_tiles] = *near_tile - & ((*_tiles)[0]); //**//
      n_near_tiles++;
    }
  }
}


//----------------------------------------------------------------------
/// run a tiled clustering
void ClusterSequence::_tiled_N2_cluster() {

  _initialise_tiles();
  //std::cout << "   ********************** _initialise_tiles() EXITED (CLUSTERING STILL GOING) ***********************\n";
  int n = _jets.size();
  TiledJet * briefjets = new TiledJet[n];
  TiledJet * jetA = briefjets, * jetB;
  TiledJet oldB;
  oldB.tile_index=0; // prevents a gcc warning

  // will be used quite deep inside loops, but declare it here so that
  // memory (de)allocation gets done only once
  vector<int> tile_union(3*n_tile_neighbours);
  
  // initialise the basic jet info
  for (int i = 0; i< n; i++) {
    _tj_set_jetinfo(jetA, i);
    //cout << i<<": "<<jetA->tile_index<<"\n";
    jetA++; // move on to next entry of briefjets
  }
  TiledJet * tail = jetA; // a semaphore for the end of briefjets
  TiledJet * head = briefjets; // a nicer way of naming start

  //std::cout << "   ********************** RELEASE THE HOUNDS\n";
  if ( tman->getMulti() ){ //**//
    tman->waitOnThreads(); //**//
  //std::cout << "   ********************** done waiting on threads to finish initializing\n";
    tman->closeBottom(); //**//
  //std::cout << "   ********************** locked bottom mutex\n";
    tman->resetFlag( _tiles ); //**//
  //std::cout << "   ********************** just reset flag\n";
    tman->openTop(); //**//
  //std::cout << "   ********************** unlocked top mutex\n";
    tman->waitOnThreads(); //**//
  //std::cout << "   ********************** done waiting on threads to NN-algo\n";
    tman->closeTop(); //**//
  //std::cout << "   ********************** locked top\n";
    tman->resetFlag( _tiles ); //**//
    tman->openBottom(); //**//
  //std::cout << "   ********************** HOUNDS RELEASED\n";
  } else { //**//
    // set up the initial nearest neighbour information
    vector<Tile>::const_iterator tile;
    for (tile = _tiles->begin(); tile != _tiles->end(); tile++) { //**//
      // first do it on this tile
      for (jetA = tile->head; jetA != NULL; jetA = jetA->next) {
        for (jetB = tile->head; jetB != jetA; jetB = jetB->next) {
   double dist = _bj_dist(jetA,jetB);
  if (dist < jetA->NN_dist) {jetA->NN_dist = dist; jetA->NN = jetB;}
if (dist < jetB->NN_dist) {jetB->NN_dist = dist; jetB->NN = jetA;}
        }
      }
      // then do it for RH tiles
      for (Tile ** RTile = tile->RH_tiles; RTile != tile->end_tiles; RTile++) {
        for (jetA = tile->head; jetA != NULL; jetA = jetA->next) {
   for (jetB = (*RTile)->head; jetB != NULL; jetB = jetB->next) {
double dist = _bj_dist(jetA,jetB);
if (dist < jetA->NN_dist) {jetA->NN_dist = dist; jetA->NN = jetB;}
if (dist < jetB->NN_dist) {jetB->NN_dist = dist; jetB->NN = jetA;}
  }
        }
      }
    }
  }




  // now create the diJ (where J is i's NN) table -- remember that
  // we differ from standard normalisation here by a factor of R2
  double * diJ = new double[n];
  jetA = head;
  for (int i = 0; i < n; i++) {
    diJ[i] = _bj_diJ(jetA);
    jetA++; // have jetA follow i
  }

  // now run the recombination loop
  int history_location = n-1;
  while (tail != head) {

    // find the minimum of the diJ on this round
    double diJ_min = diJ[0];
    int diJ_min_jet = 0;
    for (int i = 1; i < n; i++) {
      if (diJ[i] < diJ_min) {diJ_min_jet = i; diJ_min = diJ[i];}
    }

    // do the recombination between A and B
    history_location++;
    jetA = & briefjets[diJ_min_jet];
    jetB = jetA->NN;
    // put the normalisation back in
    diJ_min *= _invR2;

    //if (n == 19) {cout << "Hello "<<jetA-head<<" "<<jetB-head<<"\n";}

    //cout <<" WILL RECOMBINE "<< jetA-briefjets<<" "<<jetB-briefjets<<"\n";

    if (jetB != NULL) {
      // jet-jet recombination
      // If necessary relabel A & B to ensure jetB < jetA, that way if
      // the larger of them == newtail then that ends up being jetA and
      // the new jet that is added as jetB is inserted in a position that
      // has a future!
      if (jetA < jetB) {std::swap(jetA,jetB);}

      int nn; // new jet index
      _do_ij_recombination_step(jetA->_jets_index, jetB->_jets_index, diJ_min, nn);

      //OBS// get the two history indices
      //OBSint hist_a = _jets[jetA->_jets_index].cluster_hist_index();
      //OBSint hist_b = _jets[jetB->_jets_index].cluster_hist_index();
      //OBS// create the recombined jet
      //OBS_jets.push_back(_jets[jetA->_jets_index] + _jets[jetB->_jets_index]);
      //OBSint nn = _jets.size() - 1;
      //OBS_jets[nn].set_cluster_hist_index(history_location);
      //OBS// update history
      //OBS//cout <<n-1<<" "<<jetA-head<<" "<<jetB-head<<"; ";
      //OBS_add_step_to_history(history_location,
      //OBS min(hist_a,hist_b),max(hist_a,hist_b),
      //OBS nn, diJ_min);

      // what was jetB will now become the new jet
      _bj_remove_from_tiles(jetA);
      oldB = * jetB; // take a copy because we will need it...
      _bj_remove_from_tiles(jetB);
      _tj_set_jetinfo(jetB, nn); // also registers the jet in the tiling
    } else {
      // jet-beam recombination
      _do_iB_recombination_step(jetA->_jets_index, diJ_min);

      //OBS// get the hist_index
      //OBSint hist_a = _jets[jetA->_jets_index].cluster_hist_index();
      //OBS//cout <<n-1<<" "<<jetA-head<<" "<<-1<<"; ";
      //OBS_add_step_to_history(history_location,hist_a,BeamJet,Invalid,diJ_min);
      _bj_remove_from_tiles(jetA);
    }

    // first establish the set of tiles over which we are going to
    // have to run searches for updated and new nearest-neighbours
    int n_near_tiles = 0;
    _add_neighbours_to_tile_union(jetA->tile_index, tile_union, n_near_tiles);
    if (jetB != NULL) {
      bool sort_it = false;
      if (jetB->tile_index != jetA->tile_index) {
sort_it = true;
_add_neighbours_to_tile_union(jetB->tile_index,tile_union,n_near_tiles);
      }
      if (oldB.tile_index != jetA->tile_index &&
oldB.tile_index != jetB->tile_index) {
sort_it = true;
_add_neighbours_to_tile_union(oldB.tile_index,tile_union,n_near_tiles);
      }

      if (sort_it) {
// sort the tiles before then compressing the list
sort(tile_union.begin(), tile_union.begin()+n_near_tiles);
// and now condense the list
int nnn = 1;
for (int i = 1; i < n_near_tiles; i++) {
if (tile_union[i] != tile_union[nnn-1]) {
tile_union[nnn] = tile_union[i];
nnn++;
}
}
n_near_tiles = nnn;
      }
    }

    // now update our nearest neighbour info and diJ table
    // first reduce size of table
    tail--; n--;
    if (jetA == tail) {
      // there is nothing to be done
    } else {
      // Copy last jet contents and diJ info into position of jetA
      *jetA = *tail;
      diJ[jetA - head] = diJ[tail-head];
      // IN the tiling fix pointers to tail and turn them into
      // pointers to jetA (from predecessors, successors and the tile
      // head if need be)
      if (jetA->previous == NULL) {
((*_tiles)[jetA->tile_index]).head = jetA; //**//
      } else {
jetA->previous->next = jetA;
      }
      if (jetA->next != NULL) {jetA->next->previous = jetA;}
    }

    // Initialise jetB's NN distance as well as updating it for
    // other particles.
    for (int itile = 0; itile < n_near_tiles; itile++) {
      Tile * tile_ptr = &((*_tiles)[tile_union[itile]]); //**//
      for (TiledJet * jetI = tile_ptr->head; jetI != NULL; jetI = jetI->next) {
// see if jetI had jetA or jetB as a NN -- if so recalculate the NN
if (jetI->NN == jetA || (jetI->NN == jetB && jetB != NULL)) {
jetI->NN_dist = _R2;
jetI->NN = NULL;
// now go over tiles that are neighbours of I (include own tile)
for (Tile ** near_tile = tile_ptr->begin_tiles;
near_tile != tile_ptr->end_tiles; near_tile++) {
// and then over the contents of that tile
for (TiledJet * jetJ = (*near_tile)->head;
                            jetJ != NULL; jetJ = jetJ->next) {
double dist = _bj_dist(jetI,jetJ);
if (dist < jetI->NN_dist && jetJ != jetI) {
jetI->NN_dist = dist; jetI->NN = jetJ;
}
}
}
diJ[jetI-head] = _bj_diJ(jetI); // update diJ
}
// check whether new jetB is closer than jetI's current NN and
// if need to update things
if (jetB != NULL) {
double dist = _bj_dist(jetI,jetB);
if (dist < jetI->NN_dist) {
if (jetI != jetB) {
jetI->NN_dist = dist;
jetI->NN = jetB;
diJ[jetI-head] = _bj_diJ(jetI); // update diJ...
}
}
if (dist < jetB->NN_dist) {
if (jetI != jetB) {
jetB->NN_dist = dist;
jetB->NN = jetI;}
}
}
      }
    }


    if (jetB != NULL) {diJ[jetB-head] = _bj_diJ(jetB);}
    //cout << n<<" "<<briefjets[95].NN-briefjets<<" "<<briefjets[95].NN_dist <<"\n";

    // remember to update pointers to tail
    for (Tile ** near_tile = ((*_tiles)[tail->tile_index]).begin_tiles; //**//
near_tile!= ((*_tiles)[tail->tile_index]).end_tiles; near_tile++){ //**//
      // and then the contents of that tile
      for (TiledJet * jetJ = (*near_tile)->head;
jetJ != NULL; jetJ = jetJ->next) {
if (jetJ->NN == tail) {jetJ->NN = jetA;}
      }
    }

    //for (int i = 0; i < n; i++) {
    // if (briefjets[i].NN-briefjets >= n && briefjets[i].NN != NULL) {cout <<"YOU MUST BE CRAZY for n ="<<n<<", i = "<<i<<", NN = "<<briefjets[i].NN-briefjets<<"\n";}
    //}


    if (jetB != NULL) {diJ[jetB-head] = _bj_diJ(jetB);}
    //cout << briefjets[95].NN-briefjets<<" "<<briefjets[95].NN_dist <<"\n";

  }

  // final cleaning up;
  delete[] diJ;
  delete[] briefjets;
  if (tman->getMulti()) {tman->setFirst();} //**//
  //static int eventnumber = 0; eventnumber++;
  //std::cout << "   ********************** " << eventnumber << "th CLUSTERING COMPLETE ***********************\n";
}


//----------------------------------------------------------------------
/// run a tiled clustering
void ClusterSequence::_faster_tiled_N2_cluster() {

  _initialise_tiles();

  int n = _jets.size();
  TiledJet * briefjets = new TiledJet[n];
  TiledJet * jetA = briefjets, * jetB;
  TiledJet oldB;
  oldB.tile_index=0; // prevents a gcc warning

  // will be used quite deep inside loops, but declare it here so that
  // memory (de)allocation gets done only once
  vector<int> tile_union(3*n_tile_neighbours);
  
  // initialise the basic jet info
  for (int i = 0; i< n; i++) {
    _tj_set_jetinfo(jetA, i);
    //cout << i<<": "<<jetA->tile_index<<"\n";
    jetA++; // move on to next entry of briefjets
  }
  TiledJet * head = briefjets; // a nicer way of naming start

  // set up the initial nearest neighbour information
  vector<Tile>::const_iterator tile;
  for (tile = _tiles->begin(); tile != _tiles->end(); tile++) {
    // first do it on this tile
    for (jetA = tile->head; jetA != NULL; jetA = jetA->next) {
      for (jetB = tile->head; jetB != jetA; jetB = jetB->next) {
double dist = _bj_dist(jetA,jetB);
if (dist < jetA->NN_dist) {jetA->NN_dist = dist; jetA->NN = jetB;}
if (dist < jetB->NN_dist) {jetB->NN_dist = dist; jetB->NN = jetA;}
      }
    }
    // then do it for RH tiles
    for (Tile ** RTile = tile->RH_tiles; RTile != tile->end_tiles; RTile++) {
      for (jetA = tile->head; jetA != NULL; jetA = jetA->next) {
for (jetB = (*RTile)->head; jetB != NULL; jetB = jetB->next) {
double dist = _bj_dist(jetA,jetB);
if (dist < jetA->NN_dist) {jetA->NN_dist = dist; jetA->NN = jetB;}
if (dist < jetB->NN_dist) {jetB->NN_dist = dist; jetB->NN = jetA;}
}
      }
    }
    // no need to do it for LH tiles, since they are implicitly done
    // when we set NN for both jetA and jetB on the RH tiles.
  }

  
  // now create the diJ (where J is i's NN) table -- remember that
  // we differ from standard normalisation here by a factor of R2
  // (corrected for at the end).
  struct diJ_plus_link {
    double diJ; // the distance
    TiledJet * jet; // the jet (i) for which we've found this distance
                    // (whose NN will the J).
  };
  diJ_plus_link * diJ = new diJ_plus_link[n];
  jetA = head;
  for (int i = 0; i < n; i++) {
    diJ[i].diJ = _bj_diJ(jetA); // kt distance * R^2
    diJ[i].jet = jetA; // our compact diJ table will not be in
    jetA->diJ_posn = i; // one-to-one corresp. with non-compact jets,
                        // so set up bi-directional correspondence here.
    jetA++; // have jetA follow i
  }

  // now run the recombination loop
  int history_location = n-1;
  while (n > 0) {

    // find the minimum of the diJ on this round
    diJ_plus_link * best, *stop; // pointers a bit faster than indices
    // could use best to keep track of diJ min, but it turns out to be
    // marginally faster to have a separate variable (avoids n
    // dereferences at the expense of n/2 assignments).
    double diJ_min = diJ[0].diJ; // initialise the best one here.
    best = diJ; // and here
    stop = diJ+n;
    for (diJ_plus_link * here = diJ+1; here != stop; here++) {
      if (here->diJ < diJ_min) {best = here; diJ_min = here->diJ;}
    }

    // do the recombination between A and B
    history_location++;
    jetA = best->jet;
    jetB = jetA->NN;
    // put the normalisation back in
    diJ_min *= _invR2;

    if (jetB != NULL) {
      // jet-jet recombination
      // If necessary relabel A & B to ensure jetB < jetA, that way if
      // the larger of them == newtail then that ends up being jetA and
      // the new jet that is added as jetB is inserted in a position that
      // has a future!
      if (jetA < jetB) {std::swap(jetA,jetB);}

      int nn; // new jet index
      _do_ij_recombination_step(jetA->_jets_index, jetB->_jets_index, diJ_min, nn);
      
      //OBS// get the two history indices
      //OBSint ihstry_a = _jets[jetA->_jets_index].cluster_hist_index();
      //OBSint ihstry_b = _jets[jetB->_jets_index].cluster_hist_index();
      //OBS// create the recombined jet
      //OBS_jets.push_back(_jets[jetA->_jets_index] + _jets[jetB->_jets_index]);
      //OBSint nn = _jets.size() - 1;
      //OBS_jets[nn].set_cluster_hist_index(history_location);
      //OBS// update history
      //OBS//cout <<n-1<<" "<<jetA-head<<" "<<jetB-head<<"; ";
      //OBS_add_step_to_history(history_location,
      //OBS min(ihstry_a,ihstry_b),max(ihstry_a,ihstry_b),
      //OBS nn, diJ_min);
      // what was jetB will now become the new jet
      _bj_remove_from_tiles(jetA);
      oldB = * jetB; // take a copy because we will need it...
      _bj_remove_from_tiles(jetB);
      _tj_set_jetinfo(jetB, nn); // cause jetB to become _jets[nn]
                                 // (also registers the jet in the tiling)
    } else {
      // jet-beam recombination
      // get the hist_index
      _do_iB_recombination_step(jetA->_jets_index, diJ_min);
      //OBSint ihstry_a = _jets[jetA->_jets_index].cluster_hist_index();
      //OBS//cout <<n-1<<" "<<jetA-head<<" "<<-1<<"; ";
      //OBS_add_step_to_history(history_location,ihstry_a,BeamJet,Invalid,diJ_min);
      _bj_remove_from_tiles(jetA);
    }

    // first establish the set of tiles over which we are going to
    // have to run searches for updated and new nearest-neighbours --
    // basically a combination of vicinity of the tiles of the two old
    // and one new jet.
    int n_near_tiles = 0;
    _add_untagged_neighbours_to_tile_union(jetA->tile_index,
tile_union, n_near_tiles);
    if (jetB != NULL) {
      if (jetB->tile_index != jetA->tile_index) {
_add_untagged_neighbours_to_tile_union(jetB->tile_index,
tile_union,n_near_tiles);
      }
      if (oldB.tile_index != jetA->tile_index &&
oldB.tile_index != jetB->tile_index) {
_add_untagged_neighbours_to_tile_union(oldB.tile_index,
tile_union,n_near_tiles);
      }
    }

    // now update our nearest neighbour info and diJ table
    // first reduce size of table
    n--;
    // then compactify the diJ by taking the last of the diJ and copying
    // it to the position occupied by the diJ for jetA
    diJ[n].jet->diJ_posn = jetA->diJ_posn;
    diJ[jetA->diJ_posn] = diJ[n];

    // Initialise jetB's NN distance as well as updating it for
    // other particles.
    // Run over all tiles in our union
    for (int itile = 0; itile < n_near_tiles; itile++) {
      Tile * tile_ptr = &(*_tiles)[tile_union[itile]];
      tile_ptr->tagged = false; // reset tag, since we're done with unions
      // run over all jets in the current tile
      for (TiledJet * jetI = tile_ptr->head; jetI != NULL; jetI = jetI->next) {
// see if jetI had jetA or jetB as a NN -- if so recalculate the NN
if (jetI->NN == jetA || (jetI->NN == jetB && jetB != NULL)) {
jetI->NN_dist = _R2;
jetI->NN = NULL;
// now go over tiles that are neighbours of I (include own tile)
for (Tile ** near_tile = tile_ptr->begin_tiles;
near_tile != tile_ptr->end_tiles; near_tile++) {
// and then over the contents of that tile
for (TiledJet * jetJ = (*near_tile)->head;
                            jetJ != NULL; jetJ = jetJ->next) {
double dist = _bj_dist(jetI,jetJ);
if (dist < jetI->NN_dist && jetJ != jetI) {
jetI->NN_dist = dist; jetI->NN = jetJ;
}
}
}
diJ[jetI->diJ_posn].diJ = _bj_diJ(jetI); // update diJ kt-dist
}
// check whether new jetB is closer than jetI's current NN and
// if jetI is closer than jetB's current (evolving) nearest
// neighbour. Where relevant update things
if (jetB != NULL) {
double dist = _bj_dist(jetI,jetB);
if (dist < jetI->NN_dist) {
if (jetI != jetB) {
jetI->NN_dist = dist;
jetI->NN = jetB;
diJ[jetI->diJ_posn].diJ = _bj_diJ(jetI); // update diJ...
}
}
if (dist < jetB->NN_dist) {
if (jetI != jetB) {
jetB->NN_dist = dist;
jetB->NN = jetI;}
}
}
      }
    }

    // finally, register the updated kt distance for B
    if (jetB != NULL) {diJ[jetB->diJ_posn].diJ = _bj_diJ(jetB);}

  }

  // final cleaning up;
  delete[] diJ;
  delete[] briefjets;
}



//----------------------------------------------------------------------
/// run a tiled clustering, with our minheap for keeping track of the
/// smallest dij
void ClusterSequence::_minheap_faster_tiled_N2_cluster() {

  _initialise_tiles();

  int n = _jets.size();
  TiledJet * briefjets = new TiledJet[n];
  TiledJet * jetA = briefjets, * jetB;
  TiledJet oldB;
  oldB.tile_index=0; // prevents a gcc warning
  

  // will be used quite deep inside loops, but declare it here so that
  // memory (de)allocation gets done only once
  vector<int> tile_union(3*n_tile_neighbours);
  
  // initialise the basic jet info
  for (int i = 0; i< n; i++) {
    _tj_set_jetinfo(jetA, i);
    //cout << i<<": "<<jetA->tile_index<<"\n";
    jetA++; // move on to next entry of briefjets
  }
  TiledJet * head = briefjets; // a nicer way of naming start

  // set up the initial nearest neighbour information
  vector<Tile>::const_iterator tile;
  for (tile = _tiles->begin(); tile != _tiles->end(); tile++) {
    // first do it on this tile
    for (jetA = tile->head; jetA != NULL; jetA = jetA->next) {
      for (jetB = tile->head; jetB != jetA; jetB = jetB->next) {
double dist = _bj_dist(jetA,jetB);
if (dist < jetA->NN_dist) {jetA->NN_dist = dist; jetA->NN = jetB;}
if (dist < jetB->NN_dist) {jetB->NN_dist = dist; jetB->NN = jetA;}
      }
    }
    // then do it for RH tiles
    for (Tile ** RTile = tile->RH_tiles; RTile != tile->end_tiles; RTile++) {
      for (jetA = tile->head; jetA != NULL; jetA = jetA->next) {
for (jetB = (*RTile)->head; jetB != NULL; jetB = jetB->next) {
double dist = _bj_dist(jetA,jetB);
if (dist < jetA->NN_dist) {jetA->NN_dist = dist; jetA->NN = jetB;}
if (dist < jetB->NN_dist) {jetB->NN_dist = dist; jetB->NN = jetA;}
}
      }
    }
    // no need to do it for LH tiles, since they are implicitly done
    // when we set NN for both jetA and jetB on the RH tiles.
  }

  
  //// now create the diJ (where J is i's NN) table -- remember that
  //// we differ from standard normalisation here by a factor of R2
  //// (corrected for at the end).
  //struct diJ_plus_link {
  // double diJ; // the distance
  // TiledJet * jet; // the jet (i) for which we've found this distance
  // // (whose NN will the J).
  //};
  //diJ_plus_link * diJ = new diJ_plus_link[n];
  //jetA = head;
  //for (int i = 0; i < n; i++) {
  // diJ[i].diJ = _bj_diJ(jetA); // kt distance * R^2
  // diJ[i].jet = jetA; // our compact diJ table will not be in
  // jetA->diJ_posn = i; // one-to-one corresp. with non-compact jets,
  // // so set up bi-directional correspondence here.
  // jetA++; // have jetA follow i
  //}

  vector<double> diJs(n);
  for (int i = 0; i < n; i++) {
    diJs[i] = _bj_diJ(&briefjets[i]);
    briefjets[i].label_minheap_update_done();
  }
  MinHeap minheap(diJs);
  // have a stack telling us which jets we'll have to update on the heap
  vector<TiledJet *> jets_for_minheap;
  jets_for_minheap.reserve(n);

  // now run the recombination loop
  int history_location = n-1;
  while (n > 0) {

    double diJ_min = minheap.minval() *_invR2;
    jetA = head + minheap.minloc();

    // do the recombination between A and B
    history_location++;
    jetB = jetA->NN;

    if (jetB != NULL) {
      // jet-jet recombination
      // If necessary relabel A & B to ensure jetB < jetA, that way if
      // the larger of them == newtail then that ends up being jetA and
      // the new jet that is added as jetB is inserted in a position that
      // has a future!
      if (jetA < jetB) {std::swap(jetA,jetB);}

      int nn; // new jet index
      _do_ij_recombination_step(jetA->_jets_index, jetB->_jets_index, diJ_min, nn);
      
      // what was jetB will now become the new jet
      _bj_remove_from_tiles(jetA);
      oldB = * jetB; // take a copy because we will need it...
      _bj_remove_from_tiles(jetB);
      _tj_set_jetinfo(jetB, nn); // cause jetB to become _jets[nn]
                                 // (also registers the jet in the tiling)
    } else {
      // jet-beam recombination
      // get the hist_index
      _do_iB_recombination_step(jetA->_jets_index, diJ_min);
      _bj_remove_from_tiles(jetA);
    }

    // remove the minheap entry for jetA
    minheap.remove(jetA-head);

    // first establish the set of tiles over which we are going to
    // have to run searches for updated and new nearest-neighbours --
    // basically a combination of vicinity of the tiles of the two old
    // and one new jet.
    int n_near_tiles = 0;
    _add_untagged_neighbours_to_tile_union(jetA->tile_index,
tile_union, n_near_tiles);
    if (jetB != NULL) {
      if (jetB->tile_index != jetA->tile_index) {
_add_untagged_neighbours_to_tile_union(jetB->tile_index,
tile_union,n_near_tiles);
      }
      if (oldB.tile_index != jetA->tile_index &&
oldB.tile_index != jetB->tile_index) {
// GS: the line below generates a warning that oldB.tile_index
// may be used uninitialised. However, to reach this point, we
// ned jetB != NULL (see test a few lines above) and is jetB
// !=NULL, one would have gone through "oldB = *jetB before
// (see piece of code ~20 line above), so the index is
// initialised. We do not do anything to avoid the warning to
// avoid any potential speed impact.
_add_untagged_neighbours_to_tile_union(oldB.tile_index,
tile_union,n_near_tiles);
      }
      // indicate that we'll have to update jetB in the minheap
      jetB->label_minheap_update_needed();
      jets_for_minheap.push_back(jetB);
    }


    // Initialise jetB's NN distance as well as updating it for
    // other particles.
    // Run over all tiles in our union
    for (int itile = 0; itile < n_near_tiles; itile++) {
      Tile * tile_ptr = &(*_tiles)[tile_union[itile]];
      tile_ptr->tagged = false; // reset tag, since we're done with unions
      // run over all jets in the current tile
      for (TiledJet * jetI = tile_ptr->head; jetI != NULL; jetI = jetI->next) {
// see if jetI had jetA or jetB as a NN -- if so recalculate the NN
if (jetI->NN == jetA || (jetI->NN == jetB && jetB != NULL)) {
jetI->NN_dist = _R2;
jetI->NN = NULL;
// label jetI as needing heap action...
if (!jetI->minheap_update_needed()) {
jetI->label_minheap_update_needed();
jets_for_minheap.push_back(jetI);}
// now go over tiles that are neighbours of I (include own tile)
for (Tile ** near_tile = tile_ptr->begin_tiles;
near_tile != tile_ptr->end_tiles; near_tile++) {
// and then over the contents of that tile
for (TiledJet * jetJ = (*near_tile)->head;
                            jetJ != NULL; jetJ = jetJ->next) {
double dist = _bj_dist(jetI,jetJ);
if (dist < jetI->NN_dist && jetJ != jetI) {
jetI->NN_dist = dist; jetI->NN = jetJ;
}
}
}
}
// check whether new jetB is closer than jetI's current NN and
// if jetI is closer than jetB's current (evolving) nearest
// neighbour. Where relevant update things
if (jetB != NULL) {
double dist = _bj_dist(jetI,jetB);
if (dist < jetI->NN_dist) {
if (jetI != jetB) {
jetI->NN_dist = dist;
jetI->NN = jetB;
// label jetI as needing heap action...
if (!jetI->minheap_update_needed()) {
jetI->label_minheap_update_needed();
jets_for_minheap.push_back(jetI);}
}
}
if (dist < jetB->NN_dist) {
if (jetI != jetB) {
jetB->NN_dist = dist;
jetB->NN = jetI;}
}
}
      }
    }

    // deal with jets whose minheap entry needs updating
    while (jets_for_minheap.size() > 0) {
      TiledJet * jetI = jets_for_minheap.back();
      jets_for_minheap.pop_back();
      minheap.update(jetI-head, _bj_diJ(jetI));
      jetI->label_minheap_update_done();
    }
    n--;
  }

  // final cleaning up;
  delete[] briefjets;
}


//---------------------------------------------------start Jon
 void ClusterSequence::s_NN_INIT( Tile * tile , ThreadManager * tman ){//one thread per tile
    while(1){
      TiledJet * jetA; //needs to be made public?
      TiledJet * jetB; //needs to be made public?
      //tman->coutFlag(" rdy: ");
      tman->decFlag();
      tman->waitTop();
      // set up the initial nearest neighbour information
  //std::cout << "       " << tile->address << " passed decflag/waittop" << "\n"; ////////////////////////////////////////////////////////////////
      ((tman->vm_tiles)[tile->address])->lock() ; // TODO : ERROR OCCURS HERE!
  //std::cout << "       " << tile->address << " just locked it's lock" << "\n"; ////////////////////////////////////////////////////////////////
      // first do it on this tile
      for (jetA = tile->head; jetA != NULL; jetA = jetA->next) {
  //std::cout << "       " << tile->address << " is inside forloop accessing tile->head" << "\n"; ////////////////////////////////////////////////////////////////
        for (jetB = tile->head; jetB != jetA; jetB = jetB->next) {
          double dist = _bj_dist(jetA,jetB);
          if (dist < jetA->NN_dist) {jetA->NN_dist = dist; jetA->NN = jetB;}
          if (dist < jetB->NN_dist) {jetB->NN_dist = dist; jetB->NN = jetA;}
        }
      }
      (tman->vm_tiles[tile->address])->unlock() ;
      // then do it for RH tiles
      for (Tile ** RTile = tile->RH_tiles; RTile != tile->end_tiles; RTile++) {
  //std::cout << "       " << tile->address  << " is trying to access grabTwoLocks(tile, RTile)" << "\n"; ////////////////////////////////////////////////////////////////
        tman->grabTwoLocks( tile->address , (*RTile)->address );
  //std::cout << "       " << tile->address << " is at decflag/waittop" << "\n"; ////////////////////////////////////////////////////////////////
        for (jetA = tile->head; jetA != NULL; jetA = jetA->next) {
          for (jetB = (*RTile)->head; jetB != NULL; jetB = jetB->next) {
            double dist = _bj_dist(jetA,jetB);
            if (dist < jetA->NN_dist) {jetA->NN_dist = dist; jetA->NN = jetB;}
            if (dist < jetB->NN_dist) {jetB->NN_dist = dist; jetB->NN = jetA;}
          }
        }
  //std::cout << "       " << tile->address << " is unlocking two locks" << "\n"; ////////////////////////////////////////////////////////////////
        tman->vm_tiles[tile->address]->unlock() ;
        tman->vm_tiles[(*RTile)->address]->unlock() ;
  //std::cout << "       " << tile->address << " just unlocked two locks" << "\n"; ////////////////////////////////////////////////////////////////
      }
      tman->decFlag();
      tman->waitBottom();
  //std::cout << "       " << tile->address << " just exited waitBottom()" << "\n"; ////////////////////////////////////////////////////////////////
      //tman->coutFlag(" done: ");
    }//end while(1)
  }
void ClusterSequence::s_CREATE_THREADS( std::vector<Tile> * _tiles , ThreadManager * tman ){
  for (int i = 0; i < (*_tiles).size(); ++i ) {
    //std::cout << "          creating THREAD " << i << "\n";
    Tile * tile = &((*_tiles)[i]);
    tile->address = i;
    std::mutex * _m = new std::mutex;
    (tman->vm_tiles).push_back( _m ) ;
    std::thread t1(&fastjet::ClusterSequence::s_NN_INIT, this, tile, tman);
    t1.detach();
    //std::cout << "          finished creating/detaching THREAD " << i << "\n";
  }
}
//---------------------------------------------------end Jon

FASTJET_END_NAMESPACE
