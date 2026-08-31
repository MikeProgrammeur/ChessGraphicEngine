#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <csignal>
#include <array>
#include <ctime>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "constants.hpp"
#include "types.hpp"
#include "textures.hpp"
#include "rendering.hpp"
#include "layout.hpp"
#include "engine.hpp"

// ---------------------------------------------------------------------------
// Starting position
// ---------------------------------------------------------------------------

static const Board INITIAL_BOARD = {{
    {1, 2, 3, 4, 5, 3, 2, 1},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {12, 12, 12, 12, 12, 12, 12, 12},
    {12, 12, 12, 12, 12, 12, 12, 12},
    {12, 12, 12, 12, 12, 12, 12, 12},
    {12, 12, 12, 12, 12, 12, 12, 12},
    {6, 6, 6, 6, 6, 6, 6, 6},
    {7, 8, 9, 10, 11, 9, 8, 7}
}};

// GUI ↔ UCI conversion
// GUI move: "{row1}_{col1}_to_{row2}_{col2}" (row 0 = rank 8, col 0 = file a)
// UCI move: "e2e4" (file letter + rank digit)
static std::string guiMoveToUCI(const std::string& guiMove, const Board& board) {
    int r1, c1, r2, c2;
    if (std::sscanf(guiMove.c_str(), "%d_%d_to_%d_%d", &r1, &c1, &r2, &c2) != 4)
        return "";

    char uci[6];
    uci[0] = static_cast<char>('a' + c1);
    uci[1] = static_cast<char>('1' + (7 - r1));
    uci[2] = static_cast<char>('a' + c2);
    uci[3] = static_cast<char>('1' + (7 - r2));

    // Auto-promote to queen if a pawn reaches the last rank
    int piece = board[r1][c1];
    bool reachesLastRank = (piece == 6 && r2 == 0) || (piece == 0 && r2 == 7);
    if (reachesLastRank) {
        uci[4] = 'q';
        uci[5] = '\0';
    } else {
        uci[4] = '\0';
    }
    return std::string(uci);
}

// Apply a UCI move to the board (handles castling, en passant, promotion)
static void applyUCIMove(Board& board, const std::string& uci,
                         CapturedPieces& capturedWhite, CapturedPieces& capturedBlack) {
    if (uci.length() < 4) return;

    int c1 = uci[0] - 'a';
    int r1 = 7 - (uci[1] - '1');
    int c2 = uci[2] - 'a';
    int r2 = 7 - (uci[3] - '1');

    int piece = board[r1][c1];
    int target = board[r2][c2];
    bool isWhite = (piece >= 6 && piece <= 11);

    // --- En passant: pawn diagonal to empty square ---
    bool isPawn = (piece == 0 || piece == 6);
    bool isEnPassant = isPawn && target == EMPTY_SQUARE && (c1 != c2);
    if (isEnPassant) {
        int captured = board[r1][c2];
        board[r1][c2] = EMPTY_SQUARE;
        if (isWhite && captured < 6)              capturedBlack[captured]++;
        else if (!isWhite && captured >= 6)       capturedWhite[captured - 6]++;
    } else if (target != EMPTY_SQUARE) {
        // --- Normal capture ---
        if (isWhite && target < 6)                capturedBlack[target]++;
        else if (!isWhite && target >= 6)         capturedWhite[target - 6]++;
    }

    // --- Castling: king moves 2 squares horizontally ---
    bool isKing = (piece == 5 || piece == 11);
    if (isKing && std::abs(c2 - c1) == 2) {
        if (c2 > c1) { // Kingside
            board[r1][5] = board[r1][7];
            board[r1][7] = EMPTY_SQUARE;
        } else {       // Queenside
            board[r1][3] = board[r1][0];
            board[r1][0] = EMPTY_SQUARE;
        }
    }

    // --- Promotion ---
    if (uci.length() > 4) {
        switch (uci[4]) {
            case 'q': piece = isWhite ? 10 : 4;  break;
            case 'r': piece = isWhite ?  7 : 1;  break;
            case 'b': piece = isWhite ?  9 : 3;  break;
            case 'n': piece = isWhite ?  8 : 2;  break;
        }
    }

    board[r2][c2] = piece;
    board[r1][c1] = EMPTY_SQUARE;
}

