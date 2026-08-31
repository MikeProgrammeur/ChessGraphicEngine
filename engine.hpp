#pragma once

#include <string>
#include <memory>
#include <vector>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
using EnginePid = HANDLE;
#else
#include <unistd.h>
using EnginePid = int;
#endif

enum class EngineType { UCI };

// ---------------------------------------------------------------------------
// Abstract interface
// ---------------------------------------------------------------------------
class EngineProtocol {
public:
    virtual ~EngineProtocol() = default;

    virtual bool start(const std::string& path) = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;
    virtual const std::string& name() const = 0;

    virtual void newGame() = 0;
    virtual void setMoveTime(int ms) = 0;
    virtual void setSkillLevel(int level) = 0;

    virtual void sendHumanMove(const std::string& uciMove) = 0;

    virtual void go() = 0;

    // returns the engine's move when available.
    virtual std::string poll() = 0;

    // Last principal variation line from the engine.
    virtual const std::string& lastPV() const = 0;

    virtual bool isThinking() const = 0;
};

std::unique_ptr<EngineProtocol> createEngine(EngineType type);

// ---------------------------------------------------------------------------
// UCI engine (Stockfish for example)
// ---------------------------------------------------------------------------
class UCIEngine : public EngineProtocol {
public:
    UCIEngine() = default;
    ~UCIEngine() override;

    bool start(const std::string& path) override;
    void stop() override;
    bool isRunning() const override {
#ifdef _WIN32
        return m_pid != nullptr;
#else
        return m_pid > 0;
#endif
    }
    const std::string& name() const override { return m_name; }

    void newGame() override;
    void setMoveTime(int ms) override { m_movetime = ms; }
    void setSkillLevel(int level) override;

    void sendHumanMove(const std::string& uciMove) override;
    void go() override;
    std::string poll() override;

    bool isThinking() const override { return m_thinking; }
    const std::string& lastPV() const override { return m_lastPV; }

private:
    void send(const std::string& cmd);
    bool readLine(std::string& out);

    EnginePid m_pid = static_cast<EnginePid>(0);
    FILE* m_in = nullptr;
    FILE* m_out = nullptr;
    bool m_thinking = false;
    int m_movetime = 1500;
    int m_skillLevel = 20;
    std::string m_name;
    std::string m_lastPV;
    std::vector<std::string> m_history;
};
