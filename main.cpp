// SMFL INCLUDES
#include <SFML/Graphics.hpp>
#include <SFML/Window/Mouse.hpp>

// MATHS INCLUDE
#include <iostream>
#include <vector>
#include <array>
#include <cmath>

// TYPES INCLUDE
#include <string>

// CONSTANTS DECLARATION
#define HEIGHT 720
#define WIDTH 1280

// FAKE GAME DESCRIPTION (THOSE SHOULD BE THE DATA GIVEN TO THE MAIN FUNCTION OF MY LIBRARY)

std::array<std::array<int, 8>, 8> chessboard = 
    {{{1, 2, 3, 4, 5, 3, 2, 1},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {12, 12, 12, 12, 12, 12, 12, 12},
    {12, 12, 12, 12, 12, 12, 12, 12},
    {12, 12, 12, 12, 12, 12, 12, 12},
    {12, 12, 12, 12, 12, 12, 12, 12},
    {6, 6, 6, 6, 6, 6, 6, 6},
    {7, 8, 9, 10, 11, 9, 8, 7}}};

/*  
    0: black pawn 1: black rook 2: black knight 3: black bishop 4: black queen 5: black king
    6: white pawn 7: white rook 8: white knight 9: white bishop 10: white queen 11: white king 
    12: empty
*/

std::array<int, 6> eated_black = {8, 2, 2, 2, 1, 1};
std::array<int, 6> eated_white = {1, 1, 1, 1, 1, 1};

int secLeftBlack = 60;
int secLeftWhite = 121;

bool gameState = true;

// END OF FAKE GAME DESCRIPTION

void printArrayFour(std::array<int, 4> ar){
    std::cout << static_cast<int>(ar[0]) << " " << static_cast<int>(ar[1]) << " " << static_cast<int>(ar[2]) << " " << static_cast<int>(ar[3]) << std::endl;
}

bool twoIntsArrayCheck(std::array<int, 2> ar1, std::array<int, 2> ar2){ return ((ar1[0]==ar2[0])&&(ar1[1]==ar2[1]));}

bool isHorizontal(int height, int width)
{
    return (width > height);
}

std::array<int, 8> squareInScreen(int height, int width)
{
    int squareSide = std::min(height, width);
    auto dv = std::div(squareSide, 8);
    std::array<int, 8> result;
    for (int i = 0; i < 8; ++i) {
        result[i] = (i < dv.rem) ? (dv.quot + 1) : dv.quot;
    }
    return result;
} // CORRECT

std::array<int, 4> leftSpace(int height, int width)
{
    std::array<int, 4> result; // Create an array of size 4 composed of (leftSpace top left height, leftSpace top left width, leftSpace size height, leftSpace size width)
    int squareSide = std::min(height, width);
    if (isHorizontal(height, width)){ // if horizontal window free area for gui is at the right of the board
        result[0] = 0;
        result[1] = squareSide;
        result[2] = height;
        result[3] = width - squareSide;
    }
    else{ // if vertical window free area for gui is under the board
        result[0] = squareSide;
        result[1] = 0;
        result[2] = height - squareSide;
        result[3] = width;
    }
    return result;
} // CORRECT


std::array<int, 4> centerPlot(std::array<int, 4> windowProperties, int imageHeight, int imageWidth, float borderInPercent){ // windowProperties = (top left height, top left width, height, width)
    // returns also (top left height, top left width, height, width)
    // i added the possibility to leave a free space around the sprite, we express it in percent of the dimensions, so for border = 5%, we now that image cant occupy more than 90% of the height and 90% of the width
    float borderMultiplier = 1.0 - 2.0 * std::min(std::max(borderInPercent, static_cast<float>(0.0)), static_cast<float>(50.0)) / 100; // we ensure border to be positive so image is not bigger than the window, and smaller than 50% so there is still space for image

    float alpha = std::min(borderMultiplier * static_cast<float>(windowProperties[2]) / imageHeight, borderMultiplier * static_cast<float>(windowProperties[3]) / imageWidth );
    std::array<int, 4> result = {0, 0, imageHeight * alpha, imageWidth * alpha};
    result[0] = windowProperties[0] + (windowProperties[2] - result[2])/2; // top left height
    result[1] = windowProperties[1] + (windowProperties[3] - result[3])/2; // top left width
    return result;
} // CORRECT 