// Chess rules: check / checkmate detection
static bool isWhitePiece(int p) { return p >= 6 && p <= 11; }
static bool isBlackPiece(int p) { return p >= 0 && p <= 5; }

static bool inBounds(int r, int c) { return r >= 0 && r < 8 && c >= 0 && c < 8; }

// Is square (r,c) attacked by `byWhite` pieces?
static bool isSquareAttacked(const Board& b, int r, int c, bool byWhite) {
    // Knight attacks
    static const int knightDr[] = {-2,-2,-1,-1, 1, 1, 2, 2};
    static const int knightDc[] = {-1, 1,-2, 2,-2, 2,-1, 1};
    for (int d = 0; d < 8; ++d) {
        int nr = r + knightDr[d], nc = c + knightDc[d];
        if (!inBounds(nr, nc)) continue;
        int p = b[nr][nc];
        if (byWhite && p == 8) return true;
        if (!byWhite && p == 2) return true;
    }

    // Pawn attacks
    int pawnDir = byWhite ? 1 : -1;
    int pawnId   = byWhite ? 6 : 0;
    for (int dc = -1; dc <= 1; dc += 2) {
        int pr = r + pawnDir, pc = c + dc;
        if (inBounds(pr, pc) && b[pr][pc] == pawnId) return true;
    }

    // Sliding: rook/queen on straight lines
    static const int straightDr[] = {-1, 1, 0, 0};
    static const int straightDc[] = {0, 0, -1, 1};
    for (int d = 0; d < 4; ++d) {
        for (int dist = 1; dist < 8; ++dist) {
            int nr = r + straightDr[d]*dist, nc = c + straightDc[d]*dist;
            if (!inBounds(nr, nc)) break;
            int p = b[nr][nc];
            if (p == EMPTY_SQUARE) continue;
            if (byWhite && (p == 7 || p == 10)) return true;
            if (!byWhite && (p == 1 || p == 4)) return true;
            break;
        }
    }

    // Sliding: bishop/queen on diagonals
    static const int diagDr[] = {-1,-1, 1, 1};
    static const int diagDc[] = {-1, 1,-1, 1};
    for (int d = 0; d < 4; ++d) {
        for (int dist = 1; dist < 8; ++dist) {
            int nr = r + diagDr[d]*dist, nc = c + diagDc[d]*dist;
            if (!inBounds(nr, nc)) break;
            int p = b[nr][nc];
            if (p == EMPTY_SQUARE) continue;
            if (byWhite && (p == 9 || p == 10)) return true;
            if (!byWhite && (p == 3 || p == 4)) return true;
            break;
        }
    }

    // King attacks
    static const int kingDr[] = {-1,-1,-1, 0, 0, 1, 1, 1};
    static const int kingDc[] = {-1, 0, 1,-1, 1,-1, 0, 1};
    for (int d = 0; d < 8; ++d) {
        int nr = r + kingDr[d], nc = c + kingDc[d];
        if (!inBounds(nr, nc)) continue;
        int p = b[nr][nc];
        if (byWhite && p == 11) return true;
        if (!byWhite && p == 5) return true;
    }

    return false;
}

static GridPos findKing(const Board& b, bool white) {
    int kingId = white ? 11 : 5;
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            if (b[r][c] == kingId) return {r, c};
    return {-1, -1};
}

static bool isInCheck(const Board& b, bool whiteKing) {
    GridPos kp = findKing(b, whiteKing);
    if (!kp.isValid()) return false;
    return isSquareAttacked(b, kp.row, kp.col, !whiteKing);
}

