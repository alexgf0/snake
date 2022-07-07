#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <termios.h>
#include <sys/select.h>
#define say(str) write(1, str, sizeof(str))
#define menuHeight '3'

struct termios initial;
int status, score, snakeHead[2], snakeTail[2];
char board[16][16];

void menu();
int get_input();

int game_input() { //exit=0, up=-1, down=1, right=-2, left=2, '\n'=3, keep same direction=-3
  int retval;
  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(0, &rfds);

  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;

  retval = select(1, &rfds, NULL, NULL, &timeout);

  if(retval == 1)
    return get_input();
  else
    return -3;
}


bool check_straight_collision(char nextPos, int y, int x) {
  switch(nextPos) {
    case 'w':
      if (y == 1)
       return true;
      else
       return false;
      break;
    case 'a':
      if (x == 1)
       return true;
      else
       return false;
      break;
    case 's':
      if (y == -1)
        return true;
      else
        return false;
      break;
    case 'd':
      if (x == -1)
        return true;
      else
        return false;
      break;
  }
}


bool move(int y, int x) {
  char nextPos = board[(snakeHead[0] + y)] [(snakeHead[1] + x)];
  if(snakeHead[0]+y == 15 || snakeHead[0]+y == 0 || snakeHead[1]+x == 15 || snakeHead[1]+x == 0)
    return false;
  else if(nextPos == 'w' || nextPos == 'a' || nextPos == 's' || nextPos == 'd') { //hits it's boddy
    if(check_straight_collision(nextPos, y, x))
      return true;
    else
      return false;
  } else { //apple && normal move
    switch (y) {
      case -1:
        board[snakeHead[0]][snakeHead[1]] = 'w';
        snakeHead[0] -= 1;
        board[snakeHead[0]][snakeHead[1]] = 'O';
        break;
      case 1:
        board[snakeHead[0]][snakeHead[1]] = 's';
        snakeHead[0] += 1;
        board[snakeHead[0]][snakeHead[1]] = 'O';
        break;
      default:
        break;
    }
    switch(x) {
      case -1:
        board[snakeHead[0]][snakeHead[1]] = 'a';
        snakeHead[1] -= 1;
        board[snakeHead[0]][snakeHead[1]] = 'O';
        break;
      case 1:
        board[snakeHead[0]][snakeHead[1]] = 'd';
        snakeHead[1] += 1;
        board[snakeHead[0]][snakeHead[1]] = 'O';
        break;
      default:
        break;
    }
  if(nextPos != 'A') { //simple move (remove tail)
    char remove = board[snakeTail[0]][snakeTail[1]];
    board[snakeTail[0]][snakeTail[1]] = ' ';
    switch(remove){
      case 'w':
        snakeTail[0] -= 1;
        break;
      case 'a':
        snakeTail[1] -= 1;
        break;
      case 's':
        snakeTail[0] += 1;
        break;
      case 'd':
        snakeTail[1] += 1;
        break;
     }
     return true;
   } else
     score++;
  }
}


bool default_move() {
  if(board[(snakeHead[0]+1)][snakeHead[1]] == 'w') //if the one below has 'w' it's going upwards
    return move(-1, 0);
  else if(board[(snakeHead[0]-1)][snakeHead[1]] == 's')
    return move(1, 0);
  else if(board[snakeHead[0]][(snakeHead[1]+1)] == 'a')
    return move(0, -1);
  else if(board[snakeHead[0]][(snakeHead[1]-1)] == 'd')
    return move(0, 1);
  else
    perror("default_move");
}


bool process_input(int input) { //input: exit=0 up=-1, down=1, right=-2, left=2, '\n'=3, keep going = -3
  switch(input) {
    case 0:
      printf("\x1b[2J");
      printf("\x1b[?1049l");
      exit(0);
      break;
    case 1: //down
      return move(1, 0);
      break;
    case -1: //up
      return move(-1, 0);
      break;
    case 2: //left
      return move(0, -1);
      break;
    case -2: //right
      return move(0, 1);
      break;
    case -3: //no input
      return default_move();
      break;
  }
}


void print_board() {
  printf("\x1b[2J"); //clear terminal
  printf("\x1b[2H"); //pos 2
  for(int i=0; i<16; i++) {
    for(int j=0; j<16; j++) {
      if(i == snakeHead[0] && j == snakeHead[1])
        printf("\x1b[33mO\x1b[0m ");
      else if(board[i][j] == 'w' || board[i][j] == 'a' || board[i][j] == 's' || board[i][j] == 'd')
        printf("\x1b[32m0\x1b[0m ");
      else if(i == 0 || i == 15)
        printf("--");
      else if(j == 0)
        printf("| ");
      else if(j == 15)
        printf(" |");
      else if(board[i][j] == 'A')
        printf("\x1b[31mA\x1b[0m ");
      else
       printf("%c ", board[i][j]);
    }
   printf("\n");
  }
  printf("score: %d\n", score);
}