int computeEatedWidth(std::array<int, 6> eated, int pieceDim){
    // we plot each eated piece, if they are of the same type we overlap them by 75% this explains the result : sum (i s.t. eated[i]>0) ((eated[i]-1) * pieceDim/4 + pieceDim) 
    // can be simplified into sum (i s.t. eated[i]>0) pieceDim * (1 + (eated[i]-1) / 4 ) 
    float result = 0.0;
    for (int i = 0; i < 6; ++i){
        if (eated[i]>0){
            result += (1 + (static_cast<float>(eated[i]) - 1) / 4);
        }
    }
    return pieceDim * result;
} // CORRECT

bool checkMouseInBounds(sf::Vector2i mousePosition, std::array<int, 4> detectionAreaProperties){
    // mousePosition has x and y attributes and detectionAreaProperties is like (top left height, top left width, height, width)
    return (mousePosition.y >= detectionAreaProperties[0] &&
                mousePosition.y < detectionAreaProperties[0] + detectionAreaProperties[2] &&
                mousePosition.x >= detectionAreaProperties[1] &&
                mousePosition.x < detectionAreaProperties[1] + detectionAreaProperties[3]);
}

int plotEated(sf::RenderWindow& window, std::array<int, 6> eated, std::array<int, 4> eatedAreaProperties, bool white, int pieceDim, std::array<sf::Texture, 12> chessTextures){    
    // we have access to std::array<sf::Texture, 12> chessTextures and we need to plot all the eated pieces using a loop
    // indices for black are 0,1,2,3,4,5 and for white pieces 6,7,8,9,10,11 that is why we define an offset of 6 so we can automatically pick the wong color
    int colorOffset = white * 6;
    int currentPieceTopLeftCornerXpos = eatedAreaProperties[1] + eatedAreaProperties[3] - eatedAreaProperties[2]; // Xpos (top left corner x coord) + W (area for plot width) - H (area for plot height <-> piece height)
    for (int i = 5; i > -1; --i) { // start at the last piece so the pieces look stacked up like in chess.com
        sf::Sprite currentPieceSprite(chessTextures[i+colorOffset]); 
        for (int j = 1; j <= eated[i]; ++j) { // we repeat plot 
            currentPieceSprite.setPosition(sf::Vector2f(static_cast<float>(currentPieceTopLeftCornerXpos), static_cast<float>(eatedAreaProperties[0])));
            float scale = static_cast<float>(eatedAreaProperties[2]) / static_cast<float>(pieceDim);
            currentPieceSprite.setScale(sf::Vector2f(scale, scale));
            window.draw(currentPieceSprite);
            if (j < eated[i]) { // shift by 25% only if next piece is the same type
                currentPieceTopLeftCornerXpos -= eatedAreaProperties[2] / 4; // we overlap same pieces by 25%
            }
        }
        if (eated[i]>0) { // shift only if we have this exact piece
            currentPieceTopLeftCornerXpos -= eatedAreaProperties[2]; // no overlap (side to side plot) for different pieces
        } 
    }
    return 0;
} // CORRECT

std::array<int, 4> secToDigits(int n){
    std::array<int, 4> result = {0, 0, 0, 0};
    int mins = n / 60;
    int secs = n % 60;
    result[0] = mins / 10;
    result[1] = mins % 10;
    result[2] = secs / 10;
    result[3] = secs % 10;
    return result;
}

