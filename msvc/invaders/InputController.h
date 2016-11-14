#pragma once


enum CommandType
{
    Command_none,
    Command_exitGame,
    Command_fireOpen,
    Command_fireCease,
    Command_moveLeft,
    Command_moveRight,
    Command_moveStop,
};

struct Command
{
    double moment;
    CommandType command;

    Command(double moment, CommandType command):
        moment(moment), command(command)
    {}
};

typedef vector<Command> Commands;

// InputController collect events with it's own thread
class InputController: noncopyable
{
public:
    InputController();
    ~InputController();

    void start();
    void nowEnought(); // signal to stop working; waits for input thread stop

    Commands popCommands(); // takes out waiting commands to process

private:
    HANDLE _console;

    typedef function<void(InputController*, const double moment)> Callable;
    struct Button
    {
        bool pressedNow;
        Callable onChangeState;

        Button(Callable onChangeState):
            pressedNow(false),
            onChangeState(onChangeState)
        {}
    };
    Button _btnLeft;
    Button _btnRight;
    Button _btnSpace;

    void _handleEvent(const INPUT_RECORD& rec);
    void _handleKeyEvent(const double moment, const KEY_EVENT_RECORD& rec);
    void _handleMouseEvent(const double moment, const MOUSE_EVENT_RECORD& rec);
    bool _handleShortcut(const double moment, const KEY_EVENT_RECORD& rec);
    Button* _syncButtonState(const double moment, const KEY_EVENT_RECORD& rec);

    void onBtnLeft(const double moment);
    void onBtnRight(const double moment);
    void onBtnSpace(const double moment);
    void onMoveBtn(const double moment, Button& btnFired, Button& btnOpposite, 
        CommandType cmd, CommandType cmdOpposite);

    typedef list<Command> CommandList;
    Commands _queue;
    boost::mutex _queueAccess;

    // keep it last members
    volatile LONG _working;
    thread _collectThread;
    void collectCycle();
};

