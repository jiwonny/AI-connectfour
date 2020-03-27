#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <vector>

// winning line vector 부분
using namespace std;

typedef struct {
	int pos[4][2];
	//line 이루는 각 4개의 cell 마다 x좌표 y좌표
} node;


vector<node> WLINE_O;   // winning lines of O
vector<node> WLINE_X;   // winning lines of X

				  // winning line vector 정의 끝.

#define BOARD_ROWS 6
#define BOARD_COLS 7

#define PERFECT_DEPTH 12
extern char board[BOARD_ROWS][BOARD_COLS];

//--함수 정의 시작
void removeLine(vector<node>*, int, int);
void editWLINE(int col, char mark);

void StandardWLine(void);// 보드의 각 칸마다 winning line 생성
int findNextMove(int level, int turn);// 보드의 모든 열을 탐색하는 데 걸린 시간 출력 + CPU의 선택과 그에 대한 이유 설명
int colHeight(int col);// 탐색하는 column 에 대해서, 둘 수 있는 위치 (ROW) 반환
int checkWLine(char mark, node cell);// 가능한 나의 winning line 들 중 4개 다 나의 mark로 채워지면 내가 이기게 되는 경우
int getScore(char mark);// evaluation table을 사용하여각 node (status) 에 대한 점수 반환
int checkFull(int column);// column이 꽉 찼는가? check
int funcHeuristic(int ismax, char mark, int select, int depth, int level, int alpha, int beta);// Heuristic
int findMaxHeuristic(int depth, int level, char mark, int turn);// Heuristic 점수 중 max값 반환
int getHint(int yes, int depth, int level, char mark, int turn);// Heuristic을 사용하여 플레이어에게 힌트 제공
int sequenceScore(char mark);
void printReason(int turn, int maxcol, int score[], int level, char mark);

extern int checkFirst(int);// Player First == 0, AI First == 1

					 //--함수 정의 끝


					 //나의 착수점에 따른 상대방의 winning line 제거
void removeLine(vector<node>* obj, int xpos, int ypos) {
	for (vector<node>::iterator pr = (*obj).begin(); pr != (*obj).end();) {
		for (int i = 0; i < 4; i++) {
			if ((*pr).pos[i][0] == xpos && (*pr).pos[i][1] == ypos) {
				pr = (*obj).erase(pr);
				break;
			}
			if (i == 3) pr++;
		}
	}
}

//나의 착수점에 따른 상대방의 winning line 제거
void editWLINE(int col, char mark) {
	int prevCol = col;
	int prevRow = colHeight(prevCol) - 1;
	char prevMark = mark;
	char currentMark = (prevMark == 'X') ? 'O' : 'X';

	//printf("\n%d %d에 뒀다", prevCol, prevRow);
	//printf("\n---prev----WLINE_O size %d", WLINE_O.size());
	//printf("\n---prev----WLINE_X size %d", WLINE_X.size());


	if (prevMark == 'X') {
		removeLine(&WLINE_O, prevRow, prevCol);
	}
	else removeLine(&WLINE_X, prevRow, prevCol);

	/*printf("\n---current----WLINE_O size %d", WLINE_O.size());
	printf("\n---current----WLINE_X size %d", WLINE_X.size());*/
}

