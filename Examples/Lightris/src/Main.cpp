#include <Arclight/Colour.h>
#include <Arclight/Core/Application.h>
#include <Arclight/Core/Input.h>
#include <Arclight/Core/Logger.h>
#include <Arclight/Core/ResourceManager.h>
#include <Arclight/ECS/World.h>
#include <Arclight/Graphics/Image.h>
#include <Arclight/Graphics/Sprite.h>
#include <Arclight/Graphics/Text.h>
#include <Arclight/Graphics/Texture.h>
#include <Arclight/Systems/Renderer2D.h>
#include <Arclight/Systems/StdoutFPSCounter.h>

#include <array>
#include <cassert>

#include "Block.h"

using namespace Arclight;

// In the case that the piece is obstructed by the wall or another block,
// test all of these translations until one succeeds.
// If none succeed, the piece is stuck and cannot be rotated
// This is known as 'wall kick'
constexpr std::array<Vector2i, 5> obstructionTests = {
    Vector2i{0, 0},   // No translation
    Vector2i{-1, 0},  // Left one
    Vector2i{-1, -1}, // Left one and up one
    Vector2i{0, 2},   // Down two
    Vector2i{-1, 2}   // Left one, down two
};

// The I piece needs difference translations
constexpr std::array<Vector2i, 5> iPieceObstructionTests = {
    Vector2i{0, 0},   // No translation
    Vector2i{-2, 0},  // Left two
    Vector2i{-2, -1}, // Left two and up one
    Vector2i{1, 0},   // Right one
    Vector2i{-1, 2}   // Left one, down two
};

// clang-format off
constexpr const char pieces[7][17] = {
    {
        "11  "
        " 11 "
        "    "
        "    "
    },
    {
        " 22 "
        "22  "
        "    "
        "    "
    },
    {
        " 33 "
        " 33 "
        "    "
        "    "
    },
    {
        "  4 "
        "444 "
        "    "
        "    "
    },
    {
        "5   "
        "555 "
        "    "
        "    "
    },
    {
        "  6 "
        "  6 "
        "  6 "
        "  6 "
    },
    {
        " 7  "
        "777 "
        "    "
        "    "
    }
};
// clang-format on

constexpr const Vector2i pieceOrigins[7] = {
    {1, -1}, {1, -1}, {2, -1}, {1, -1}, {1, -1}, {2, -2}, {1, -1},
};

#define BOARD_HEIGHT 20
#define BOARD_WIDTH 10

using Board = std::array<std::array<Entity, BOARD_WIDTH>, BOARD_HEIGHT>;

#define MAX_MOVE_RESETS 15

Texture* blockTexture;
Texture* boardTexture;

Vector2f boardScreenPos;

struct Piece {
    struct Block {
        int type;
        Vector2i position;
        Entity sprite;
    };

    int type;
    Vector2i origin;

    // Blocks composing the piece
    std::vector<Block> blocks;
    std::vector<Entity> sprites;

    // Time elapsed since the piece was last moved down
    float elaspedSinceLastMove = 0.0f;

    bool isLocked = false;
    // Remaining lock delay
    float lockDelay = 0.0f;
    int remainingMoveResets = MAX_MOVE_RESETS;
};

struct PieceQueue {
    std::list<int> queuedPieces;

    bool alreadySwapped = false; // Piece has already been swapped?
    Piece holdPiece = {0};       // Piece that is currently being held
};

struct GameState {
    static const int das = 9;       // Delayed auto shift (In 60th/seconds)
    static const int lockDelay = 30; // Lock delay (60th/seconds)
    // Amount of times the piece can be rotated to reset lock delay
    static const int maxMoveResets = MAX_MOVE_RESETS;

    // In lines/60th seconds
    float startGravity = 0.02f;
    float softDropGravity = 1;
    float maxGravity = 1;

    float currentGravity = startGravity;
    float dasTimer = das;
};

enum {
    StateMenu,
    StateGame,
};

