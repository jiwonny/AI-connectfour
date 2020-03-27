#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <windows.h>


#pragma warning(disable:4996)
#define BOARD_ROWS 6
#define BOARD_COLS 7

void MakeBoard(char(*board)[BOARD_COLS]);
int ModeAI(char(*board)[BOARD_COLS], int player, const char* PIECES, int level, int turn);
int ModePVP(char(*board)[BOARD_COLS], int player, const char* PIECES, int turn);
int checkWin(char(*board)[BOARD_COLS]);
int connectCheck(char(*board)[BOARD_COLS]);

int checkFirst(int first);
int checkMode(int mode);

extern int findNextMove(int level, int turn);
extern void FindCurrentBoardStatus();
extern void StandardWLine();
extern int getHint(int yes, int depth, int level, char mark, int turn);
extern void editWLINE(int col, char mark);

char board[BOARD_ROWS][BOARD_COLS];
int numOfHint = 5;
int yes = 5;
int main() {
	StandardWLine();
	const char* PIECES = "OX";
	memset(board, ' ', BOARD_ROWS * BOARD_COLS);

	int turn, done = 0;
	int mode = 0; 2;
	int first = 0;
	int level = 0;

	int f = 1;
	while (f) {
		puts("\n    ^~^ CONNECT FOUR ^~^\n");
		printf("    1. Player vs Player\n");
		printf("    2. Player vs AI\n\n");
		printf("    Please Select Mode : ");
		f = 0;

		while (1) {
			if (1 != scanf("%d", &mode) || mode < 1 || mode > 2) {
				while (getchar() != '\n');
				printf("\n    ERROR! Try again.\n    Please Select Mode : ");
			}
			else {
				break;
			}
		}
	}
	system("cls");

	if (mode == 1) {
		//done : 이기면 1 return
		for (turn = 0; turn < BOARD_ROWS * BOARD_COLS && !done; turn++) {
			MakeBoard(board);


			//공을 놓았다면 ModePVP 1 return
			//turn%2 == 0 or 1 -> player
			//공을 놓지 못했을 경우 column is full
			while (!ModePVP(board, turn % 2, PIECES, turn)) {
				MakeBoard(board);
				if (yes >= 0)
					puts("This column is FULL!\n");
			}

			//horizontal, vertical, diagonal 4개면 return 1
			done = checkWin(board);
		}
		MakeBoard(board);

		// done 이 아닌데 마지막 turn 일 때
		if (turn == BOARD_ROWS * BOARD_COLS && !done) {
			puts("It's a tie!");
		}
		else {
			//done 이거나 board 가 다 안채워졌을 때.
			turn--;
			//이전의 turn 이 먼저 공을 놓은 player 라면.  
			if (turn % 2 == checkFirst(0)) printf("Player %d wins!\n", turn % 2 + 1);
			else printf("Player %d wins!\n", turn % 2 + 1);
		}
		return 0;

	}
	else
	{
		int l = 1;
		while (l) {
			puts("\n    ^~^ CONNECT FOUR ^~^\n");
			printf("       1. Hard\n");
			printf("       2. Moderate\n");
			printf("       3. Easy\n\n");
			printf("    Please Select Level : ");
			l = 0;

			while (1) {
				if (1 != scanf("%d", &level) || level < 1 || level > 3) {
					while (getchar() != '\n');
					printf("\n    ERROR! Try again.\n    Please Select Level : ");
				}
				else {
					break;
				}
			}
		}
		system("cls");

		if (level == 1) {//Hard: depth=12
			level = 12;
		}
		else if (level == 2) {//Moderate: depth=6
			level = 6;
		}
		else {//Easy: depth=2
			level = 2;
		}


		int first_2 = 0;
		int f_2 = 1;
		while (f_2) {
			puts("\n    ^~^ CONNECT FOUR ^~^\n");
			printf("    1. Player First\n");
			printf("    2. AI First\n\n");
			printf("    Please Select Mode : ");
			f_2 = 0;

			while (1) {
				if (1 != scanf("%d", &first_2) || first_2 < 1 || first_2 > 2) {
					while (getchar() != '\n');
					printf("\n    ERROR! Try again.\n    Please Select Mode : ");
				}
				else {
					break;
				}
			}
		}
		checkFirst(first_2);
		for (turn = 0; turn < BOARD_ROWS * BOARD_COLS && !done; turn++) {
			MakeBoard(board);
			while (!ModeAI(board, turn % 2, PIECES, level, turn)) {
				MakeBoard(board);
				if (yes >= 0)
					puts("This column is FULL!\n");
			}
			done = checkWin(board);
		}
		MakeBoard(board);

		if (turn == BOARD_ROWS * BOARD_COLS && !done) {
			puts("It's a tie!");
		}
		else {
			turn--;
			if (turn % 2 == checkFirst(0)) printf("Player (%c) wins!\n", PIECES[turn % 2]);
			else printf("AI (%c) wins!\n", PIECES[turn % 2]);
		}
		return 0;
	}

}

