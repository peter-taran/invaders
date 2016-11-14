#pragma once


// flags for ShortcutListener
static const WORD Shortcut_withAlt  = 0xA0;
static const WORD Shortcut_withCtrl = 0x0C;

typedef function<void()> ShortcutListener;
struct Shortcut
{
    WORD virtualKeyCode;
    WORD flags;
    bool callOnce; // with long press call listener only once; true by default
    ShortcutListener listener;

    Shortcut(const WORD virtualKeyCode, const WORD flags,
        const ShortcutListener& listener, bool callOnce = true);
};

// State of game controller, "what the player requests now?"
struct InputController
{
    struct FireMode: equality_comparable<FireMode>
    {
        bool opened; // user wants shoot

        FireMode();
        bool operator==(const FireMode& cmpWith) const;
    };
    FireMode gunFireMode;

    struct MoveMode: equality_comparable<MoveMode>
    {
        bool moveToRequested; // higher priority movement - to position
        int moveTo; // X-coord

        int direction;
            // -1: move left
            //  0: stay
            // +1: move right

        MoveMode();
        bool operator==(const MoveMode& cmpWith) const;
    };
    MoveMode gunMoveMode;

    InputController();
};

template<class State>
struct Update
{
    const double moment;
    State& now; // handler can change state

    Update(const double moment, State& now):
        moment(moment), now(now)
    {}
};

typedef function<
    void(const Update<InputController::FireMode>& mode)
> FireModeListener;

typedef function<
    void(const Update<InputController::MoveMode>& mode)
> MoveModeListener;


// InputProcessor collect events with it's own background thread
// and processes it by explicit requests
class InputProcessor: noncopyable
{
public:
    InputProcessor();
    ~InputProcessor();

    void listenShortcut(const Shortcut& shortcut);
    void listenGunFireModeChange(FireModeListener listener);
    void listenGunMoveModeChange(MoveModeListener listener);

    void start();
    void nowEnought(); // signal to stop working; waits for input thread stop

    void processWaitingEvents(); // process and clean queue of events

private:
    HANDLE _console;

    struct InputEvent
    {
        double moment;
        INPUT_RECORD rec;

        InputEvent();
        InputEvent(const INPUT_RECORD& rec);
    };

    struct ButtonState: equality_comparable<ButtonState>
    {
        bool pressed;

        ButtonState();
        bool operator==(const ButtonState& cmpWith) const;
    };

    // State of input controls
    struct InputState
    {
        DisplayCoord mousePos;
    
        ButtonState btnMouse1; // VK_LBUTTON: leftmost button
        ButtonState btnMouse2; // VK_RBUTTON: rightmost button
        ButtonState btnLeft;   // VK_LEFT
        ButtonState btnRight;  // VK_RIGHT
        ButtonState btnSpace;  // VK_SPACE
    
        ButtonState btnCtrl; // any of Alt buttons
        ButtonState btnAlt;  // any of Control buttons

        InputState();
    };

    InputState _inputState;

    InputController _state;

    struct ShortcutEx: Shortcut
    {
        ButtonState btnState;
        ShortcutEx(const Shortcut& from);
    };
    typedef list<ShortcutEx> Shortcuts;
    Shortcuts _shortcuts;
    void _handleShortcut(const double moment, const KEY_EVENT_RECORD& rec);

    void _handleEvent(const InputEvent& e);
    void _handleKeyEvent(const double moment, const KEY_EVENT_RECORD& rec);
    void _handleMouseEvent(const double moment, const MOUSE_EVENT_RECORD& rec);
    
    void _syncControlButtonStates(DWORD controlKeysState);
    void _syncMouseButtonStates(DWORD mouseButtonsState);
    bool _syncFiredButtonState(const KEY_EVENT_RECORD& rec); // true if state changed
    bool _syncButtonState(ButtonState& btn, bool pressedNow); // true if state changed

    MoveModeListener _onChange_gunMove;
    void _updateState_gunMove(const double moment);
    
    FireModeListener _onChange_gunFire;
    void _updateState_gunFire(const double moment);

    typedef vector<InputEvent> InputEvents;
    InputEvents _queue;
    boost::mutex _queueAccess;

    // keep it last members
    volatile LONG _working;
    thread _collectThread;
    void collectCycle();
};