inline Vector2f BlockToPixelCoords(const Vector2i& blockCoords) {
    // Blocks are drawn at an offset from the corner of the screen
    return boardScreenPos +
           Vector2f{static_cast<float>(blockCoords.x) * BLOCK_SIZE,
                    static_cast<float>(BOARD_HEIGHT - blockCoords.y - 1) * BLOCK_SIZE};
}

Entity ConstructBlockSprite(World& world, int block, int x, int y) {
    Entity entity = world.CreateEntity();

    // Make sure it is a valid block type
    assert(block > 0 && block < 8);

    // Create sprite object with block texture
    Sprite spr = CreateSprite({BLOCK_SIZE}, Rectf(Vector2f{1.0f}), blockColours[block].AsFloat());
    spr.texture = blockTexture;
    spr.transform.SetPosition(BlockToPixelCoords({x, y}));
    spr.transform.SetZIndex(1.f);

    // Add the Sprite and Block to the block as a component
    world.AddComponent<Sprite>(entity, spr);
    return entity;
}

Entity ConstructPiece(World& world, int piece, int x, int y) {
    Piece p;
    assert(piece > 0 && piece <= 7);

    p.type = piece;
    p.origin = {x, y};

    const char* pieceData = pieces[piece - 1];
    const Vector2i& pieceOrigin = pieceOrigins[piece - 1];
    for (int i = 0; i < 4; i++) {
        const char* row = pieceData + i * 4;
        for (int j = 0; j < 4; j++) {
            // Pieces are represented at numerical characters
            if (isdigit(row[j])) {
                int bType = row[j] - '0';

                Entity ent = ConstructBlockSprite(world, bType, j - pieceOrigin.x + x,
                                                  (-i) - pieceOrigin.y + y);
                p.sprites.push_back(ent);
                p.blocks.push_back(
                    Piece::Block{bType, {j - pieceOrigin.x, (-i) - pieceOrigin.y}, ent});
            } else
                assert(isspace(row[j]));
        }
    }

    Entity entity = world.CreateEntity();
    world.AddComponent<Piece>(entity, std::move(p));

    return entity;
}

PieceQueue& GetPieceQueue(World& world) {
    auto queueView = world.View<PieceQueue>();
    assert(queueView.size() == 1);

    return queueView.get<PieceQueue>(queueView.front());
}

void DestroyPiece(World& world, Piece& piece, Entity entityID) {
    for (Entity sprite : piece.sprites) {
        world.DestroyEntity(sprite);
    }
    world.DestroyEntity(entityID);

    // If the piece is destroyed,
    // allow the player to swap places again
    auto& q = GetPieceQueue(world);
    q.alreadySwapped = false;
}

void MenuInit(float, World& world) {
    Entity titleEntity = world.CreateEntity();
    Entity footnoteEntity = world.CreateEntity();
    Entity controlsEntity = world.CreateEntity();

    WindowContext* window = WindowContext::Instance();

    std::shared_ptr<Font> font =
        ResourceManager::Instance().GetResource<Font>("assets/inconsolata.ttf");
    assert(font.get());

    Text text("Lightris");
    text.SetFontSize(64);
    text.SetFont(font);
    Text text2("Made with Arclight Engine https://arclightengine.org");
    text2.SetFontSize(20);
    text2.SetFont(font);
    text.transform.SetPosition(
        Vector2f{window->GetSize().x / 2.f - text.Bounds().x / 2.f, window->GetSize().y / 3.f});
    text2.transform.SetPosition(Vector2f{5.f, static_cast<float>(window->GetSize().y) - 25});

    Text controls("Press R to start and restart.\n"
                  "Move pieces with LEFT and RIGHT ARROW keys\n"
                  "Rotate piece clockwise with UP ARROW\n"
                  "Hold piece with LEFT SHIFT\n"
                  "Soft drop with DOWN ARROW\n"
                  "Hard drop with SPACE");
    controls.SetFontSize(20);
    controls.SetFont(font);
    controls.transform.SetPosition({36, window->GetSize().y / 2.f});

    world.AddComponent<Text>(titleEntity, std::move(text));
    world.AddComponent<Text>(footnoteEntity, std::move(text2));
    world.AddComponent<Text>(controlsEntity, std::move(controls));
}