void MakeBoard(char(*board)[BOARD_COLS]) {
	int row, col;

	system("cls");
	puts("\n    ^~^ CONNECT FOUR ^~^\n");
	for (row = BOARD_ROWS - 1; row >= 0; row--) {
		for (col = 0; col < BOARD_COLS; col++) {
			if (board[row][col] == 'O') {
				printf("| ");
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);
				printf("%c ", board[row][col]);
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
			}
			else if (board[row][col] == 'X') {
				printf("| ");
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 13);
				printf("%c ", board[row][col]);
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
			}
			else if (board[row][col] == ' ') {
				printf("| %c ", board[row][col]);
			}
		}
		puts("|");
		puts("-----------------------------");

	}
	puts("  1   2   3   4   5   6   7\n");
}

int ModePVP(char(*board)[BOARD_COLS], int player, const char* PIECES, int turn) {
	int row = 0, col = 0;
	int firstTime = 1;
	if (player == 0) {
		printf("Player 1 turn! : ");
		while (1) {
			if (1 != scanf("%d", &col) || col < 0 || col>7) {
				while (getchar() != '\n');
				puts("Number out of bounds! Try again..");
			}
			else if (turn == 0 && col == 4) {
				while (getchar() != '\n');
				printf("First marker CANNOT be placed in the fourth column! Try again..\n");
			}
			else {
				break;
			}
		}
		col--;
		for (row = 0; row < BOARD_ROWS; row++) {
			if (board[row][col] == ' ') {
				board[row][col] = PIECES[player];
				return 1;
			}
		}
	}
	else {
		printf("Player 2 turn! : ");
		while (1) {
			if (1 != scanf("%d", &col) || col < 1 || col > 7) {
				while (getchar() != '\n');
				puts("Number out of bounds! Try again.");
			}
			else {
				break;
			}
		}
		col--;
		for (row = 0; row < BOARD_ROWS; row++) {
			if (board[row][col] == ' ') {
				board[row][col] = PIECES[player];
				return 1;
			}
		}
	}

}
int ModeAI(char(*board)[BOARD_COLS], int player, const char* PIECES, int level, int turn) {
	int row, col = 0;
	//player turn일 경우
	if (player == checkFirst(0)) {
		printf("Player Turn!\n");

		printf("\nChoose Column Number.\n");
		if (numOfHint >= 0) {
			printf("Remaining Hints : %d \n", numOfHint);
		}
		else if (numOfHint <= 0 && yes == 0) {
			printf("ERROR! You CANNOT use any more hints..:(\n");

		}
		else printf("ERROR! You CANNOT use any more hints..:(\n");
		printf("Type 0 for a HINT : ");


		while (1) {
			if (1 != scanf("%d", &col) || col < 0 || col>7) {
				while (getchar() != '\n');
				puts("Number out of bounds! Try again..");
				printf("\nChoose Column Number.\n");
				if (numOfHint >= 0) {
					printf("Remaining Hints : %d \n", numOfHint);
				}
				else if (numOfHint <= 0 && yes <= 0) {
					printf("ERROR! You CANNOT use any more hints..:(\n");
				}
				printf("\nType 0 for a HINT : ");
			}
			else if (turn == 0 && col == 4) {
				while (getchar() != '\n');
				printf("First marker CANNOT be placed in the fourth column! Try again..\n");
				printf("\nChoose Column Number.\n");
				if (numOfHint >= 0) {
					printf("Remaining Hints : %d \n", numOfHint);
				}
				else if (numOfHint <= 0 && yes <= 0) {
					printf("ERROR! You CANNOT use any more hints..:(\n");
				}
				printf("\nType 0 for a HINT : ");
			}
			else {
				break;
			}
		}
		if (col == 0) {//hint를 요청했을 경우
			yes -= 1;  //hint 사용시 하나씩 차감
			while (yes >= 0) {//hint가 존재할때까지
				char mark = (checkFirst(0) == 0) ? 'X' : 'O';
				int hint = getHint(yes, 1, 6, mark, turn);
				numOfHint--;
				printf("\nLine %d may be your Best Choice!\n", hint + 1);
				printf("Remaining Hints : %d\n", numOfHint);
				printf("\nChoose Column Number. : ");

				while (1) {
					if (1 != scanf("%d", &col) || col < 0 || col > 7 || (turn == 0 && col == 4)) {
						while (getchar() != '\n');
						if (turn == 0 && col == 4)
							puts("First marker can not be placed in the fourth column! Try again");
						else
							puts("Number out of bounds! Try again.");

						printf("\nNumber out of bounds! Try again.\n");
						printf("\nChoose Column Number.");
						printf("Remaining Hints : %d \n", numOfHint);

						printf("\nType 0 for a HINT : ");
					}
					else {
						break;
					}
				}
				col--;
				for (row = 0; row < BOARD_ROWS; row++) {
					if (board[row][col] == ' ') {
						board[row][col] = PIECES[player];
						editWLINE(col, PIECES[player]);
						return 1;
					}
				}
			}
			printf("ERROR! You CANNOT use any more hints..:(\n");
			Sleep(3000);
		}
		else {
			col--;
			for (row = 0; row < BOARD_ROWS; row++) {
				if (board[row][col] == ' ') {
					board[row][col] = PIECES[player];
					editWLINE(col, PIECES[player]);
					return 1;
				}
			}
		}

	}
	else {
		printf("AI's Turn!\n");

		col = findNextMove(level, turn);

		for (row = 0; row < BOARD_ROWS; row++) {
			if (board[row][col] == ' ') {
				board[row][col] = PIECES[player];
				editWLINE(col, PIECES[player]);
				return 1;
			}
		}
	}
	return 0;

}
int checkWin(char(*board)[BOARD_COLS]) {
	return (connectCheck(board));
}