int plotClock(sf::RenderWindow& window, int secLeft, std::array<int, 4> clockAreaProperties, int clockDim, std::array<sf::Texture, 12> clockTextures){
    int currentDigitTopLeftCornerXpos = clockAreaProperties[1];

    // Draw clock
    sf::Sprite clockSprite(clockTextures[10]);
    clockSprite.setPosition(sf::Vector2f(static_cast<float>(currentDigitTopLeftCornerXpos), static_cast<float>(clockAreaProperties[0]))); 
    clockSprite.setScale(sf::Vector2f(static_cast<float>(clockAreaProperties[2]) / clockDim, static_cast<float>(clockAreaProperties[2]) / clockDim));
    window.draw(clockSprite);

    currentDigitTopLeftCornerXpos += clockAreaProperties[2];

    // Draw digits
    std::array<int, 4> digitsArray = secToDigits(secLeft);
    for (int i=0; i<4; ++i){
        sf::Sprite digitSprite(clockTextures[digitsArray[i]]);
        digitSprite.setPosition(sf::Vector2f(static_cast<float>(currentDigitTopLeftCornerXpos), static_cast<float>(clockAreaProperties[0]))); 
        digitSprite.setScale(sf::Vector2f(static_cast<float>(clockAreaProperties[2]) / clockDim , static_cast<float>(clockAreaProperties[2]) / clockDim));
        window.draw(digitSprite);
        currentDigitTopLeftCornerXpos += clockAreaProperties[2] * 5 / 8;
        if (i==1){
            sf::Sprite doublePointsSprite(clockTextures[11]);
            doublePointsSprite.setPosition(sf::Vector2f(static_cast<float>(currentDigitTopLeftCornerXpos), static_cast<float>(clockAreaProperties[0]))); 
            doublePointsSprite.setScale(sf::Vector2f(static_cast<float>(clockAreaProperties[2]) / clockDim , static_cast<float>(clockAreaProperties[2]) / clockDim));
            window.draw(doublePointsSprite);
            currentDigitTopLeftCornerXpos += clockAreaProperties[2] * 3 / 8;
        }
    }
    return 0;
}