void MenuSystem(float, World& world) {
    if (Input::GetKeyPress(KeyCode_R)) {
        Application::Instance().commands.LoadState<StateGame>();
        Application::Instance().commands.LoadWorld(std::make_shared<World>());
    }
}

int GetNextPiece(World& world) {
    auto& queue = GetPieceQueue(world);
    assert(queue.queuedPieces.size());

    int value = queue.queuedPieces.front();
    queue.queuedPieces.pop_front();
    return value;
}

// Use a bag of 7 pieces
std::list<int> GeneratePieceBag() {
    std::list<int> bag = {1, 2, 3, 4, 5, 6, 7};

    for (int i = 0; i < 20; i++) {
        bag.sort([](int a, int b) -> bool { return a * rand() < b * rand(); });
    }

    return bag;
}

void BoardInit(float, World& world) {
    Entity boardEntity = world.CreateEntity();
    Board board;
    for (unsigned i = 0; i < board.size(); i++) {
        board.at(i).fill(entt::null);
    }

    srand(time(NULL));

    WindowContext& window = *WindowContext::Instance();
    boardScreenPos.x = window.GetSize().x / 2 - (BOARD_WIDTH / 2 * BLOCK_SIZE);
    boardScreenPos.y = BLOCK_SIZE;

    Sprite boardSprite =
        CreateSprite({(BOARD_WIDTH + 2) * BLOCK_SIZE, (BOARD_HEIGHT + 1) * BLOCK_SIZE});
    boardSprite.texture = boardTexture;
    boardSprite.transform.SetPosition(boardScreenPos - Vector2f{BLOCK_SIZE, 0});
    boardSprite.transform.SetZIndex(-10);

    world.AddComponent<PieceQueue>(boardEntity, PieceQueue{GeneratePieceBag()});
    world.AddComponent<Board>(boardEntity, std::move(board));
    world.AddComponent<Sprite>(boardEntity, std::move(boardSprite));
    world.AddComponent<GameState>(boardEntity);

    ConstructPiece(world, GetNextPiece(world), 2, 20);
}

void ClearBoard(World& world, Board& board) {
    for (auto& row : board) {
        for (auto& block : row) {
            if (block != entt::null) {
                world.DestroyEntity(block);
            }
        }

        row.fill(entt::null);
    }
}

//////////////////////////////////////////////////////////
/// \brief Take a copy of the piece and translate it
///
/// \param blocks Piece block list value copy
/// \param translation Translation in board squares
///
/// \return Piece value copy with translation applied
//////////////////////////////////////////////////////////
std::vector<Piece::Block> MovePieceAndCopy(std::vector<Piece::Block> blocks,
                                           const Vector2i& translation) {
    for (auto& block : blocks) {
        block.position += translation;
    }

    return blocks;
}

void UpdatePieceBlocks(World& world, Piece& piece) {
    for (auto& block : piece.blocks) {
        auto& sprite = world.GetComponent<Sprite>(block.sprite);
        sprite.transform.SetPosition(BlockToPixelCoords(piece.origin + block.position));
    }
}

// Check if a piece would be obstructed
bool WouldBeObstructed(Board& board, const Vector2i& origin,
                       const std::vector<Piece::Block>& blocks) {
    for (auto& block : blocks) {
        // Outside the walls
        if (origin.y + block.position.y < 0 || origin.x + block.position.x < 0 ||
            origin.x + block.position.x >= BOARD_WIDTH) {
            return true;
        }

        // Check if there is a block in the way
        if (origin.y + block.position.y < BOARD_HEIGHT &&
            board[origin.y + block.position.y][origin.x + block.position.x] != entt::null) {
            return true;
        }
    }

    return false;
}