int connectCheck(char(*board)[BOARD_COLS]) {
	//horizontal
	for (int row = 0; row < BOARD_ROWS; row++) {
		for (int col = 0; col < BOARD_COLS - 3; col++) {
			char temp = board[row][col];
			if ((temp != ' ') &&
				(board[row][col + 1] == temp) &&
				(board[row][col + 2] == temp) &&
				(board[row][col + 3] == temp))
			{
				return 1;
			}
		}
	}

	//vertical
	for (int row = 0; row < BOARD_ROWS - 3; row++) {
		for (int col = 0; col < BOARD_COLS; col++) {
			char temp = board[row][col];
			if ((temp != ' ') &&
				(board[row + 1][col] == temp) &&
				(board[row + 2][col] == temp) &&
				(board[row + 3][col] == temp))
			{
				return 1;
			}
		}
	}

	//diagonal
	for (int row = 0; row < BOARD_ROWS - 3; row++) {
		for (int col = 0; col < BOARD_COLS - 3; col++) {
			char temp = board[row][col];
			if ((temp != ' ') &&
				(board[row + 1][col + 1] == temp) &&
				(board[row + 2][col + 2] == temp) &&
				(board[row + 3][col + 3] == temp))
			{
				return 1;
			}
		}
	}

	for (int row = 0; row < BOARD_ROWS - 3; row++) {
		for (int col = 3; col < BOARD_COLS; col++) {
			char temp = board[row][col];
			if ((temp != ' ') &&
				(board[row + 1][col - 1] == temp) &&
				(board[row + 2][col - 2] == temp) &&
				(board[row + 3][col - 3] == temp))
			{
				return 1;
			}
		}
	}
	return 0;

}

int checkFirst(int first) { // Player First == 0, AI First == 1
					 //n 을 static int 로 선언함으로써 값 유지.
	static int n = -1;
	if (n == -1) n = first - 1;
	return n;
}

int checkMode(int mode) {
	static int n = -1;
	if (n == -1) n = mode - 1;
	return n;
}