// Simulate a move and check if the side that just moved left its own king in check
static bool moveLeavesKingInCheck(const Board& b, int r1, int c1, int r2, int c2, bool whiteMoving) {
    Board copy = b;
    int piece = copy[r1][c1];
    int target = copy[r2][c2];

    // Handle en passant capture
    bool isPawn = (piece == 0 || piece == 6);
    if (isPawn && target == EMPTY_SQUARE && c1 != c2) {
        copy[r1][c2] = EMPTY_SQUARE;
    }

    // Handle castling rook
    bool isKing = (piece == 5 || piece == 11);
    if (isKing && std::abs(c2 - c1) == 2) {
        if (c2 > c1) { copy[r1][5] = copy[r1][7]; copy[r1][7] = EMPTY_SQUARE; }
        else         { copy[r1][3] = copy[r1][0]; copy[r1][0] = EMPTY_SQUARE; }
    }

    copy[r2][c2] = piece;
    copy[r1][c1] = EMPTY_SQUARE;

    return isInCheck(copy, whiteMoving);
}

// Full castling validation
static bool canCastle(const Board& b, bool white, bool kingside) {
    int row = white ? 7 : 0;
    int king = white ? 11 : 5;
    int rook = white ? 7 : 1;
    bool byWhite = !white;

    if (b[row][4] != king) return false;

    if (kingside) {
        if (b[row][7] != rook) return false;
        if (b[row][5] != EMPTY_SQUARE || b[row][6] != EMPTY_SQUARE) return false;
        if (isSquareAttacked(b, row, 4, byWhite)) return false;
        if (isSquareAttacked(b, row, 5, byWhite)) return false;
        if (isSquareAttacked(b, row, 6, byWhite)) return false;
    } else {
        if (b[row][0] != rook) return false;
        if (b[row][1] != EMPTY_SQUARE || b[row][2] != EMPTY_SQUARE ||
            b[row][3] != EMPTY_SQUARE) return false;
        if (isSquareAttacked(b, row, 4, byWhite)) return false;
        if (isSquareAttacked(b, row, 3, byWhite)) return false;
        if (isSquareAttacked(b, row, 2, byWhite)) return false;
    }
    return true;
}

// Does this (r1,c1)->(r2,c2) move match the piece's movement pattern and
// respect blocking/en-passant rules? (Does not check castling or king safety.)
static bool isValidPiecePattern(const Board& b, int r1, int c1, int r2, int c2, bool whiteToMove) {
    int piece = b[r1][c1];
    int kind = piece % 6;
    int target = b[r2][c2];

    // Cannot move to (let alone "capture") a square occupied by one's own piece.
    if (whiteToMove && isWhitePiece(target)) return false;
    if (!whiteToMove && isBlackPiece(target)) return false;

    int dr = r2 - r1, dc = c2 - c1;

    switch (kind) {
        case 0: { // Pawn
            int dir = whiteToMove ? -1 : 1;
            int startRow = whiteToMove ? 6 : 1;
            if (dc == 0) {
                if (target != EMPTY_SQUARE) return false; // cannot capture straight
                if (dr == dir) return true;
                return (r1 == startRow && dr == 2 * dir && b[r1+dir][c1] == EMPTY_SQUARE);
            }
            if (std::abs(dc) == 1 && dr == dir) {
                if (target != EMPTY_SQUARE) return true; // normal capture
                // En passant
                if (b[r1][c2] == (whiteToMove ? 0 : 6)) return true;
            }
            return false;
        }
        case 1: // Rook
            if ((dr != 0) == (dc != 0)) return false; // not on a straight line
            {
                int sr = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);
                int sc = (dc == 0) ? 0 : (dc > 0 ? 1 : -1);
                int steps = std::max(std::abs(dr), std::abs(dc));
                for (int d = 1; d < steps; ++d)
                    if (b[r1+sr*d][c1+sc*d] != EMPTY_SQUARE) return false;
            }
            return true;
        case 2: // Knight
            return (std::abs(dr) == 2 && std::abs(dc) == 1) ||
                   (std::abs(dr) == 1 && std::abs(dc) == 2);
        case 3: // Bishop
            if (std::abs(dr) != std::abs(dc) || dr == 0) return false;
            {
                int sr = dr > 0 ? 1 : -1, sc = dc > 0 ? 1 : -1;
                for (int d = 1; d < std::abs(dr); ++d)
                    if (b[r1+sr*d][c1+sc*d] != EMPTY_SQUARE) return false;
            }
            return true;
        case 4: // Queen
            if (dr == 0 || dc == 0) {
                if (dr == 0 && dc == 0) return false;
                int sr = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);
                int sc = (dc == 0) ? 0 : (dc > 0 ? 1 : -1);
                int steps = std::max(std::abs(dr), std::abs(dc));
                for (int d = 1; d < steps; ++d)
                    if (b[r1+sr*d][c1+sc*d] != EMPTY_SQUARE) return false;
            } else if (std::abs(dr) == std::abs(dc)) {
                int sr = dr > 0 ? 1 : -1, sc = dc > 0 ? 1 : -1;
                for (int d = 1; d < std::abs(dr); ++d)
                    if (b[r1+sr*d][c1+sc*d] != EMPTY_SQUARE) return false;
            } else {
                return false;
            }
            return true;
        case 5: // King
            if (std::abs(dr) <= 1 && std::abs(dc) <= 1 && (dr != 0 || dc != 0)) return true;
            return (dr == 0 && std::abs(dc) == 2 && canCastle(b, whiteToMove, dc > 0));
    }
    return false;
}