void MoveBlocksToBoard(World& world, Board& board, Piece& piece) {
    for (auto& block : piece.blocks) {
        if (piece.origin.y + block.position.y < BOARD_HEIGHT) {
            board[piece.origin.y + block.position.y][piece.origin.x + block.position.x] =
                block.sprite;
        } else {
            world.DestroyEntity(block.sprite);
        }
    }
    piece.blocks.clear();
    piece.sprites.clear();
}

void RotatePieceClockwise(Board& board, Piece& piece, float lockDelay) {
    if (piece.isLocked) {
        if (piece.remainingMoveResets <= 0) {
            return; // Cannot be rotated any more
        } else {
            piece.remainingMoveResets--;
            piece.lockDelay = lockDelay;
        }
    }

    // Rotate around the origin
    if (piece.type == BlockYellow) {
        return; // Don't rotate O piece
    }

    std::vector<Piece::Block> rotatedBlocks;
    for (auto& block : piece.blocks) {
        auto newBlock = block;
        // Need to negate the y-values again as they are relative to bottom of the board
        newBlock.position = {-(-block.position.y), -(block.position.x)};
        rotatedBlocks.push_back(newBlock);
    }

    // Check if the piece would be obstructed
    Vector2i origin = piece.origin;
    if (WouldBeObstructed(board, origin, rotatedBlocks)) {
        const auto* tests = obstructionTests.data();
        int testCount = obstructionTests.size();

        // Use different obstruction tests depending on the piece type
        if (piece.type == BlockCyan) {
            tests = iPieceObstructionTests.data();
            testCount = iPieceObstructionTests.size();
        }

        int i = 0;
        while (i < testCount && WouldBeObstructed(board, origin, rotatedBlocks)) {
            origin = piece.origin + tests[i];
            i++;
        }

        // Try again with the negative
        i = 0;
        while (i < testCount && WouldBeObstructed(board, origin, rotatedBlocks)) {
            origin = piece.origin - tests[i];
            i++;
        }
    }

    if (!WouldBeObstructed(board, origin, rotatedBlocks)) {
        piece.origin = origin;
        piece.blocks = std::move(rotatedBlocks);
    }
}

void RotatePieceCounterClockwise(Board& board, Piece& piece, float lockDelay) {
    if (piece.isLocked) {
        if (piece.remainingMoveResets <= 0) {
            return; // Cannot be rotated any more
        } else {
            piece.remainingMoveResets--;
            piece.lockDelay = lockDelay;
        }
    }

    // Rotate around the origin
    if (piece.type == BlockYellow) {
        return; // Don't rotate O piece
    }

    std::vector<Piece::Block> rotatedBlocks;
    for (auto& block : piece.blocks) {
        auto newBlock = block;
        // Need to negate the y-values again as they are relative to bottom of the board
        newBlock.position = {(-block.position.y), (block.position.x)};
        rotatedBlocks.push_back(newBlock);
    }

    // Check if the piece would be obstructed
    Vector2i origin = piece.origin;
    if (WouldBeObstructed(board, origin, rotatedBlocks)) {
        const auto* tests = obstructionTests.data();
        int testCount = obstructionTests.size();

        // Use different obstruction tests depending on the piece type
        if (piece.type == BlockCyan) {
            tests = iPieceObstructionTests.data();
            testCount = iPieceObstructionTests.size();
        }

        int i = 0;
        while (i < testCount && WouldBeObstructed(board, origin, rotatedBlocks)) {
            origin = piece.origin + tests[i];
            i++;
        }

        // Try again with the negative
        i = 0;
        while (i < testCount && WouldBeObstructed(board, origin, rotatedBlocks)) {
            origin = piece.origin - tests[i];
            i++;
        }
    }

    if (!WouldBeObstructed(board, origin, rotatedBlocks)) {
        piece.origin = origin;
        piece.blocks = std::move(rotatedBlocks);
    }
}

void DestroyLineEntities(World& world, std::array<Entity, BOARD_WIDTH>& line) {
    for (Entity ent : line) {
        if (ent != entt::null) {
            world.DestroyEntity(ent);
        }
    }
}