std::string plotTimerButtonsLost(sf::RenderWindow& window, int height, int width, // Window params
    std::array<int, 6> eated_black, std::array<int, 6> eated_white, int secLeftBlack, int secLeftWhite, // Display params
    bool* gameStatePtr, std::array<sf::Texture, 12> chessTextures, std::array<sf::Texture, 4> buttonTextures, std::array<sf::Texture, 12> clockTextures, // Textures params
    bool mouseLeftClicked, sf::Vector2i mousePosition) // Mouse params
{
    // We are going to divide the space according to the shape of it, 
    // if windows is horizontal, we divide into 7 almost equivalent horizontal pieces (white lost pieces, black timer, button 1, button 2, button 3, white timer, black lost pieces)
    // if windows is vertical,  we divide into 3 almost equivalent horizontal pieces ((white lost pieces, black timer), (button 1, button 2, button 3), (white timer, black lost pieces))
    // gameState is True iff not paused

    std::string uciRequest = ""; // by default if nothing is clicked return empty string

    std::array<int, 4> leftSpaceVar = leftSpace(height, width); // blank space : (top left height, top left width, height, width)
    // first we define all the sizes of the sprites in pixels so we can compute all the scaling factors later
    // 1ST ROW:
    int eatedWhiteImageHeight = 16;
    int eatedWhiteImageMaximalWidth = 8.5 * eatedWhiteImageHeight;
    int eatedWhiteImageWidth = computeEatedWidth(eated_white, eatedWhiteImageHeight);

    // 2ND AND 6TH ROW:
    int timerHeight = 8;
    int timerWidth = 31; // 8(1 clock) + 3(1 double point) + 4*5(4 digits)

    // 3RD, 4TH, 5TH ROW:
    int buttonImageHeight = 16;
    int buttonImageWidth = 64;

    // 7TH ROW:
    int eatedBlackImageHeight = 16;
    int eatedBlackImageMaximalWidth = 8.5 * eatedBlackImageHeight;
    int eatedBlackImageWidth = computeEatedWidth(eated_black, eatedBlackImageHeight);
    if (isHorizontal(height, width)){
        int dividedHeight = leftSpaceVar[2]/7;
        // 1ST ROW: Eated White Pieces
        std::array<int, 4> whitePiecesAreaProperties = {0, leftSpaceVar[1], dividedHeight, leftSpaceVar[3]};
        // to compute the area we use the maximal width of eated pieces, so piece sizes are consistant during the game, but we plot at computed width so the pieces starts always at the same place
        std::array<int, 4> centeredWhitePiecesAreaProperties = centerPlot(whitePiecesAreaProperties, eatedWhiteImageHeight, eatedWhiteImageMaximalWidth, 0);
        centeredWhitePiecesAreaProperties[3] =
            static_cast<int>(
                static_cast<float>(eatedWhiteImageWidth)
                * centeredWhitePiecesAreaProperties[2]
                / eatedWhiteImageHeight
            ); // plot with compute width and not theoritical maximum width
        int plotWhiteEatedWorked = plotEated(window, eated_white, centeredWhitePiecesAreaProperties, true, eatedWhiteImageHeight, chessTextures);

        // 2ND ROW: Black Timer
        std::array<int, 4> blackTimerAreaProperties = {dividedHeight, leftSpaceVar[1], dividedHeight, leftSpaceVar[3]};
        std::array<int, 4> centeredBlackTimerAreaProperties = centerPlot(blackTimerAreaProperties, timerHeight, timerWidth, 2);
        int plotTimerBlackWorked = plotClock(window, secLeftBlack, centeredBlackTimerAreaProperties, timerHeight, clockTextures);

        // 3RD ROW: Pause Button
        std::array<int, 4> pauseAreaProperties = {2 * dividedHeight, leftSpaceVar[1], dividedHeight, leftSpaceVar[3]};
        std::array<int, 4> centeredPlayPauseButtonProperties = centerPlot(pauseAreaProperties, buttonImageHeight, buttonImageWidth,5);
        sf::Sprite playPauseSprite(buttonTextures[0]); // by default we load "pause" texture
        if (!(*gameStatePtr)) { // if game is not playing we load "play" texture
            playPauseSprite.setTexture(buttonTextures[1]);
        }
        playPauseSprite.setPosition(sf::Vector2f(static_cast<float>(centeredPlayPauseButtonProperties[1]), static_cast<float>(centeredPlayPauseButtonProperties[0]))); 
        playPauseSprite.setScale(sf::Vector2f(static_cast<float>(centeredPlayPauseButtonProperties[3])/static_cast<float>(buttonImageWidth), static_cast<float>(centeredPlayPauseButtonProperties[2])/static_cast<float>(buttonImageHeight)));
        
        if (checkMouseInBounds(mousePosition, centeredPlayPauseButtonProperties))
        {
            playPauseSprite.setPosition(sf::Vector2f(static_cast<float>(centeredPlayPauseButtonProperties[1]) - 0.05 * centeredPlayPauseButtonProperties[3], static_cast<float>(centeredPlayPauseButtonProperties[0]) - 0.05 * centeredPlayPauseButtonProperties[2])); 
            playPauseSprite.setScale(sf::Vector2f(1.1*static_cast<float>(centeredPlayPauseButtonProperties[3])/static_cast<float>(buttonImageWidth), 1.1*static_cast<float>(centeredPlayPauseButtonProperties[2])/static_cast<float>(buttonImageHeight)));
            // button have 5% margin all around, so if we scale it by 1.1 there will be no leak in the surrounding area, 0.9 * 1.1 = 0.99 < 1
            if (mouseLeftClicked){
                uciRequest = "";
                *gameStatePtr = !(*gameStatePtr);
            }
        }
        window.draw(playPauseSprite);

        // 4TH ROW: Screenshot Button
        std::array<int, 4> snapAreaProperties = {3 * dividedHeight, leftSpaceVar[1], dividedHeight, leftSpaceVar[3]};
        std::array<int, 4> centeredSnapButtonProperties = centerPlot(snapAreaProperties, buttonImageHeight, buttonImageWidth,5);
        sf::Sprite snapSprite(buttonTextures[2]);
        snapSprite.setPosition(sf::Vector2f(static_cast<float>(centeredSnapButtonProperties[1]), static_cast<float>(centeredSnapButtonProperties[0]))); 
        snapSprite.setScale(sf::Vector2f(static_cast<float>(centeredSnapButtonProperties[3])/static_cast<float>(buttonImageWidth), static_cast<float>(centeredSnapButtonProperties[2])/static_cast<float>(buttonImageHeight)));
        
        if (checkMouseInBounds(mousePosition, centeredSnapButtonProperties))
        {
            snapSprite.setPosition(sf::Vector2f(static_cast<float>(centeredSnapButtonProperties[1]) - 0.05 * centeredSnapButtonProperties[3], static_cast<float>(centeredSnapButtonProperties[0]) - 0.05 * centeredSnapButtonProperties[2])); 
            snapSprite.setScale(sf::Vector2f(1.1*static_cast<float>(centeredSnapButtonProperties[3])/static_cast<float>(buttonImageWidth), 1.1*static_cast<float>(centeredSnapButtonProperties[2])/static_cast<float>(buttonImageHeight)));
            if (mouseLeftClicked){
                sf::Texture screenshtotTexture(window.getSize());
            screenshtotTexture.update(window);

            sf::Image screenshotImage = screenshtotTexture.copyToImage();   
            screenshotImage.saveToFile("./ScreenShots/screenshot.jpg");
            }
        }
        window.draw(snapSprite);

        // 5TH ROW: Reset Button
        std::array<int, 4> resetAreaProperties = {4 * dividedHeight, leftSpaceVar[1], dividedHeight, leftSpaceVar[3]};
        std::array<int, 4> centeredResetButtonProperties = centerPlot(resetAreaProperties, buttonImageHeight, buttonImageWidth, 5);
        sf::Sprite resetSprite(buttonTextures[3]);
        resetSprite.setPosition(sf::Vector2f(static_cast<float>(centeredResetButtonProperties[1]), static_cast<float>(centeredResetButtonProperties[0]))); 
        resetSprite.setScale(sf::Vector2f(static_cast<float>(centeredResetButtonProperties[3])/static_cast<float>(buttonImageWidth), static_cast<float>(centeredResetButtonProperties[2])/static_cast<float>(buttonImageHeight)));
        
        if (checkMouseInBounds(mousePosition, centeredResetButtonProperties) )
        {
            resetSprite.setPosition(sf::Vector2f(static_cast<float>(centeredResetButtonProperties[1]) - 0.05 * centeredResetButtonProperties[3], static_cast<float>(centeredResetButtonProperties[0]) - 0.05 * centeredResetButtonProperties[2])); 
            resetSprite.setScale(sf::Vector2f(1.1*static_cast<float>(centeredResetButtonProperties[3])/static_cast<float>(buttonImageWidth), 1.1*static_cast<float>(centeredResetButtonProperties[2])/static_cast<float>(buttonImageHeight)));
            if (mouseLeftClicked) {
                uciRequest = "ucinewgame";
            }
        }
        window.draw(resetSprite);

        // 6TH ROW: White Timer
        std::array<int, 4> whiteTimerAreaProperties = {5 * dividedHeight, leftSpaceVar[1], dividedHeight, leftSpaceVar[3]};
        std::array<int, 4> centeredWhiteTimerAreaProperties = centerPlot(whiteTimerAreaProperties, timerHeight, timerWidth, 2);
        int plotTimerWhiteWorked = plotClock(window, secLeftWhite, centeredWhiteTimerAreaProperties, timerHeight, clockTextures);

        // 7TH ROW: Eated Black Pieces
        std::array<int, 4> blackPiecesAreaProperties = {6 * dividedHeight, leftSpaceVar[1], dividedHeight, leftSpaceVar[3]};
        std::array<int, 4> centeredBlackPiecesAreaProperties = centerPlot(blackPiecesAreaProperties, eatedBlackImageHeight, eatedBlackImageMaximalWidth, 0);
        centeredBlackPiecesAreaProperties[3] =
            static_cast<int>(
                static_cast<float>(eatedBlackImageWidth)
                * centeredBlackPiecesAreaProperties[2]
                / eatedBlackImageHeight
            ); // plot with compute width and not theoritical maximum width
        int plotBlackEatedWorked = plotEated(window, eated_black, centeredBlackPiecesAreaProperties, false, eatedBlackImageHeight, chessTextures);
    }
    return uciRequest;
}

