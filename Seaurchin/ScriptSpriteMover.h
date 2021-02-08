#pragma once

#include "MoverFunctionExpression.h"
#include "ScriptSprite.h"

class MoverObject {
public:
    static constexpr double DefaultValue = std::numeric_limits<double>::quiet_NaN();

    enum class StateID
    {
        Suspend,
        Waiting,
        Working,
        Done,

        Unexpected
    };

    static bool RegisterType(asIScriptEngine *engine);

    MoverObject()
        : reference(1)
        , target(nullptr)
        , pFunction(nullptr)
        , variables()
        , fieldID(SSprite::FieldID::Undefined)
        , state(StateID::Suspend)
        , waiting(0)
        , time(0)
        , wait(0)
        , begin(DefaultValue)
        , end(DefaultValue)
        , isBeginOffset(false)
        , isEndOffset(false)
    {}

    static MoverObject* Factory();
    MoverObject* Clone();
    void Clear();

    void AddRef() { ++reference; }
    void Release() { if (--reference == 0) delete this; }
    int GetRefCount() const { return reference; }

    bool SetTargetSprite(SSprite* pSprite) { if (!pSprite) return false; target = pSprite; return true; }
    bool HasFunction() { return !!pFunction; }
    StateID GetState() { return state; }

    bool RegisterTargetField(SSprite::FieldID id) { fieldID = id; return true; }

    bool InitVariables();
    bool Apply(const std::string &dict);
    bool Apply(const std::string &key, double value);
    bool Apply(const std::string &key, const std::string &value);

    bool Tick(double delta);
    bool Execute() const { 
        return SSprite::SetField(target, fieldID, pFunction->Execute(variables));
    }
    bool Abort();

private:
    int reference;

    SSprite* target;
    MoverFunctionExpressionSharedPtr pFunction;
    MoverFunctionExpressionVariables variables;
    SSprite::FieldID fieldID;

    StateID state;
    double waiting;

    double time;
    double wait;
    double begin;
    double end;
    bool isBeginOffset;
    bool isEndOffset;
};

class SSprite;

class SSpriteMover final {
public:
    static asUINT StrTypeId;
private:
    SSprite* target;
    std::list<MoverObject *> moves;

public:
    SSpriteMover(SSprite* target)
        : target(target)
    {}

    ~SSpriteMover()
    {
        Abort(false);
    }

    void Tick(double delta);

    bool AddMove(const std::string &dict);
    bool AddMove(const std::string &key, const CScriptDictionary *dict);
    bool AddMove(MoverObject* pMover);

    void Abort(bool completeMove);
};
