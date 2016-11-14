#include "stdafx.h"
#include "Game.h"


/*
_______
\==o==/


     *
     *


     |
   ((*))
 <<=====>>
 
 ooooooooo


    O
   ^*^
  / | \
    =
   / \
  /   \

*/

int main()
{
    try
    {
        Game game;
        game.run();

        return 0;
    }
    catch(const std::exception& e)
    {
        cerr
            << endl
            << endl
            << "{Program terminated. " << e.what() << "}" << endl
            << endl;
        return 1;
    }
    catch(...)
    {
        cerr
            << endl
            << endl
            << "{Program terminated with unknown exception}" << endl
            << endl;
        return 1;
    }
}
