#include "stdafx.h"
#include "Game.h"


Game* g_gameObject = nullptr;

static void cleanGameObject(Game* game)
{
    g_gameObject = nullptr;
    delete game;
}

int main()
{
    try
    {
        shared_ptr<Game> game(g_gameObject = new Game(), &cleanGameObject);

        g_gameObject->run();

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