std::array<int, 2> plotChessBoard(sf::RenderWindow& window, int width, int height, // Window params
    std::array<std::array<int, 8>, 8> chessboard, std::array<int, 2> selectedSquare, int elapsedSinceLaunchMilli, // Chess board param
    std::array<sf::Color, 2> colors, int pieceDim, std::array<sf::Texture, 12> chessTextures, std::array<sf::Texture, 2> selectedTexture,// textures params
    bool mouseLeftClicked, sf::Vector2i mousePosition) // Mouse params
{
    std::array<int, 2> mouseInGridLoc = {-1,-1}; // this variable is the returned one, it contains the coordinates (x,y) in the board €[0, 7]²u{-1}², if mouse is outside board we return {-1, -1}
    std::array<int, 8> casesSides = squareInScreen(height, width); // biggest square dimensions we can plot in the window
    int squareSide = std::min(height, width);
    // the two next variables define the location of the top left corner of the current chess board case
    int currentIpos = 0;
    int currentJpos = 0;

    bool mouseInCurrentSquare = false;
    for (int i = 0; i < 8; ++i) {
        int currentIlength = casesSides[i];
        currentJpos = 0;
        for (int j = 0; j < 8; ++j) {
            mouseInCurrentSquare = false;
            int currentJlength = casesSides[j];
            // CHECK IF MOUSE IS IN THE CURRENT SQUARE (if so there will be change in square display)
            std::array<int, 4> currentSquareAreaProperties = {currentIpos, currentJpos, currentIlength, currentJlength};
            if (checkMouseInBounds(mousePosition, currentSquareAreaProperties))
            {
                mouseInGridLoc = {i, j};
                mouseInCurrentSquare = true;
            }

            // PRINT THE BACKGROUND
            // Create the rectangle shape (Width <-> j, Height <-> i)
            sf::RectangleShape caseRect(sf::Vector2f(static_cast<float>(currentJlength), static_cast<float>(currentIlength)));
            int colorIdx = (i+j) % 2; // 0: white case, 1: black case
            
            // Customize the rectangle
            caseRect.setFillColor(colors[colorIdx]); // case color
            caseRect.setPosition(sf::Vector2f(static_cast<float>(currentJpos), static_cast<float>(currentIpos)));  

            // Draw the rectangle
            window.draw(caseRect);

            // PRINT THE CHESS PIECE
            if (chessboard[i][j] != 12){ // if case is not empty
                sf::Sprite pieceSprite(chessTextures[chessboard[i][j]]);

                if (mouseInCurrentSquare){ // scale up the piece if hovered
                    pieceSprite.setPosition(sf::Vector2f(static_cast<float>(currentJpos) - 0.05 * currentJlength , static_cast<float>(currentIpos) - 0.05 * currentIlength));
                    // this strange 0.05 is simply (1.1-1)/2, we ensure to recenter piece in the square when scaling it up, if not done the scaling will be in the right and bottom directions
                    pieceSprite.setScale(sf::Vector2f(1.1*currentJlength/static_cast<float>(pieceDim), 1.1*currentIlength/static_cast<float>(pieceDim)));
                    // pieces are 14x14 pixels maximum in a 16x16 texture image, so if we scale it by 1.1 there will be no leak in the surrounding squares, 1.1*14/16 < 1
                }
                else{
                    pieceSprite.setPosition(sf::Vector2f(static_cast<float>(currentJpos), static_cast<float>(currentIpos)));
                    pieceSprite.setScale(sf::Vector2f(currentJlength/static_cast<float>(pieceDim), currentIlength/static_cast<float>(pieceDim)));
                }
                window.draw(pieceSprite);
            }

            // IF CASE IS SELECTED DISPLAY "SELECTED" ANIMATION
            if ((!twoIntsArrayCheck(selectedSquare, std::array<int, 2> {-1,-1}) ) && (selectedSquare[0]==i) && (selectedSquare[1]==j) && ( (elapsedSinceLaunchMilli%1000) < 500)){ // if a square is selected (last condition is for flickering)
                sf::Sprite selectedSprite(selectedTexture[colorIdx]);
                selectedSprite.setPosition(sf::Vector2f(static_cast<float>(currentJpos), static_cast<float>(currentIpos)));
                selectedSprite.setScale(sf::Vector2f(currentJlength/static_cast<float>(pieceDim), currentIlength/static_cast<float>(pieceDim)));
                window.draw(selectedSprite);
            }

            currentJpos += currentJlength;
        }
        currentIpos += currentIlength;
    }
    return mouseInGridLoc; 
}