void StandardWLine() {
	//board 의 각 칸 마다 winning line 생성.

	for (int i = 0; i < BOARD_ROWS; i++) {
		for (int j = 0; j < BOARD_COLS - 3; j++) {
			int line[4][2] = { { i, j },{ i, j + 1 },{ i, j + 2 },{ i, j + 3 } }; // 가로
			node n;
			for (int p = 0; p < 4; p++) {
				for (int q = 0; q < 2; q++) {
					n.pos[p][q] = line[p][q]; // node n을 만들어서 모든 pos에 값을 넣고
				}
			}
			WLINE_O.push_back(n); // WLINE_O에 push
			WLINE_X.push_back(n); // WLINE_X에 push
		}
	}

	for (int i = 0; i < BOARD_ROWS - 3; i++) {
		for (int j = 0; j < BOARD_COLS; j++) {
			int line[4][2] = { { i, j },{ i + 1, j },{ i + 2, j },{ i + 3, j } }; // 세로
			node n;
			for (int p = 0; p < 4; p++) {
				for (int q = 0; q < 2; q++) {
					n.pos[p][q] = line[p][q];
				}
			}
			WLINE_O.push_back(n);
			WLINE_X.push_back(n);
		}
	}

	for (int i = 0; i < BOARD_ROWS - 3; i++) {
		for (int j = 0; j < BOARD_COLS - 3; j++) {
			int line[4][2] = { { i, j },{ i + 1, j + 1 },{ i + 2, j + 2 },{ i + 3, j + 3 } }; // 45도 대각선
			node n;
			for (int p = 0; p < 4; p++) {
				for (int q = 0; q < 2; q++) {
					n.pos[p][q] = line[p][q];
				}
			}
			WLINE_O.push_back(n);
			WLINE_X.push_back(n);
		}
	}
	for (int i = BOARD_ROWS - 1; i >= 3; i--) {
		for (int j = 0; j < BOARD_COLS - 3; j++) {
			int line[4][2] = { { i, j },{ i - 1, j + 1 },{ i - 2, j + 2 },{ i - 3, j + 3 } }; // 135도 대각선
			node n;
			for (int p = 0; p < 4; p++) {
				for (int q = 0; q < 2; q++) {
					n.pos[p][q] = line[p][q];
				}
			}
			WLINE_O.push_back(n);
			WLINE_X.push_back(n);
		}
	}
}

int findNextMove(int level, int turn) {
	//player 가 첫번째로 시행하는 것이면 AI 의 mark = X.
	char mark = (checkFirst(0) == 0) ? 'X' : 'O'; // checkFirst==0이면 Player 선공, ==1 이면 AI First
	int column;

	time_t startTime = clock();
	column = findMaxHeuristic(1, level, mark, turn);
	time_t endTime = clock();
	column++;

	printf("Execution Time : %.3lf", (float)(endTime - startTime) / (CLOCKS_PER_SEC));
	printf("\nCPU's choice: ");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
	printf("Column %d\n", column);

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);

	column--;
	Sleep(8000);
	srand(time(NULL));

	return column;
}



int colHeight(int col) { // 탐색하는 column 에 대해서, 둘 수 있는 위치 (ROW) 반환
	int row;
	for (row = 0; row < BOARD_ROWS; row++) {
		if (board[row][col] == ' ') {
			return row;
		}
	}
	return row;
}

int checkWLine(char mark, node cell) {
	//가능한 나의 winning line 들 중 다 나의 mark로 채워져있다면  == 내가 이기게 되는 경우
	if (board[cell.pos[0][0]][cell.pos[0][1]] == mark &&
		board[cell.pos[1][0]][cell.pos[1][1]] == mark &&
		board[cell.pos[2][0]][cell.pos[2][1]] == mark &&
		board[cell.pos[3][0]][cell.pos[3][1]] == mark) {
		return 1000;
	}
}

int checkLLine(char mark, node cell) { //check lose line. 상대방의 mark 에 winning line 있는가?
							  // 가능한 상대방의 winning line 들 중 다 상대방의 mark 로 채워져있다면 == 상대방이 이기게 되는 경우
	char opp = (mark == 'O') ? 'X' : 'O'; //opponent mark

	if (board[cell.pos[0][0]][cell.pos[0][1]] == opp &&
		board[cell.pos[1][0]][cell.pos[1][1]] == opp &&
		board[cell.pos[2][0]][cell.pos[2][1]] == opp &&
		board[cell.pos[3][0]][cell.pos[3][1]] == opp) {
		return -1000;
	}
}

