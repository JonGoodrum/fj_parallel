#include "fastjet/ClusterSequence.hh"
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <time.h>
using namespace fastjet;
using namespace std;

//-----------------------------------------------------------------------------
//This void is used to display the processed jets
void printjets( vector<PseudoJet> jets , int numcores , bool append )
{
  ofstream PrintToFile;
  //decides which file to write to.  Appends or re-writes the file.
  if ( ( numcores == 1 ) && ( append == false ) ) {
    PrintToFile.open ( "ONE_CORE_RESULTS.txt" );
  }
  else if ( ( numcores == 1 ) && ( append == true ) ) {
    PrintToFile.open ( "ONE_CORE_RESULTS.txt" , std::ios::app|ios::ate );
  }
  else if ( ( numcores > 1 ) && ( append == false ) ) {
    PrintToFile.open ( "MULTICORE_RESULTS.txt" );
  }
  else if ( ( numcores > 1 ) && ( append == true ) ) {
    PrintToFile.open ( "MULTICORE_RESULTS.txt" , std::ios::app|ios::ate );
  }
  //  prints out the jets
  PrintToFile << endl << "##################################################" << endl << endl <<   "        pt y phi" << endl;
  for (unsigned i = 0; i < jets.size(); i++) {
    PrintToFile << "jet " << i << ": "<< jets[i].perp() << " " 
                   << jets[i].rap() << " " << jets[i].phi() << endl;
    vector<PseudoJet> constituents = jets[i].constituents();
    for (unsigned j = 0; j < constituents.size(); j++) {
      PrintToFile << "    constituent " << j << "'s pt: " << constituents[j].perp()
           << endl;
    }
  }
  PrintToFile.close();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int main ( int argc , char *argv[] ) {


 //initialize stuff
  int numcores = 1 ;
  bool append = false ;
  bool repeat = true ;
  time_t timer1 = time(NULL);
  string this_line; 

 //handle arguments
  if ( argc > 1 )
  {
    //read whether user wants to append or replace RESULTS file:
    if ( ( string( argv[1] ) == "add" ) or ( string( argv[1] ) == "append" ) )
    {
      append = true ;
    }
    if ( atoi(argv[1]) != 0 ) //needs to check if argv1 is an int.  Numcores=0 without this
    {
      numcores = atoi(argv[1]) ;
    }
    if ( argc > 2 )
    {
      if ( ( string( argv[2] ) == "add" ) or ( string( argv[2] ) == "append" ) )
      {
        append = true ;
      }
    }
  }


    vector<fastjet::ClusterSequence::Tile> * tiles;
    fastjet::ClusterSequence::ThreadManager * tman = new fastjet::ClusterSequence::ThreadManager;
    tman->multiCore();
 //do for every event on the data file:
  while ( repeat == true )
  {
    repeat = false ;
    vector<fastjet::PseudoJet> particles;  
    double px, py , pz, E;


  // read the event data from the file
    while (cin >> px >> py >> pz >> E)
    {
      particles.push_back(fastjet::PseudoJet(px,py,pz,E));
     //terminate program if complete
      if (particles.empty())
      {
        time_t timer2 = time(NULL);
        time_t seconds = difftime(timer2,timer1);
        cout << "Elapsed time, " << numcores << "-core: " << seconds << endl;
        return 42;
      }
    }
  // check to see if more events are present
    cin.clear();
    getline( cin , this_line , '\n' );
    if ( this_line == "#END" )
    {
      repeat = true ;
      append = true ;
    }

  // choose a jet definition
    double R = 0.7;
    JetDefinition jet_def(cambridge_algorithm, R, E_scheme, N2PoorTiled);

  // run the clustering, extract the jets
    ClusterSequence cs(particles, jet_def, tman, tiles);
    vector<PseudoJet> jets = sorted_by_pt(cs.inclusive_jets());

  // print the jets to a file
    printjets( jets , numcores , append );


  }//end While(repeat=true)
  time_t timer2 = time(NULL);
  time_t seconds = difftime(timer2,timer1);
  cout << "Elapsed time, " << numcores << "-core: " << seconds << endl;
}//end Main()



//   TODO : When compiling, make sure to use the following library at the end:
//   g++ modified-short-example.cc -o OUTPUTFILENAME  `fastjet-install/bin/fastjet-config --cxxflags --libs --plugins`