void ClearLinesIfNeeded(World& world, Board& board) {
    // Iterate top-down
    bool hasClearedLine = false;
    for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
        bool lineIsFull = true;
        for (Entity x : board[y]) {
            // If any value is a null entity,
            // the line is not full
            if (x == entt::null) {
                lineIsFull = false;
                break;
            }
        }

        // Line is full, clear it
        if (lineIsFull) {
            Logger::Debug("Line ", y, " cleared!");
            hasClearedLine = true;

            DestroyLineEntities(world, board[y]);
            for (int i = y + 1; i < BOARD_HEIGHT; i++) {
                board[i - 1] = std::move(board[i]);
            }
        }
    }

    // Reiterate to update the sprites (if needed)
    if (hasClearedLine) {
        for (int y = 0; y < BOARD_HEIGHT; y++) {
            for (int x = 0; x < BOARD_WIDTH; x++) {
                Entity block = board[y][x];
                if (block != entt::null)
                    world.GetComponent<Sprite>(block).transform.SetPosition(
                        BlockToPixelCoords(Vector2i{x, y}));
            }
        }
    }
}

void BoardSystem(float elapsed, World& world) {
    // Get Board Data
    auto boardView = world.View<Board>();
    assert(boardView.size() == 1);
    PieceQueue& queue = GetPieceQueue(world);
    Board& board = boardView.get<Board>(boardView.front());
    GameState& gameState = world.GetComponent<GameState>(boardView.front());

    auto pieceView = world.View<Piece>();
    if (Input::GetKeyPress(KeyCode_R)) { // Reset game
        ClearBoard(world, board);
        if (!pieceView.empty()) {
            auto& piece = pieceView.get<Piece>(pieceView.front());
            DestroyPiece(world, piece, pieceView.front());
        }

        ConstructPiece(world, GetNextPiece(world), 5, BOARD_HEIGHT);
        return;
    }

    if (!pieceView.empty()) {
        // Make sure there is one piece
        assert(pieceView.size() == 1);
        auto& piece = pieceView.get<Piece>(pieceView.front());
        piece.elaspedSinceLastMove += elapsed;

        auto movePiece = [&](int direction) {
            if (piece.isLocked) {
                piece.remainingMoveResets--;
                piece.lockDelay = gameState.lockDelay / 60.f;
            }

            Vector2i translateVector = {direction, 0};
            if (!WouldBeObstructed(board, piece.origin + translateVector, piece.blocks)) {
                piece.origin += translateVector;
                UpdatePieceBlocks(world, piece);
            }
        };

        if (Input::GetKeyPress(KeyCode_Left)) {
            movePiece(-1);
            gameState.dasTimer = gameState.das / 60.f;
        } else if (Input::GetKeyPress(KeyCode_Right)) {
            movePiece(1);
            gameState.dasTimer = gameState.das / 60.f;
        }

        if (Input::GetKeyDown(KeyCode_Left)) {
            if (gameState.dasTimer <= 0) {
                movePiece(-1);
            } else
                gameState.dasTimer -= elapsed;
        } else if (Input::GetKeyDown(KeyCode_Right)) {
            if (gameState.dasTimer <= 0) {
                movePiece(1);
            } else
                gameState.dasTimer -= elapsed;
        }

        if (Input::GetKeyPress(KeyCode_Up)) {
            RotatePieceClockwise(board, piece, gameState.lockDelay / 60.f);
            UpdatePieceBlocks(world, piece);
        }

        float dropSpeed = 1.f / (gameState.currentGravity * 60.f);
        if (Input::GetKeyDown(KeyCode_Down)) {
            dropSpeed = 1.f / (gameState.softDropGravity * 60.f);
        }

        if (Input::GetKeyPress(KeyCode_LShift)) {
            if (!queue.alreadySwapped) {
                // Check if there is a piece
                if (queue.holdPiece.type < 1) {
                    queue.holdPiece = std::move(piece);
                    DestroyPiece(world, piece, pieceView.front());

                    // Add a new piece
                    ConstructPiece(world, GetNextPiece(world), 5, BOARD_HEIGHT);
                } else {
                    // Temporarily move the current piece
                    Piece temp = std::move(piece);
                    piece = std::move(queue.holdPiece);
                    piece.origin = Vector2i{5, BOARD_HEIGHT};

                    queue.holdPiece = std::move(temp);
                    UpdatePieceBlocks(world, piece);
                }

                queue.holdPiece.origin = {
                    -4, BOARD_HEIGHT - 1}; // Place the hold piece on the left, out of the board
                queue.alreadySwapped = true;
                UpdatePieceBlocks(world, queue.holdPiece);
            }
        }

        if (Input::GetKeyPress(KeyCode_Space)) {
            while (!WouldBeObstructed(board, piece.origin - Vector2{0, 1}, piece.blocks)) {
                piece.origin.y--;
            }

            UpdatePieceBlocks(world, piece);

            MoveBlocksToBoard(world, board, piece);
            DestroyPiece(world, piece, pieceView.front());

            ConstructPiece(world, GetNextPiece(world), 5, BOARD_HEIGHT);
            ClearLinesIfNeeded(world, board);
        } else if (piece.isLocked) {
            piece.lockDelay -= elapsed;

            if (!WouldBeObstructed(board, piece.origin - Vector2i{0, 1}, piece.blocks)) {
                piece.isLocked = false; // There is space under the piece
                piece.elaspedSinceLastMove = 0;
            } else if (piece.lockDelay <= 0) { // Out of lock delay??
                MoveBlocksToBoard(world, board, piece);
                ClearLinesIfNeeded(world, board);
                DestroyPiece(world, piece, pieceView.front());

                ConstructPiece(world, GetNextPiece(world), 5, BOARD_HEIGHT);
            }
        } else if (piece.elaspedSinceLastMove > dropSpeed) {
            piece.elaspedSinceLastMove -= dropSpeed;

            piece.origin.y--;
            UpdatePieceBlocks(world, piece);

            if (WouldBeObstructed(board, piece.origin - Vector2i{0, 1}, piece.blocks)) {
                piece.isLocked = true;
                piece.lockDelay = gameState.lockDelay / 60.f;
            }
        }
    }
}