int getScore(char mark) {
	// 각 node (status) 에 대한 점수 반환.

	int score_O = 0;
	int score_X = 0;


	std::vector<node>::iterator pr;
	if (mark == 'X') {
		for (pr = WLINE_O.begin(); pr != WLINE_O.end(); ++pr) {
			if (checkLLine(mark, *(pr)) == -1000) return -1000;
		}

		for (pr = WLINE_X.begin(); pr != WLINE_X.end(); ++pr) {
			if (checkWLine(mark, *(pr)) == 1000) return 1000;
		}
	}
	else {
		for (pr = WLINE_X.begin(); pr != WLINE_X.end(); ++pr) {
			if (checkLLine(mark, *(pr)) == -1000) return -1000;
		}
		for (pr = WLINE_O.begin(); pr != WLINE_O.end(); ++pr) {
			if (checkWLine(mark, *(pr)) == 1000) return 1000;
		}
	}


	score_O = sequenceScore('O') + WLINE_O.size();
	score_X = sequenceScore('X') + WLINE_X.size();


	//현재 두어야 하는 mark 가 O 인지 X 인지?
	return (mark == 'O') ? (score_O - score_X) : (score_X - score_O);
}

int checkFull(int column) {
	if (colHeight(column) == BOARD_ROWS) return 1;
	else return 0;
}

int funcHeuristic(int ismax, char mark, int select, int depth, int level, int alpha, int beta) {
	// turn n에서 휴리스틱하게 점수 판단해서 값 반환
	 //ismax = 자식 노드들에 대해 maximize 할 것이라면 1 (나 차례), minimize 할 것이라면 0 (너 차례)

	int revDepth = level - depth + 1;
	int minimizing = 0;
	int maximizing = 1;
	int value;

	//select : 어떤 column 에 대해서 heuristic 점수를 반환할 것인가?
	if (checkFull(select) == 1) return -3000; // column 이 다 차있다면 선택되지 않도록 최소의 값.
	board[colHeight(select)][select] = mark; //board 에 mark 둬본 상태에서 점수 반환할 것임.

	 //printf("\n%d 번째 column 에서 %d 번째 수를 탐색하겠다. : %d", select, depth, getScore(mark)); //디버깅

	int score = getScore(mark);// evaluation table을 사용하여 노드 별 점수를 얻어온 후 alpha-beta pruning

	if (depth == level) {
		value = (ismax == 1) ? -score : score; //최대화 노드라면, 상대방 차례이므로 score * (-1)
	}
	else {
		if (score == 1000 || score == -1000) {
			board[colHeight(select) - 1][select] = ' ';
			return (ismax == 1) ? -(score + revDepth) : (score + revDepth);
		}

		else {
			if (ismax == 1) { // 최대화노드일 경우 (상대방 차례일 경우) select max
				value = -1000 - PERFECT_DEPTH;
				for (int i = 0; i < BOARD_COLS; i++) {
					int temp = funcHeuristic(minimizing, (mark == 'O') ? 'X' : 'O', i, depth + 1, level, alpha, beta);
					if (temp != -3000) {
						value = max(value, temp);
						if (value >= beta) {
							//printf("\n value %d >= %d (beta) 이므로 sibling 확인 안함",value,beta); //디버깅
							board[colHeight(select) - 1][select] = ' ';
							return value;
						}

						alpha = max(alpha, value);
					}
				}
			}
			else {
				value = 1000 + PERFECT_DEPTH;
				for (int i = 0; i < BOARD_COLS; i++) {
					int temp = funcHeuristic(maximizing, (mark == 'O') ? 'X' : 'O', i, depth + 1, level, alpha, beta);
					//printf("***temp 값 %d ****", temp); //디버깅
					if (temp != -3000) {
						value = min(value, temp);
						if (value <= alpha) {
							//printf("value %d <= %d (alpha) 이므로 sibling 확인 안함",value,alpha); //디버깅
							board[colHeight(select) - 1][select] = ' ';
							return value;

						}
						beta = min(beta, value);
					}
				}
			}
		}

	}

	board[colHeight(select) - 1][select] = ' '; // 아까 둬봤던 돌 제거
									 //printf("turn : %d column : %d, score = %d\n", turn, choice, score);
	return value; // 점수 리턴
}

