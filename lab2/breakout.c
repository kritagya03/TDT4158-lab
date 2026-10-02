/***************************************************************************************************
 * DON'T REMOVE THE VARIABLES BELOW THIS COMMENT                                                   *
 **************************************************************************************************/
unsigned long long __attribute__((used)) VGAaddress = 0xc8000000; // Memory storing pixels
unsigned int __attribute__((used)) red = 0x0000F0F0;
unsigned int __attribute__((used)) green = 0x00000F0F;
unsigned int __attribute__((used)) blue = 0x000000FF;
unsigned int __attribute__((used)) white = 0x0000FFFF;
unsigned int __attribute__((used)) black = 0x0;

// Don't change the name of this variables
#define NCOLS 18// <- Supported value range: [1,18]
#define NROWS 16  // <- This variable might change.
#define TILE_SIZE 15 // <- Tile size, might change.

char *won = "You Won";       // DON'T TOUCH THIS - keep the string as is
char *lost = "You Lost";     // DON'T TOUCH THIS - keep the string as is
unsigned short height = 240; // DON'T TOUCH THIS - keep the value as is
unsigned short width = 320;  // DON'T TOUCH THIS - keep the value as is
char font8x8[128][8];        // DON'T TOUCH THIS - this is a forward declaration
unsigned char tiles[NROWS][NCOLS] __attribute__((used)) = { 0 }; // DON'T TOUCH THIS - this is the tile map
/**************************************************************************************************/

//************ BAR VARIABLES

#define BAR_START_Y 105

typedef struct _bar
{
    int pos_y;
    unsigned int width;
    unsigned int region_height;
} Bar;

Bar bar = {
    .pos_y = BAR_START_Y,
    .width = 7,
    .region_height = 15
};

#define BAR_HEIGHT (bar.region_height * 3)

//************ BALL VARIABLES

#define BALL_START_X 15
#define BALL_START_Y 122
#define BALL_START_DX 1
#define BALL_START_DY 1

typedef struct _ball
{
    int pos_x;
    int pos_y;
    int speed_dx;
    int speed_dy;
    int radius;
} Ball;

Ball ball = {
    .pos_x = BALL_START_X,
    .pos_y = BALL_START_Y,
    .speed_dx = BALL_START_DX,
    .speed_dy = BALL_START_DY,
    .radius = 3
};

#define BALL_LEFT   (ball.pos_x - ball.radius)
#define BALL_RIGHT  (ball.pos_x + ball.radius)
#define BALL_TOP    (ball.pos_y - ball.radius)
#define BALL_BOTTOM (ball.pos_y + ball.radius)

//************ GAME STATE
typedef enum _gameState
{
    Stopped = 0,
    Running = 1,
    Won = 2,
    Lost = 3,
    Exit = 4,
} GameState;
GameState currentState = Stopped;

//************ Variables to keep track of for statistics
unsigned int blocks_removed = 0;
unsigned int paddle_moves = 0;

//************ C declarations for assembly functions
void SetPixel(unsigned int x_coord, unsigned int y_coord, unsigned int color);
void ClearScreen();
int ReadUart();
void WriteUart(char c);

//************ Assembly functions implementations
asm("ClearScreen: \n\t"
    "    PUSH {LR} \n\t"
    "    PUSH {R4, R5, R6, R7} \n\t"
	
    "    LDR R6, =height \n\t"
    "    LDRH R6, [R6] \n\t" //R6 = height of VGA screen

    "    LDR R7, =width \n\t"
    "    LDRH R7, [R7] \n\t" // R7 = width of VGA screen

	"    MOV R4, #0 \n\t" //R4 = y pixel position
	
    "clear_y_loop: \n\t"
    "    MOV R5, #0 \n\t" //R5 = x pixel position

    "clear_x_loop: \n\t"
    "    MOV R0, R5 \n\t" //R0 = x
    "    MOV R1, R4 \n\t" //R1 = y
    "    LDR R2, =0xFFFF \n\t" //R2 = white
    "    BL SetPixel \n\t" //SetPixel sets the pixel in that position to white

    "    ADD R5, R5, #1 \n\t"
    "    CMP R5, R7 \n\t"
    "    BLT clear_x_loop \n\t" //Continue clearing the row if row not finished

    "    ADD R4, R4, #1 \n\t"
    "    CMP R4, R6 \n\t"
    "    BLT clear_y_loop \n\t" //Continue to next row until all columns finished
	
    "    POP {R4,R5,R6,R7}\n\t"
    "    POP {LR} \n\t"
    "    BX LR");

