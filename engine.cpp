#include "engine.hpp"

#include <iostream>
#include <cstring>
#include <cstdlib>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#else
#include <sys/wait.h>
#include <sys/select.h>
#endif

// ---------------------------------------------------------------------------
// Cross-platform process spawn / wait / read helpers
// ---------------------------------------------------------------------------

#ifdef _WIN32

// Spawn `cmd` as a child process, wiring its stdin/stdout to `in`/`out`
// FILE* streams. On success returns a process HANDLE (or nullptr).
static EnginePid spawnProcess(const std::string& cmd, FILE** in, FILE** out) {
    HANDLE hInR, hInW, hOutR, hOutW;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    if (!CreatePipe(&hInR, &hInW, &sa, 0)) return nullptr;   // child stdin
    if (!CreatePipe(&hOutR, &hOutW, &sa, 0)) {               // child stdout
        CloseHandle(hInR); CloseHandle(hInW);
        return nullptr;
    }
    SetHandleInformation(hInW, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hOutR, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    std::memset(&si, 0, sizeof(si));
    std::memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = hInR;
    si.hStdOutput = hOutW;
    si.hStdError = hOutW;

    // CreateProcess wants a mutable command line
    std::string cmdLine = cmd;
    BOOL ok = CreateProcessA(nullptr, &cmdLine[0], nullptr, nullptr, TRUE,
                             CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
    CloseHandle(hInR);
    CloseHandle(hOutW);

    if (!ok) {
        CloseHandle(hInW);
        CloseHandle(hOutR);
        CloseHandle(pi.hThread);
        return nullptr;
    }

    CloseHandle(pi.hThread);

    // Convert child ends to CRT file descriptors, then to FILE*
    int inFd = _open_osfhandle((intptr_t)hInW, _O_WRONLY);
    int outFd = _open_osfhandle((intptr_t)hOutR, _O_RDONLY);
    *in = _fdopen(inFd, "w");
    *out = _fdopen(outFd, "r");
    if (!*in || !*out) {
        if (*in) fclose(*in); else _close(inFd);
        if (*out) fclose(*out); else _close(outFd);
        CloseHandle(pi.hProcess);
        return nullptr;
    }
    return pi.hProcess;
}

// Blocking read of one line; returns false on EOF
static bool readPipeLine(FILE* stream, std::string& out) {
    char buf[4096];
    if (!fgets(buf, sizeof(buf), stream)) return false;
    size_t end = strcspn(buf, "\r\n");
    buf[end] = '\0';
    out = buf;
    return true;
}

// Non-blocking: is data available to read?
static bool hasPipeData(FILE* stream) {
    if (!stream) return false;
    int fd = _fileno(stream);
    HANDLE h = (HANDLE)_get_osfhandle(fd);
    DWORD avail = 0;
    return PeekNamedPipe(h, nullptr, 0, nullptr, &avail, nullptr) && avail > 0;
}

static void waitProcess(EnginePid pid) {
    if (pid) {
        WaitForSingleObject(pid, INFINITE);
        CloseHandle(pid);
    }
}

#else // POSIX (Mac/Linux)

static EnginePid spawnProcess(const std::string& cmd, FILE** in, FILE** out) {
    int p2c[2], c2p[2];
    if (pipe(p2c) != 0 || pipe(c2p) != 0) return 0;

    EnginePid pid = fork();
    if (pid < 0) {
        close(p2c[0]); close(p2c[1]);
        close(c2p[0]); close(c2p[1]);
        return 0;
    }
    if (pid == 0) {
        dup2(p2c[0], 0);
        dup2(c2p[1], 1);
        close(p2c[0]); close(p2c[1]);
        close(c2p[0]); close(c2p[1]);
        execl(cmd.c_str(), cmd.c_str(), static_cast<char*>(nullptr));
        _exit(127);
    }

    close(p2c[0]);
    close(c2p[1]);
    *in = fdopen(p2c[1], "w");
    *out = fdopen(c2p[0], "r");
    if (!*in || !*out) {
        if (*in) fclose(*in);
        if (*out) fclose(*out);
        waitpid(pid, nullptr, 0);
        return 0;
    }
    return pid;
}

static bool readPipeLine(FILE* stream, std::string& out) {
    char buf[4096];
    if (!fgets(buf, sizeof(buf), stream)) return false;
    size_t end = strcspn(buf, "\r\n");
    out = std::string(buf, end);
    return true;
}

static bool hasPipeData(FILE* stream) {
    if (!stream) return false;
    int fd = fileno(stream);
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    struct timeval tv = {0, 0};
    return select(fd + 1, &fds, nullptr, nullptr, &tv) > 0;
}

static void waitProcess(EnginePid pid) {
    if (pid > 0) waitpid(pid, nullptr, 0);
}

#endif

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::string strip(const std::string& s) {
    size_t end = s.find_last_not_of("\n\r");
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

static bool hasData(FILE* stream) {
    return hasPipeData(stream);
}

static std::string extractBestMove(const std::string& line) {
    size_t pos = line.find("bestmove ");
    if (pos == std::string::npos) return "";
    size_t start = pos + 9;
    size_t end = line.find(' ', start);
    return line.substr(start, (end == std::string::npos) ? std::string::npos : end - start);
}

// ===========================================================================
//  UCIEngine
// ===========================================================================

UCIEngine::~UCIEngine() { stop(); }

void UCIEngine::send(const std::string& cmd) {
    if (!m_in) return;
    fprintf(m_in, "%s\n", cmd.c_str());
    fflush(m_in);
}

bool UCIEngine::readLine(std::string& out) {
    return readPipeLine(m_out, out);
}

bool UCIEngine::start(const std::string& path) {
    m_pid = spawnProcess(path, &m_in, &m_out);
#ifdef _WIN32
    if (!m_pid || !m_in || !m_out) { stop(); return false; }
#else
    if (m_pid <= 0 || !m_in || !m_out) { stop(); return false; }
#endif
    setvbuf(m_in, nullptr, _IONBF, 0);
    setvbuf(m_out, nullptr, _IONBF, 0);

    send("uci");
    std::string line;
    for (;;) {
        if (!readLine(line)) { stop(); return false; }
        if (line.compare(0, 7, "id name") == 0) m_name = line.substr(8);
        if (line == "uciok") break;
    }

    send("isready");
    for (;;) {
        if (!readLine(line)) { stop(); return false; }
        if (line == "readyok") break;
    }

    if (m_skillLevel >= 0 && m_skillLevel <= 20)
        send("setoption name Skill Level value " + std::to_string(m_skillLevel));

    return true;
}

void UCIEngine::stop() {
    if (m_in) { send("quit"); fclose(m_in); m_in = nullptr; }
    if (m_out) { fclose(m_out); m_out = nullptr; }
    waitProcess(m_pid);
    m_pid = static_cast<EnginePid>(0);
    m_thinking = false;
}

void UCIEngine::newGame() {
    m_history.clear();
    if (m_thinking) { send("stop"); m_thinking = false; }
    send("ucinewgame");
    if (m_skillLevel >= 0 && m_skillLevel <= 20)
        send("setoption name Skill Level value " + std::to_string(m_skillLevel));
    send("isready");
    std::string line;
    for (;;) {
        if (!readLine(line)) break;
        if (line == "readyok") break;
    }
}

void UCIEngine::setSkillLevel(int level) {
    m_skillLevel = level;
    if (m_in)
        send("setoption name Skill Level value " + std::to_string(level));
}

void UCIEngine::sendHumanMove(const std::string& uciMove) {
    m_history.push_back(uciMove);
}

void UCIEngine::go() {
    std::string cmd = "position startpos";
    if (!m_history.empty()) {
        cmd += " moves";
        for (const auto& m : m_history) cmd += " " + m;
    }
    send(cmd);
    send("go movetime " + std::to_string(m_movetime));
    m_thinking = true;
}

std::string UCIEngine::poll() {
    if (!m_out || !m_thinking) return "";

    while (hasData(m_out)) {
        char buf[4096];
        if (!fgets(buf, sizeof(buf), m_out)) { m_thinking = false; return ""; }
        std::string line = strip(buf);
        if (line.empty()) continue;

        // Parse info lines — keep the full line for display
        if (line.compare(0, 5, "info ") == 0) {
            m_lastPV = line.substr(5);
            continue;
        }

        if (line.compare(0, 8, "bestmove") == 0) {
            m_thinking = false;
            std::string move = extractBestMove(line);
            if (!move.empty() && move != "0000") {
                m_history.push_back(move);
                return move;
            }
            return "";
        }
    }
    return "";
}

std::unique_ptr<EngineProtocol> createEngine(EngineType /*type*/) {
    return std::make_unique<UCIEngine>();
}