int main()
{
    //Loading all the textures : 
    std::array<sf::Texture, 12> chessTextures;
    chessTextures[0].loadFromFile("./Assets/Pieces/Black/Pawn.png");
    chessTextures[1].loadFromFile("./Assets/Pieces/Black/Rook.png");
    chessTextures[2].loadFromFile("./Assets/Pieces/Black/Knight.png");
    chessTextures[3].loadFromFile("./Assets/Pieces/Black/Bishop.png");
    chessTextures[4].loadFromFile("./Assets/Pieces/Black/Queen.png");
    chessTextures[5].loadFromFile("./Assets/Pieces/Black/King.png");
    chessTextures[6].loadFromFile("./Assets/Pieces/White/Pawn.png");
    chessTextures[7].loadFromFile("./Assets/Pieces/White/Rook.png");
    chessTextures[8].loadFromFile("./Assets/Pieces/White/Knight.png");
    chessTextures[9].loadFromFile("./Assets/Pieces/White/Bishop.png");
    chessTextures[10].loadFromFile("./Assets/Pieces/White/Queen.png");
    chessTextures[11].loadFromFile("./Assets/Pieces/White/King.png");

    std::array<sf::Texture, 4> buttonTextures;
    buttonTextures[0].loadFromFile("./Assets/Buttons/Pause.png");
    buttonTextures[1].loadFromFile("./Assets/Buttons/Play.png");
    buttonTextures[2].loadFromFile("./Assets/Buttons/Snap.png");
    buttonTextures[3].loadFromFile("./Assets/Buttons/Reset.png");

    std::array<sf::Texture, 12> clockTextures;
    clockTextures[0].loadFromFile("./Assets/Clock/Zero.png");
    clockTextures[1].loadFromFile("./Assets/Clock/One.png");
    clockTextures[2].loadFromFile("./Assets/Clock/Two.png");
    clockTextures[3].loadFromFile("./Assets/Clock/Three.png");
    clockTextures[4].loadFromFile("./Assets/Clock/Four.png");
    clockTextures[5].loadFromFile("./Assets/Clock/Five.png");
    clockTextures[6].loadFromFile("./Assets/Clock/Six.png");
    clockTextures[7].loadFromFile("./Assets/Clock/Seven.png");
    clockTextures[8].loadFromFile("./Assets/Clock/Eight.png");
    clockTextures[9].loadFromFile("./Assets/Clock/Nine.png");
    clockTextures[10].loadFromFile("./Assets/Clock/Clock.png");
    clockTextures[11].loadFromFile("./Assets/Clock/DoublePoints.png");

    std::array<sf::Texture, 2> selectedTexture;
    selectedTexture[0].loadFromFile("./Assets/Pieces/SelectedWhiteSquare.png");
    selectedTexture[1].loadFromFile("./Assets/Pieces/SelectedBlackSquare.png");

    std::array<sf::Color, 2> colors = {sf::Color{0xEBECD0FF}, sf::Color{0x779556FF}}; // white and black chess cases

    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Chess Board");
    window.clear(sf::Color(0xEBECD0FF));

    // Variables init
    bool mouseLeftPressed = false;
    bool mouseLeftPressedPrevious = false;
    bool mouseLeftClicked = false;
    sf::Clock clock; // we start a timer for flickering textures

    std::array<int, 2> selectedSquare = {-1,-1};

    std::string uciRequest = "";
    std::string uciRequestButtons = "";
    std::string uciRequestBoard = "";

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }
        // [0] RESET WINDOWS
        window.clear(sf::Color(0xEBECD0FF));

        // [1] GETTING THE USER INPUT
        mouseLeftPressedPrevious = mouseLeftPressed;
        mouseLeftPressed = (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)); // mouse is pressed means the button is currently pressed
        mouseLeftClicked = (mouseLeftPressed && !mouseLeftPressedPrevious); // mouse is clicked if it is the first frame from all the consecutives pressed frame in other words we look for rising edge, we want to trigger button interaction only once
        // raising edge detection is max(0, f'(x)) and falling edge max(0, -f'(x)) but here we express it with boolean and a memory
        sf::Vector2i mousePosition = sf::Mouse::getPosition(window); // get in screen mouse position

        // // [2] DISPLAY THE GRID
        sf::Time elapsedSinceLaunch = clock.getElapsedTime(); // we compute elapsed time and pass it for chessboard display
        int elapsedSinceLaunchMilli = 2 * elapsedSinceLaunch.asMilliseconds(); // we can multiply to make it flicker faster, if no multiplier -> flicker every sec, if multiplied by 3 -> flicker 3 times per sec
        std::array<int, 2> mouseInGridLoc = plotChessBoard(window, WIDTH, HEIGHT, chessboard, selectedSquare, elapsedSinceLaunchMilli, colors , 16, chessTextures, selectedTexture, mouseLeftClicked, mousePosition);
        // mouseInGridLoc is (x,y) y positive is toward bottom
        // now we will use this to update game, if mouse pressed and cliked square is different of the selected one we set a uci request for the movement else if it is the same, we unselect the square, else if clicked on {-1,-1} do nothing
        uciRequestBoard = "";
        if (!twoIntsArrayCheck(mouseInGridLoc, std::array<int, 2> {-1,-1}) && mouseLeftClicked){ 
            if (twoIntsArrayCheck(selectedSquare, std::array<int, 2> {-1,-1})){ // if nothing selected :
                if (chessboard[mouseInGridLoc[0]][mouseInGridLoc[1]] != 12){ // and if square contains a piece :
                    selectedSquare = mouseInGridLoc; 
                    sf::Time voidVariable = clock.restart(); // we reset clock so the flickering is don't start at random time
                }
            }
            else{ // else something is selected and :
                if (twoIntsArrayCheck(selectedSquare, mouseInGridLoc)) {
                    selectedSquare = {-1, -1};
                }
                else {
                    uciRequestBoard = std::to_string(selectedSquare[0]) + "_" + std::to_string(selectedSquare[1]) + "_to_" + std::to_string(mouseInGridLoc[0]) + "_" + std::to_string(mouseInGridLoc[1]);
                    selectedSquare = {-1, -1}; // !!!TODO!!! HERE WE WOULD NEED TO CHECK IF THE MOVE IS LEGIT TO RESET SELECTION (we can do the reset outside this if-else, for example if the chess engine returns that the move was played??)
                }
            }
        }

        // [3] DISPLAY THE SIDE-GUI (eated pieces, button and clock)
        uciRequestButtons = plotTimerButtonsLost(window, HEIGHT, WIDTH, eated_black, eated_white, secLeftBlack, secLeftWhite, &gameState, chessTextures, buttonTextures, clockTextures, mouseLeftClicked, mousePosition); // for the moment no clock displayed secLeftBlack and secLeftWhite are thus useless
        
        // [4] UPDATE SCREEN
        window.display();
        // std::cout << "left click : " << mouseLeftClicked << " and mouse global position : " << mousePosition.x << ", " <<mousePosition.y << " and mouse in grid location :" << mouseInGridLoc[0] << ", " << mouseInGridLoc[1]<< std::endl;
        std::cout << uciRequest << std::endl;

        // [5] CALL THE CHESS ENGINE IF THERE IS AN USER INPUT
        if (uciRequestBoard != ""){
            uciRequest = uciRequestBoard;
        }
        else{
            uciRequest = uciRequestButtons;
        }

        // Here : Have Fun with the Lib
    
    }
}