//Assumes R0 = x pixel position, R1 = y pixel position, R2 = color value
asm("SetPixel: \n\t"
    "LDR R3, =VGAaddress \n\t"
    "LDR R3, [R3] \n\t"
    "LSL R1, R1, #10 \n\t"
    "LSL R0, R0, #1 \n\t"
    "ADD R1, R0 \n\t"
    "STRH R2, [R3,R1] \n\t"
    "BX LR");

asm("ReadUart:\n\t"
    "LDR R1, =0xFF201000 \n\t"
    "LDR R0, [R1]\n\t"
    "BX LR");

asm("WriteUart:\n\t"
    "LDR R1, =0xFF201000 \n\t"
    "STRB R0, [R1]\n\t"
    "BX LR");

//************ C function implementations below
//************ Draw on VGA screen implementations
void draw_block(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int color)
{
	for (unsigned int row = 0; row < height; row++) {
        for (unsigned int col = 0; col < width; col++) {
            SetPixel(x + col, y + row, color);
        }
    }
}

void draw_bar(unsigned int y)
{
	draw_block(0, y, bar.width, bar.region_height, red);
    draw_block(0, y + bar.region_height, bar.width, bar.region_height, black);
    draw_block(0, y + 2 * bar.region_height, bar.width, bar.region_height, red);
}

void draw_ball_color(unsigned int color)
{
    for (int row = -ball.radius; row <= ball.radius; row++) {
        int absolute_row;

        if (row < 0) {
            absolute_row = -row;
        } else {
            absolute_row = row;
        }

        int extent = ball.radius - absolute_row;
        for (int col = -extent; col <= extent; col++) {
            SetPixel(ball.pos_x + col, ball.pos_y + row, color);
        }
    }
}

void draw_ball()
{
    draw_ball_color(black);
}

void draw_playing_field()
{	
    for (int row = 0; row < NROWS; row++){
        for (int col = 0; col < NCOLS; col++){
			
			if (tiles[row][col]==1){
				continue;
			}
			
            int x = width - TILE_SIZE - col * TILE_SIZE; //Start from the back of right of the VGA screen
            int y = row * TILE_SIZE;
			int color;

			if ((row + col) % 3 == 0) {
                color = red;
            }
            else if ((row + col) % 3 == 1) {
                color = green;
            }
            else {
                color = blue;
            }
			
			draw_block(x, y, TILE_SIZE, TILE_SIZE, color);
        }
    }
}

//************ C function implementations below
//************ Game state implementations

//Helper function for update_game_state()
int hit_block(int pos_x, int pos_y)
{
    int left_edge = width - TILE_SIZE - (NCOLS - 1) * TILE_SIZE;

    if (pos_x < left_edge || pos_x >= width || pos_y < 0 || pos_y >= NROWS * TILE_SIZE) {
        return 0;
    }

    //Find the corresponding block row and column for the position of the ball  
    int row = pos_y / TILE_SIZE;
    int col = (width - 1 - pos_x) / TILE_SIZE;

    if (tiles[row][col] == 1) {
        return 0;
    }
    tiles[row][col] = 1;

    //Erase old block
    int block_x = width - TILE_SIZE - col * TILE_SIZE;
    int block_y = row * TILE_SIZE;
    draw_block(block_x, block_y, TILE_SIZE, TILE_SIZE, white);
    blocks_removed++;

    return 1;
}

void update_game_state()
{
    if (currentState != Running){
        return;
    }

    //Collision with wall
	if (BALL_TOP <= 0 || BALL_BOTTOM >= height - 1){
		ball.speed_dy = -ball.speed_dy;
	}
	
    //Collision with paddle
    if (ball.speed_dx < 0) {
        if (BALL_LEFT <= bar.width && ball.pos_y >= bar.pos_y && ball.pos_y < bar.pos_y + BAR_HEIGHT){

            if (ball.pos_y < bar.pos_y + bar.region_height) {
                ball.speed_dx = -ball.speed_dx;
                ball.speed_dy = -BALL_START_DY;
            }
            else if (ball.pos_y < bar.pos_y + bar.region_height * 2) {
                ball.speed_dx = -ball.speed_dx;
                ball.speed_dy = 0;
            }
            else {
                ball.speed_dx = -ball.speed_dx;
                ball.speed_dy = BALL_START_DY;
            }
        }
    }
	
    //Erase old ball and draw new ball position
	draw_ball_color(white);
    ball.pos_x += ball.speed_dx;
    ball.pos_y += ball.speed_dy;
	draw_ball();

    //Hit check with block, left and right side of ball
    if (hit_block(BALL_LEFT, ball.pos_y) || hit_block(BALL_RIGHT, ball.pos_y)) {
        ball.speed_dx = -ball.speed_dx;
        draw_playing_field();
    }

    //Hit check with blocks, check top and bottom of ball
    if (hit_block(ball.pos_x, BALL_TOP) || hit_block(ball.pos_x, BALL_BOTTOM)) {
        ball.speed_dy = -ball.speed_dy;
        draw_playing_field();
    }

    if (BALL_LEFT < bar.width) { //LOOSING CONDITION
        currentState = Lost;
        return;
    }

    if (BALL_RIGHT >= width){ //WINNING CONDITION
        currentState = Won;
        return;
    }
}