// Validate a full move (piece pattern, castling legality, leaving king in check)
static bool isMoveLegal(const Board& b, const std::string& uci) {
    if (uci.length() < 4) return false;
    int c1 = uci[0] - 'a';
    int r1 = 7 - (uci[1] - '1');
    int c2 = uci[2] - 'a';
    int r2 = 7 - (uci[3] - '1');
    int piece = b[r1][c1];
    if (piece == EMPTY_SQUARE) return false;
    bool whiteMoving = isWhitePiece(piece);

    if (!isValidPiecePattern(b, r1, c1, r2, c2, whiteMoving)) return false;
    if (moveLeavesKingInCheck(b, r1, c1, r2, c2, whiteMoving)) return false;
    return true;
}

// Check if the side to move has any legal move
static bool hasLegalMove(const Board& b, bool whiteToMove) {
    for (int r1 = 0; r1 < 8; ++r1) {
        for (int c1 = 0; c1 < 8; ++c1) {
            int p = b[r1][c1];
            if (p == EMPTY_SQUARE) continue;
            bool pWhite = isWhitePiece(p);
            if (pWhite != whiteToMove) continue;

            for (int r2 = 0; r2 < 8; ++r2) {
                for (int c2 = 0; c2 < 8; ++c2) {
                    if (r1 == r2 && c1 == c2) continue;
                    if (!isValidPiecePattern(b, r1, c1, r2, c2, whiteToMove)) continue;
                    if (moveLeavesKingInCheck(b, r1, c1, r2, c2, whiteToMove)) continue;
                    return true; // Found at least one legal move
                }
            }
        }
    }
    return false;
}

static bool isCheckmate(const Board& b, bool whiteToMove) {
    return isInCheck(b, whiteToMove) && !hasLegalMove(b, whiteToMove);
}

static bool isStalemate(const Board& b, bool whiteToMove) {
    return !isInCheck(b, whiteToMove) && !hasLegalMove(b, whiteToMove);
}

// Sound generation
static sf::SoundBuffer generateTone(float frequency, int durationMs, float amplitude = 0.5f) {
    const unsigned int sampleRate = 44100;
    auto numSamples = static_cast<std::uint64_t>(sampleRate * durationMs / 1000);
    std::vector<std::int16_t> samples(numSamples);
    for (std::uint64_t i = 0; i < numSamples; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        float envelope = 1.0f - static_cast<float>(i) / numSamples;
        samples[i] = static_cast<std::int16_t>(
            amplitude * 32767.f * envelope *
            std::sin(2.0f * 3.14159265f * frequency * t));
    }
    std::vector<sf::SoundChannel> ch = {sf::SoundChannel::Mono};
    sf::SoundBuffer buf(samples.data(), numSamples, 1, sampleRate, ch);
    return buf;
}