void QueueSystem(float, World& world) {
    auto queueView = world.View<PieceQueue>();
    assert(queueView.size() == 1);

    auto& queue = queueView.get<PieceQueue>(queueView.front());
    if (queue.queuedPieces.size() <= 7) {
        queue.queuedPieces.splice(queue.queuedPieces.end(), GeneratePieceBag());
    }
}

extern "C" {
void GameInit() {
    auto& app = Application::Instance();
    Logger::Debug("Starting Game!");

    app.Window().SetTitle("Lightris");
    app.Window().SetSize({704, 704});

    auto blockImage = ResourceManager::Instance().GetResource<Image>("assets/block.png");
    auto boardImage = ResourceManager::Instance().GetResource<Image>("assets/board.png");
    assert(blockImage.get());
    assert(boardImage.get());

    // Create texture from block image resource
    blockTexture = new Texture(*blockImage);
    boardTexture = new Texture(*boardImage);

    StdoutFPSCounter fpsCounter;

    app.AddState<StateMenu>();
    app.AddState<StateGame>();
    app.commands.LoadState<StateMenu>();

    app.AddSystem<Systems::Renderer2D>();
    app.AddSystem<StdoutFPSCounter, &StdoutFPSCounter::Tick, Application::When::Tick>(fpsCounter);
    app.AddSystem<MenuInit, Application::When::Init, StateMenu>();
    app.AddSystem<MenuSystem, Application::When::Tick, StateMenu>();
    app.AddSystem<BoardInit, Application::When::Init, StateGame>();
    app.AddSystem<BoardSystem, Application::When::Tick, StateGame>();
    app.AddSystem<QueueSystem, Application::When::Tick, StateGame>();

    app.Run();

    delete blockTexture;
}
}