//Format: 0x00 'Remaining Chars':2 'Ready 0x80':2 'Char 0xXX':2, sample: 0x00018077 
//(1 remaining character, buffer is ready, current character is 'w')
void update_bar_state()
{
    int out = ReadUart();
    if (!(out & 0x8000)) {
        return;
    }
    char c = out & 0xFF;

    if (c == '\n') { //if charecter is newline, exit game
        currentState = Exit;
        return;
    }

    int old_bar_y = bar.pos_y;
    if (c == 'w') {
        if (bar.pos_y >= 15) {
            bar.pos_y -= 15;
        }
    }
    else if (c == 's') {
        if (bar.pos_y + BAR_HEIGHT + 15 <= height) {
            bar.pos_y += 15;
        }
    }

    if (bar.pos_y != old_bar_y) {
        draw_block(0, old_bar_y, bar.width, BAR_HEIGHT, white);
        draw_bar(bar.pos_y);
        paddle_moves++;
    }
}

void write(char *str)
{
    while (*str != '\0') {
        WriteUart(*str);
        str++;
    }

    WriteUart('\n');
}

void play()
{		
    while (1)
    {
        update_game_state();
        update_bar_state();
        if (currentState != Running)
        {
            break;
        }

        //Control the speed of the game by changing number of itterations
        for (volatile int i = 0; i < 20000; i++) {}
    }
    if (currentState == Won)
    {
        write(won);

        char *win_print = "Paddle moves: ";
        char moves_str[11]; // 10 digits + null terminator
        sprintf(moves_str, "%u", paddle_moves);

        write(win_print);
        write(moves_str);
    }
    else if (currentState == Lost)
    {
        write(lost);

        char *lose_print = "Blocks removed: ";
        char blocks_str[11]; // 10 digits + null terminator
        sprintf(blocks_str, "%u", blocks_removed);

        write(lose_print);
        write(blocks_str);
    }
    else if (currentState == Exit)
    {
        return;
    }
    currentState = Stopped;
}

void reset()
{
    //Reset bar
    bar.pos_y = BAR_START_Y;

    //Reset ball
    ball.pos_x = BALL_START_X;
    ball.pos_y = BALL_START_Y;
    ball.speed_dx = BALL_START_DX;
    ball.speed_dy = BALL_START_DY;

    //Reset counters
    paddle_moves = 0;
    blocks_removed = 0;

    //Reset all blocks
    for (int row = 0; row < NROWS; row++) {
        for (int col = 0; col < NCOLS; col++) {
            tiles[row][col] = 0;
        }
    }

    //Initialization of the VGA screen
    ClearScreen();
    draw_ball();
	draw_bar(bar.pos_y);
    draw_playing_field();

    if (currentState != Exit) {
        write("Press 'w' or 's' to restart the game");
    }

    int remaining = 0;
    do
    {
        unsigned long long out = ReadUart();
        if (!(out & 0x8000))
        {
            // not valid - abort reading
            return;
        }
        remaining = (out & 0xFFFF0000) >> 16;
    } while (remaining > 0);
}

void wait_for_start()
{
    while(1){
        int out = ReadUart();
        if (!(out & 0x8000)) {
            continue;
        }

        char c = out & 0xFF;
        if (c == 'w' || c == 's') {
            currentState = Running;
            break;
        }

        if (c == '\n') {  //if charecter is newline, exit game
            currentState = Exit;
            return;
        }
    }
}

int main(int argc, char *argv[])
{
    //Initialization of the VGA screen
    ClearScreen();
    draw_ball();
	draw_bar(bar.pos_y);
    draw_playing_field();

    write("B! B! BREAKOUT!");
    write("'w' or 's' to start");
    if (NCOLS < 1 || NCOLS > 18) {
        write("ERROR! Invalid NCOLS");
        return 0;
    }

    while (1)
    {
        wait_for_start();
        play();
        reset();
        if (currentState == Exit)
        {
            break;
        }
    }
    return 0;
}