int findMaxHeuristic(int depth, int level, char mark, int turn) { // 휴리스틱하게 구한 7개 점수에서 최고점을 찾아 그 col을 반환하는 함수
	int score[BOARD_COLS];
	int revDepth = level - depth + 1; //depth 에 따라 다른 점수를 부과하기 위해 depth reverse.
	int max = 0;
	int minimizing = 0; //본인 차례일때, 다음 노드에 대해서 minimize.

	printf(" Column     Score\n");
	// easy(level == 2) 인 상황은 항상 level = 2 로 탐색.
	// hard(level == 12) 인 상황은 AI 가 mark 4번 둘 때 까지는 7 수 앞만 탐색.
	if (turn == 0) {
		for (int i = 0; i < BOARD_COLS; i++) {
			score[i] = 7 - abs(3 - i);
			printf("   %d        %d\n", i + 1, score[i]);
		}
	}
	else if (level != 2 && turn <= 7) { //AI 가 mark 를 네 번 둘 때까지는 네 수 앞까지만 탐색. level <= 7 로 줌
		for (int i = 0; i < BOARD_COLS; i++) {
			int reducedLevel = 7;
			score[i] = funcHeuristic(minimizing, mark, i, depth, reducedLevel, -1000 - revDepth, 1000 + revDepth);
			// 다음 depth 들에 대해서 minimize 할 노드이므로 pass minimizing parameter.
			printf("   %d        %d\n", i + 1, score[i]);
		}
	}
	else {
		// level == 2 이거나, AI 가 mark 5번째 둘 때부터 6 수 or 12 수 앞까지 다 탐색.
		//printf("(turn %d 실행) %d 수 앞까지 내다보겠어요",turn, level);
		for (int i = 0; i < BOARD_COLS; i++) {
			score[i] = funcHeuristic(minimizing, mark, i, depth, level, -1000 - revDepth, 1000 + revDepth);
			// 맨 처음은 AI가 두는 max이므로 true, turn과 mark은 parameter이고 i는 for문으로 0~6까지 돌려봄
			printf("   %d        %d\n", i + 1, score[i]);
		}
	}

	for (int i = 0; i < BOARD_COLS; i++) { // 최고점수 찾기- 동점인 경우 가운데 열과 가까운 쪽부터 착수하게 tie-breaking
		if (turn == 0 && i == 3) continue;
		if (score[i] > score[max]) max = i;
		else if (score[i] == score[max]) {
			if (abs(3 - max) >= abs(3 - i)) max = i;
		}
	}

	printReason(turn, max, score, level, mark);
	return max;
}

void printReason(int turn, int maxcol, int score[], int level, char mark) {
	int min = 0;
	char opp = (mark == 'O') ? 'X' : 'O';
	for (int i = 0; i < BOARD_COLS; i++) { // 최고점수 찾기- 동점인 경우 가운데 열과 가까운 쪽부터 착수하게 tie-breaking
		if (score[i] < score[min]) min = i;
		else if (score[i] == score[min]) {
			if (abs(3 - min) <= abs(3 - i)) min = i;
		}
	}

	if (turn == 0) {
		printf("\n첫 turn 의 경우 winning line 수가 많은 column 을 선택한다. 가운데에 가까운 column 을 선택한다.\n다만 4를 선택할 수 없으므로 3, 5 중에 하나를 선택한다.\n");
	}
	else if (score[maxcol] >= 1000) {
		int tempLevel = (turn <= 7 && level == 12) ? 7 : level;

		printf("\nsearch 결과, AI가 %d 수 앞을 탐색하는 과정에서 AI 가 이기게 되는 경우가 나와서 %d 를 선택.", tempLevel - (score[maxcol] - 1000) + 1, maxcol + 1);
	}
	else if (score[min] <= -1000) {

		int tempLevel = (turn <= 7 && level == 12) ? 7 : level;

		printf("\nsearch 결과, AI가 %d 수 앞을 탐색하는 과정에서 상대방이 이기게 되는 경우가 나와서 %d 를 선택안함.", tempLevel - (-1000 - score[min]) + 1, min + 1);

		if (score[maxcol] > -1000) {
			board[colHeight(maxcol)][maxcol] = mark; //선택된 col 에 돌을 두고 연속 돌 개수 계산

			int tempScore = sequenceScore(mark);
			int oppScore = sequenceScore(opp);

			printf("\n현재 착수점에 돌을 두었을 때, 연속된 돌 개수에 대한 점수가 %d(%c 의 점수) - %d(%c 의 점수) = \"%d\" 이다.", tempScore, mark, oppScore, opp, tempScore - oppScore);
			printf("\n 이러한 방식과 winning Line 개수의 차이를 함께 계산하며 \"%d\" 수 앞까지 더 봤을 때\n \"Column %d\" 가 가장 높은 점수를 산출하므로 이를 선택", tempLevel, maxcol + 1);

			board[colHeight(maxcol) - 1][maxcol] = ' ';

		}
	}
	else {
		board[colHeight(maxcol)][maxcol] = mark; //선택된 col 에 돌을 두고 연속 돌 개수 계산

		int tempScore = sequenceScore(mark);
		int oppScore = sequenceScore(opp);

		int tempLevel = (turn <= 7 && level == 12) ? 7 : level;

		printf("\n현재 착수점에 돌을 두었을 때, 연속된 돌 개수에 대한 점수가 %d(\'%c\' 의 점수) - %d(\'%c\' 의 점수) = \"%d\" 이다.", tempScore, mark, oppScore, opp, tempScore - oppScore);
		printf("\n (이 방식으로 구한 값 + winning Line 개수 차이)로 나온 값을 이용해 \"%d\" 수 앞까지 더 봤을 때\n \"Column %d\" 가 가장 높은 점수를 산출하므로 이를 선택", tempLevel, maxcol + 1);

		board[colHeight(maxcol) - 1][maxcol] = ' ';
	}
	printf("\n");
}

