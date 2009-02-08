#include <iostream>
#include "client/dbclient.h"

// g++ -I ../.. -L ../.. tutorial.cpp -lmongoclient -lboost_thread -lboost_filesystem

using namespace mongo;

void run() {
  DBClientConnection c;

}

int main() { 
  try { 
    run();
  } catch( DBException &e ) { 
    cout << "caught " << e.what() << endl;
  }
  return 0;
}