// THIS IS FOR THE OPTIONAL TASKS ONLY

// HINT: How to access the correct bitmask
// sample: to get character a's bitmask, use
// font8x8['a']
char font8x8[128][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0000 (nul)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0001
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0002
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0003
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0004
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0005
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0006
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0007
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0008
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0009
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000A
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000B
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000C
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000D
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000E
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+000F
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0010
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0011
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0012
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0013
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0014
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0015
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0016
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0017
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0018
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0019
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001A
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001B
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001C
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001D
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001E
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+001F
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0020 (space)
    {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00}, // U+0021 (!)
    {0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0022 (")
    {0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00}, // U+0023 (#)
    {0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00}, // U+0024 ($)
    {0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00}, // U+0025 (%)
    {0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00}, // U+0026 (&)
    {0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0027 (')
    {0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00}, // U+0028 (()
    {0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00}, // U+0029 ())
    {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00}, // U+002A (*)
    {0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00}, // U+002B (+)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06}, // U+002C (,)
    {0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00}, // U+002D (-)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // U+002E (.)
    {0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00}, // U+002F (/)
    {0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00}, // U+0030 (0)
    {0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00}, // U+0031 (1)
    {0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00}, // U+0032 (2)
    {0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00}, // U+0033 (3)
    {0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00}, // U+0034 (4)
    {0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00}, // U+0035 (5)
    {0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00}, // U+0036 (6)
    {0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00}, // U+0037 (7)
    {0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00}, // U+0038 (8)
    {0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00}, // U+0039 (9)
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // U+003A (:)
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x06}, // U+003B (;)
    {0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00}, // U+003C (<)
    {0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00}, // U+003D (=)
    {0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00}, // U+003E (>)
    {0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00}, // U+003F (?)
    {0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00}, // U+0040 (@)
    {0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00}, // U+0041 (A)
    {0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00}, // U+0042 (B)
    {0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00}, // U+0043 (C)
    {0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00}, // U+0044 (D)
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00}, // U+0045 (E)
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00}, // U+0046 (F)
    {0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00}, // U+0047 (G)
    {0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00}, // U+0048 (H)
    {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // U+0049 (I)
    {0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00}, // U+004A (J)
    {0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00}, // U+004B (K)
    {0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00}, // U+004C (L)
    {0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00}, // U+004D (M)
    {0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00}, // U+004E (N)
    {0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00}, // U+004F (O)
    {0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00}, // U+0050 (P)
    {0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00}, // U+0051 (Q)
    {0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00}, // U+0052 (R)
    {0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00}, // U+0053 (S)
    {0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // U+0054 (T)
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00}, // U+0055 (U)
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}, // U+0056 (V)
    {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00}, // U+0057 (W)
    {0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00}, // U+0058 (X)
    {0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00}, // U+0059 (Y)
    {0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00}, // U+005A (Z)
    {0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00}, // U+005B ([)
    {0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00}, // U+005C (\)
    {0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00}, // U+005D (])
    {0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00}, // U+005E (^)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF}, // U+005F (_)
    {0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+0060 (`)
    {0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00}, // U+0061 (a)
    {0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00}, // U+0062 (b)
    {0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00}, // U+0063 (c)
    {0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00}, // U+0064 (d)
    {0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00}, // U+0065 (e)
    {0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00}, // U+0066 (f)
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F}, // U+0067 (g)
    {0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00}, // U+0068 (h)
    {0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // U+0069 (i)
    {0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E}, // U+006A (j)
    {0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00}, // U+006B (k)
    {0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // U+006C (l)
    {0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00}, // U+006D (m)
    {0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00}, // U+006E (n)
    {0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00}, // U+006F (o)
    {0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F}, // U+0070 (p)
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78}, // U+0071 (q)
    {0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00}, // U+0072 (r)
    {0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00}, // U+0073 (s)
    {0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00}, // U+0074 (t)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00}, // U+0075 (u)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}, // U+0076 (v)
    {0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00}, // U+0077 (w)
    {0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00}, // U+0078 (x)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F}, // U+0079 (y)
    {0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00}, // U+007A (z)
    {0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x38, 0x00}, // U+007B ({)
    {0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00}, // U+007C (|)
    {0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x07, 0x00}, // U+007D (})
    {0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // U+007E (~)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // U+007F
};