int getHint(int yes, int depth, int level, char mark, int turn) {
	int score[BOARD_COLS];
	//parameter: depth, mark
	int minimizing = 0;
	while (yes >= 0) { //hint가 존재할때까지만 사용가능
		for (int i = 0; i < BOARD_COLS; i++) {
			score[i] = funcHeuristic(minimizing, mark, i, depth, level, -1000 - (level - depth + 1), 1000 + (level - depth + 1));
		}

		int max = 0;
		for (int i = 0; i < BOARD_COLS; i++) { // 최고점수 찾기- 동점인 경우 가운데 열과 가까운 쪽부터 착수하게 tie-breaking
			if (turn == 0 && i == 3) continue;
			if (score[i] > score[max]) max = i;
			else if (score[i] == score[max]) {
				if (abs(3 - max) >= abs(3 - i)) max = i;
			}
		}
		return max; // column 리턴
	}
	return -1;
}

int sequenceScore(char mark) {
	// 일렬로 놓이는 돌의 개수가 많을수록 점수를 더 많이 부여.
	int seqcount = 0;
	std::vector<node>::iterator pr;
	if (mark == 'O') {
		for (pr = WLINE_O.begin(); pr != WLINE_O.end(); ++pr) {
			node n = *(pr);
			//if (board[n.pos[1][0]][n.pos[1][1]] != mark) continue;

			if (board[n.pos[0][0]][n.pos[0][1]] == mark &&
				board[n.pos[1][0]][n.pos[1][1]] == mark &&
				board[n.pos[2][0]][n.pos[2][1]] == mark) {
				seqcount += 4;
			}
			else if (board[n.pos[0][0]][n.pos[0][1]] == mark &&
				board[n.pos[1][0]][n.pos[1][1]] == mark &&
				board[n.pos[2][1]][n.pos[2][1]] != mark) {
				seqcount += 2;
			}
		}
	}
	else {
		for (pr = WLINE_X.begin(); pr != WLINE_X.end(); ++pr) {
			node n = *(pr);
			if (board[n.pos[0][0]][n.pos[0][1]] == mark &&
				board[n.pos[1][0]][n.pos[1][1]] == mark &&
				board[n.pos[2][0]][n.pos[2][1]] == mark) {
				seqcount += 4;
			}
			else if (board[n.pos[0][0]][n.pos[0][1]] == mark &&
				board[n.pos[1][0]][n.pos[1][1]] == mark &&
				board[n.pos[2][1]][n.pos[2][1]] != mark) {
				seqcount += 2;
			}
		}
	}

	return seqcount;
}