void initialize_board() {
 for(int i=0; i<16; i++)
    for(int j=0; j<16; j++){
      if(i == 0 || j == 0 || i == 15 || j == 15)
        board[i][j] = 'b';
      else
        board[i][j] = ' '; //empty
    }

  board[4][5] = 'd'; //snake
  snakeTail[0] = 4; snakeTail[1] = 5;
  board[4][6] = 'd';
  board[4][7] = 'O';
  snakeHead[0] = 4; snakeHead[1] = 7;
  board[12][12] = 'A'; //apple
  board[14][8] = 'A';
}


void spawn_apples() {
  int y = snakeHead[0], x = snakeHead[1];
  int counter = 0;
  while(board[y][x] != ' ') {
    if(counter == 20)
      return;
    y = (rand() % 15 + 1)-1; //rand number between [0,14]
    x = (rand() % 15 + 1)-1;
    counter++;
  }
  board[y][x] = 'A';
}


void game_loop() {
  bool alive = true;
  int input, apples = 0;
  while(alive) {
    if((apples%5) == 0)
      spawn_apples();
    apples++;
    print_board();
    input = game_input();
    alive = process_input(input);
  }
}


void game() {
  initialize_board();
  game_loop();
  menu(status);
}


void printMenu(int pos) {
  printf("\r\x1b[2J"); //clean screen
  printf("\x1b[1H"); //pos 1
  printf("   WELCOME TO SNAKE!     \n");
  printf("   -----------------     \n");
  printf("\x1b[%cH", menuHeight);

  switch(pos) {
    case 0:
      printf("   \x1b[47m\x1b[30mPLAY NOW!\x1b[0m      \t\n");
      printf("   CHECK HIGHSCORE\t\n");
      printf("   ABOUT          \t\n");
      printf("   EXIT           \t\n");
      break;
    case 1:
      printf("   PLAY NOW!      \t\n");
      printf("   \x1b[47m\x1b[30mCHECK HIGHSCORE\x1b[0m\t\n");
      printf("   ABOUT          \t\n");
      printf("   EXIT           \t\n");
      break;
    case 2:
      printf("   PLAY NOW!      \t\n");
      printf("   CHECK HIGHSCORE\t\n");
      printf("   \x1b[47m\x1b[30mABOUT\x1b[0m          \t\n");
      printf("   EXIT           \t\n");
      break;
    case 3:
      printf("   PLAY NOW!      \t\n");
      printf("   CHECK HIGHSCORE\t\n");
      printf("   ABOUT          \t\n");
      printf("   \x1b[47m\x1b[30mEXIT\x1b[0m           \t\n\n");
      break;
  }
}


int moveCursor(int move) {
  int newStatus;
  if(status == 3 && move == 1)
    newStatus = 0;
  else if(status == 0 && move == -1)
    newStatus = 3;
  else
    newStatus = status+move;

  printMenu(newStatus);
  return newStatus;
}


void restore(void) {
  tcsetattr(1, TCSANOW, &initial);
}


void die (int i) {
  printf("\x1b[?1049h");
  printf("\x1b[2J");
  printf("\x1b[?1049l");
  exit(1);
}


void terminit(void) {
	struct termios t;
	tcgetattr(1, &t);
	initial = t;
	atexit(restore);
	signal(SIGTERM, die);
	signal(SIGINT, die);
	t.c_lflag &= (~ECHO & ~ICANON);
	tcsetattr(1, TCSANOW, &t);
}


void scores() {
  printf("\x1b[2J");
  printf("\x1b[2H");
  printf("  Nope.\n");
  sleep(1);
  printf("\x1b[2J");
  menu();
}


void about() {
  printf("\x1b[2J");
  printf("\x1b[2H");
  printf("  By Alejandro García.\n");
  sleep(2);
  printf("\x1b[2J");
  menu();
}


int get_input() { //exit=0 up=-1, down=1, right=-2, left=2, '\n'=3
  char a, b[5];
  read(1, &b, 3);
  a = b[0];
  if(a == '\n')
    return 3;
  else if(a =='w')
    return -1;
  else if(a =='s')
    return 1;
  else if(a == 'a')
    return 2;
  else if(a == 'd')
    return -2;
  else if(a == '\x1b'){
    if (b[1] == '['){
      if(b[2] == 'A')
        return -1;
      else if(b[2] == 'B')
        return 1;
      else if(b[2] == 'C')
        return -2;
      else if(b[2] == 'D')
        return 2;
      } else {
        return 0;
      }
    }
}


void menu() {
  score = 0;
  printMenu(status);
  char a, b[10];
  int input = 4;
  while(input != 3) {
    input = get_input();
    if(input == 1 || input == -1)
      status = moveCursor(input);
    else if(input == 0) {
      return;
    }
  }
  switch(status) {
    case 0:
      game();
      break;
    case 1:
      scores();
      break;
    case 2:
      about();
      break;
    case 3:
      printf("\x1b[2J");
      printf("\x1b[?1049l");
      exit(0);
      break;
  }
}


int main() {
  status = 0;
  terminit();
  printf("\x1b[?1049h"); //entering terminal "alternate buffer"
  menu();
  printf("\x1b[2J"); //should clean screen (not really working)
  printf("\x1b[?1049l"); //exiting "alterante buffer"
  return 1;
}