// Font loading (cross-platform fallback chain)
static sf::Font loadFontCrossPlatform() {
    const char* paths[] = {
        // Linux
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/liberation-sans/LiberationSans-Regular.ttf",
        "/usr/share/fonts/noto/NotoSans-Regular.ttf",
        // macOS
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Supplemental/Helvetica.ttf",
        "/Library/Fonts/Arial.ttf",
        // Windows
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        // Relative (user can bundle a font)
        "./Assets/Font.ttf",
        "./font.ttf",
    };
    for (const char* p : paths) {
        sf::Font f;
        if (f.openFromFile(p)) return f;
    }
    std::cerr << "Warning: no font found — PV text will not be displayed.\n";
    return {};
}

int main(int argc, char* argv[]) {
#ifndef _WIN32
    std::signal(SIGPIPE, SIG_IGN);
#endif

    // --- Usage ---
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0]
                  << " <engine_path> [white|black] [skill_level]"
                     " s=<engine_minutes> h=<human_minutes>\n"
                  << "  e.g. " << argv[0] << " ./stockfish\n"
                  << "  e.g. " << argv[0] << " ./stockfish black\n"
                  << "  e.g. " << argv[0] << " ./stockfish black 8\n"
                  << "  e.g. " << argv[0] << " ./stockfish black 8 s=5 h=10\n"
                  << "  white|black : color the human plays (default white)\n"
                  << "  skill_level : 0 (weak) ... 20 (strongest, default 20)\n"
                  << "  s=<min>     : engine clock in minutes (default 20)\n"
                  << "  h=<min>     : human clock in minutes (default = engine)\n";
        return 1;
    }

    const char* enginePath = argv[1];
    int humanColor = 0; // 0 = human plays white
    int skillLevel = 20;
    int engineMinutes = 20;
    int humanMinutes = -1; // -1 => default to engineMinutes

    for (int i = 2; i < argc; ++i) {
        std::string a = argv[i];
        long v;
        if (a == "white" || a == "0")                 humanColor = 0;
        else if (a == "black" || a == "1")            humanColor = 1;
        else if (a.compare(0, 2, "s=") == 0) {
            v = std::stol(a.substr(2));
            if (v < 1) v = 1;
            engineMinutes = static_cast<int>(v);
        }
        else if (a.compare(0, 2, "h=") == 0) {
            v = std::stol(a.substr(2));
            if (v < 1) v = 1;
            humanMinutes = static_cast<int>(v);
        }
        else {
            v = std::stol(a);
            if (v < 0) v = 0;
            if (v > 20) v = 20;
            skillLevel = static_cast<int>(v);
        }
    }

    if (humanMinutes < 0) humanMinutes = engineMinutes;

    // --- Load textures ---
    TextureManager textures;
    if (!textures.loadAll()) {
        std::cerr << "Failed to load textures." << std::endl;
        return 1;
    }

    // --- Start engine ---
    EngineType engineType = EngineType::UCI;
    auto engine = createEngine(engineType);
    bool engineAvailable = engine->start(enginePath);
    if (!engineAvailable)
        std::cerr << "Engine '" << enginePath << "' not found — local play mode." << std::endl;
    else
        engine->setSkillLevel(skillLevel);

    // --- Font ---
    sf::Font font = loadFontCrossPlatform();

    // --- Sounds ---
    sf::SoundBuffer checkBuf     = generateTone(880.f, 250, 0.9f);
    sf::SoundBuffer mateBuf      = generateTone(220.f, 800, 0.9f);
    sf::SoundBuffer stalemateBuf = generateTone(330.f, 400, 0.9f);
    sf::Sound soundCheck(checkBuf);
    sf::Sound soundMate(mateBuf);
    sf::Sound soundStalemate(stalemateBuf);

    // --- Window ---
    std::string title = "Chess";
    if (engineAvailable)
        title += " - " + engine->name() + " niveau " + std::to_string(skillLevel);
    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), title);
    window.setFramerateLimit(MAX_FPS);

    // --- Game state ---
    Board board = INITIAL_BOARD;
    CapturedPieces capturedBlack = {0};
    CapturedPieces capturedWhite = {0};
    int whiteMinutes = (humanColor == 0) ? humanMinutes : engineMinutes;
    int blackMinutes = (humanColor == 1) ? humanMinutes : engineMinutes;
    int secLeftWhite = whiteMinutes * 60;
    int secLeftBlack = blackMinutes * 60;

    bool gameRunning = true;
    bool engineThinking = false;
    bool clockStarted = false;
    GridPos selectedSquare;
    std::vector<std::string> moveHistory;

    bool mousePressed = false;
    bool mousePrevPressed = false;
    sf::Clock flickerClock;
    sf::Clock gameClock;
    float clockAccum = 0.f;
    std::string statusText;

    // If human plays black, let engine move first
    if (humanColor == 1 && engineAvailable) {
        engine->go();
        engineThinking = true;
    }

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                soundCheck.stop();
                soundMate.stop();
                soundStalemate.stop();
                window.close();
            }
        }

        window.clear(COLOR_LIGHT_SQUARE);

        // --- Input ---
        mousePrevPressed = mousePressed;
        mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
        bool mouseClicked = mousePressed && !mousePrevPressed;
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        InputState input{mouseClicked, mousePos};

        // --- Game clock (ticks for the side to move, even during engine thinking) ---
        sf::Time gameDt = gameClock.restart();
        if (gameRunning && clockStarted) {
            clockAccum += gameDt.asSeconds();
            while (clockAccum >= 1.f) {
                clockAccum -= 1.f;
                bool whiteToMove = (moveHistory.size() % 2 == 0);
                if (whiteToMove) secLeftWhite--;
                else             secLeftBlack--;
                if (secLeftWhite < 0) secLeftWhite = 0;
                if (secLeftBlack < 0) secLeftBlack = 0;
            }

            // Flag fall: time forfeit
            if (secLeftWhite == 0 || secLeftBlack == 0) {
                soundMate.play();
                if (secLeftWhite == 0 && secLeftBlack == 0)
                    statusText = "Draw - both flagged!";
                else if (secLeftWhite == 0)
                    statusText = "White flag! Black wins.";
                else
                    statusText = "Black flag! White wins.";
                gameRunning = false;
                engineThinking = false;
            }
        }

        // --- Poll engine (always, even when paused, so we don't miss bestmove) ---
        if (engineThinking && engineAvailable) {
            std::string engineMove = engine->poll();
            if (!engineMove.empty()) {
                applyUCIMove(board, engineMove, capturedWhite, capturedBlack);
                moveHistory.push_back(engineMove);
                clockStarted = true;
                engineThinking = false;

                // Check / checkmate / stalemate after engine move
                bool whiteToMove = (moveHistory.size() % 2 == 0);
                if (isCheckmate(board, whiteToMove)) {
                    soundMate.play();
                    statusText = (whiteToMove ? "White" : "Black") + std::string(" is checkmated!");
                } else if (isInCheck(board, whiteToMove)) {
                    soundCheck.play();
                    statusText = (whiteToMove ? "White" : "Black") + std::string(" is in check!");
                } else if (isStalemate(board, whiteToMove)) {
                    soundStalemate.play();
                    statusText = "Stalemate!";
                } else {
                    statusText.clear();
                }
            }
        }

        // --- Board interaction (only if game running and not engine's turn) ---
        sf::Time elapsed = flickerClock.getElapsedTime();
        int elapsedMillis = 2 * elapsed.asMilliseconds();

        bool whiteToMove = (moveHistory.size() % 2 == 0);
        bool isHumanTurn = gameRunning && !engineThinking &&
            (engineAvailable
                ? ((humanColor == 0 && whiteToMove) || (humanColor == 1 && !whiteToMove))
                : true); // local play: both sides are human

        std::string boardMove;
        GridPos clickedSquare;
        {
            DisplayState state{board, selectedSquare, elapsedMillis,
                               capturedBlack, capturedWhite,
                               secLeftBlack, secLeftWhite, gameRunning};
            clickedSquare = renderChessBoard(
                window, WINDOW_WIDTH, BOARD_AREA_HEIGHT, state, textures, input);
        }

        if (clickedSquare.isValid() && mouseClicked && isHumanTurn) {
            if (!selectedSquare.isValid()) {
                int piece = board[clickedSquare.row][clickedSquare.col];
                bool isWhitePiece = (piece >= 6 && piece <= 11);
                bool isBlackPiece = (piece >= 0 && piece <= 5);
                if ((humanColor == 0 && isWhitePiece) ||
                    (humanColor == 1 && isBlackPiece) ||
                    (!engineAvailable)) { // local play: any piece
                    selectedSquare = clickedSquare;
                    flickerClock.restart();
                }
            } else {
                if (selectedSquare == clickedSquare) {
                    selectedSquare = {-1, -1};
                } else {
                    boardMove = std::to_string(selectedSquare.row) + "_" +
                                std::to_string(selectedSquare.col) + "_to_" +
                                std::to_string(clickedSquare.row) + "_" +
                                std::to_string(clickedSquare.col);
                    selectedSquare = {-1, -1};
                }
            }
        }

        // --- Side panel (buttons, clocks, captured pieces) ---
        std::string buttonAction;
        {
            DisplayState state{board, selectedSquare, elapsedMillis,
                               capturedBlack, capturedWhite,
                               secLeftBlack, secLeftWhite, gameRunning};
            buttonAction = renderSidePanel(
                window, BOARD_AREA_HEIGHT, WINDOW_WIDTH,
                state, gameRunning, textures, input);

            // Handle reset
            if (buttonAction == "ucinewgame") {
                board = INITIAL_BOARD;
                capturedBlack = {0};
                capturedWhite = {0};
                secLeftBlack = blackMinutes * 60;
                secLeftWhite = whiteMinutes * 60;
                moveHistory.clear();
                selectedSquare = {-1, -1};
                engineThinking = false;
                clockStarted = false;
                statusText.clear();
                if (engineAvailable) {
                    engine->newGame();
                    if (humanColor == 1) {
                        engine->go();
                        engineThinking = true;
                    }
                }
            }
        }

        // --- Send human move to engine ---
        if (!boardMove.empty() && isHumanTurn) {
            std::string uciMove = guiMoveToUCI(boardMove, board);
            if (!uciMove.empty() && isMoveLegal(board, uciMove)) {
                applyUCIMove(board, uciMove, capturedWhite, capturedBlack);
                moveHistory.push_back(uciMove);
                clockStarted = true;

                // Check / checkmate / stalemate after human move
                bool wtm = (moveHistory.size() % 2 == 0);
                if (isCheckmate(board, wtm)) {
                    soundMate.play();
                    statusText = (wtm ? "White" : "Black") + std::string(" is checkmated!");
                } else if (isInCheck(board, wtm)) {
                    soundCheck.play();
                    statusText = (wtm ? "White" : "Black") + std::string(" is in check!");
                } else if (isStalemate(board, wtm)) {
                    soundStalemate.play();
                    statusText = "Stalemate!";
                } else {
                    statusText.clear();
                }

                if (engineAvailable) {
                    engine->sendHumanMove(uciMove);
                    engine->go();
                    engineThinking = true;
                }
            }
        }

        // --- PV zone (below the board) ---
        {
            std::string pv = engineAvailable ? engine->lastPV() : "";
            renderPVZone(window, WINDOW_WIDTH, BOARD_AREA_HEIGHT,
                         PV_ZONE_HEIGHT, pv, font, statusText);
        }

        // --- Screenshot (after full render) ---
        if (buttonAction == "__screenshot__") {
            sf::Texture screenshotTex(window.getSize());
            screenshotTex.update(window);
            sf::Image img = screenshotTex.copyToImage();
            std::time_t now = std::time(nullptr);
            std::tm* t = std::localtime(&now);
            char buf[64];
            std::snprintf(buf, sizeof(buf), "./ScreenShots/screenshot_%04d%02d%02d_%02d%02d%02d.jpg",
                          t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                          t->tm_hour, t->tm_min, t->tm_sec);
            (void)img.saveToFile(buf);
        }

        window.display();
    }

    engine->stop();
    return 0;
}
