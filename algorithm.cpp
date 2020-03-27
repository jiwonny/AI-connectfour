#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <vector>

// winning line vector �κ�
using namespace std;

typedef struct {
	int pos[4][2];
	//line �̷�� �� 4���� cell ���� x��ǥ y��ǥ
} node;


vector<node> WLINE_O;   // winning lines of O
vector<node> WLINE_X;   // winning lines of X

				  // winning line vector ���� ��.

#define BOARD_ROWS 6
#define BOARD_COLS 7

#define PERFECT_DEPTH 12
extern char board[BOARD_ROWS][BOARD_COLS];

//--�Լ� ���� ����
void removeLine(vector<node>*, int, int);
void editWLINE(int col, char mark);

void StandardWLine(void);// ������ �� ĭ���� winning line ����
int findNextMove(int level, int turn);// ������ ��� ���� Ž���ϴ� �� �ɸ� �ð� ��� + CPU�� ���ð� �׿� ���� ���� ����
int colHeight(int col);// Ž���ϴ� column �� ���ؼ�, �� �� �ִ� ��ġ (ROW) ��ȯ
int checkWLine(char mark, node cell);// ������ ���� winning line �� �� 4�� �� ���� mark�� ä������ ���� �̱�� �Ǵ� ���
int getScore(char mark);// evaluation table�� ����Ͽ��� node (status) �� ���� ���� ��ȯ
int checkFull(int column);// column�� �� á�°�? check
int funcHeuristic(int ismax, char mark, int select, int depth, int level, int alpha, int beta);// Heuristic
int findMaxHeuristic(int depth, int level, char mark, int turn);// Heuristic ���� �� max�� ��ȯ
int getHint(int yes, int depth, int level, char mark, int turn);// Heuristic�� ����Ͽ� �÷��̾�� ��Ʈ ����
int sequenceScore(char mark);
void printReason(int turn, int maxcol, int score[], int level, char mark);

extern int checkFirst(int);// Player First == 0, AI First == 1

					 //--�Լ� ���� ��


					 //���� �������� ���� ������ winning line ����
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

//���� �������� ���� ������ winning line ����
void editWLINE(int col, char mark) {
	int prevCol = col;
	int prevRow = colHeight(prevCol) - 1;
	char prevMark = mark;
	char currentMark = (prevMark == 'X') ? 'O' : 'X';

	//printf("\n%d %d�� �״�", prevCol, prevRow);
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
	//board �� �� ĭ ���� winning line ����.

	for (int i = 0; i < BOARD_ROWS; i++) {
		for (int j = 0; j < BOARD_COLS - 3; j++) {
			int line[4][2] = { { i, j },{ i, j + 1 },{ i, j + 2 },{ i, j + 3 } }; // ����
			node n;
			for (int p = 0; p < 4; p++) {
				for (int q = 0; q < 2; q++) {
					n.pos[p][q] = line[p][q]; // node n�� ���� ��� pos�� ���� �ְ�
				}
			}
			WLINE_O.push_back(n); // WLINE_O�� push
			WLINE_X.push_back(n); // WLINE_X�� push
		}
	}

	for (int i = 0; i < BOARD_ROWS - 3; i++) {
		for (int j = 0; j < BOARD_COLS; j++) {
			int line[4][2] = { { i, j },{ i + 1, j },{ i + 2, j },{ i + 3, j } }; // ����
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
			int line[4][2] = { { i, j },{ i + 1, j + 1 },{ i + 2, j + 2 },{ i + 3, j + 3 } }; // 45�� �밢��
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
			int line[4][2] = { { i, j },{ i - 1, j + 1 },{ i - 2, j + 2 },{ i - 3, j + 3 } }; // 135�� �밢��
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
	//player �� ù��°�� �����ϴ� ���̸� AI �� mark = X.
	char mark = (checkFirst(0) == 0) ? 'X' : 'O'; // checkFirst==0�̸� Player ����, ==1 �̸� AI First
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



int colHeight(int col) { // Ž���ϴ� column �� ���ؼ�, �� �� �ִ� ��ġ (ROW) ��ȯ
	int row;
	for (row = 0; row < BOARD_ROWS; row++) {
		if (board[row][col] == ' ') {
			return row;
		}
	}
	return row;
}

int checkWLine(char mark, node cell) {
	//������ ���� winning line �� �� �� ���� mark�� ä�����ִٸ�  == ���� �̱�� �Ǵ� ���
	if (board[cell.pos[0][0]][cell.pos[0][1]] == mark &&
		board[cell.pos[1][0]][cell.pos[1][1]] == mark &&
		board[cell.pos[2][0]][cell.pos[2][1]] == mark &&
		board[cell.pos[3][0]][cell.pos[3][1]] == mark) {
		return 1000;
	}
}

int checkLLine(char mark, node cell) { //check lose line. ������ mark �� winning line �ִ°�?
							  // ������ ������ winning line �� �� �� ������ mark �� ä�����ִٸ� == ������ �̱�� �Ǵ� ���
	char opp = (mark == 'O') ? 'X' : 'O'; //opponent mark

	if (board[cell.pos[0][0]][cell.pos[0][1]] == opp &&
		board[cell.pos[1][0]][cell.pos[1][1]] == opp &&
		board[cell.pos[2][0]][cell.pos[2][1]] == opp &&
		board[cell.pos[3][0]][cell.pos[3][1]] == opp) {
		return -1000;
	}
}

int getScore(char mark) {
	// �� node (status) �� ���� ���� ��ȯ.

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


	//���� �ξ�� �ϴ� mark �� O ���� X ����?
	return (mark == 'O') ? (score_O - score_X) : (score_X - score_O);
}

int checkFull(int column) {
	if (colHeight(column) == BOARD_ROWS) return 1;
	else return 0;
}

int funcHeuristic(int ismax, char mark, int select, int depth, int level, int alpha, int beta) {
	// turn n���� �޸���ƽ�ϰ� ���� �Ǵ��ؼ� �� ��ȯ
	 //ismax = �ڽ� ���鿡 ���� maximize �� ���̶�� 1 (�� ����), minimize �� ���̶�� 0 (�� ����)

	int revDepth = level - depth + 1;
	int minimizing = 0;
	int maximizing = 1;
	int value;

	//select : � column �� ���ؼ� heuristic ������ ��ȯ�� ���ΰ�?
	if (checkFull(select) == 1) return -3000; // column �� �� ���ִٸ� ���õ��� �ʵ��� �ּ��� ��.
	board[colHeight(select)][select] = mark; //board �� mark �ֺ� ���¿��� ���� ��ȯ�� ����.

	 //printf("\n%d ��° column ���� %d ��° ���� Ž���ϰڴ�. : %d", select, depth, getScore(mark)); //�����

	int score = getScore(mark);// evaluation table�� ����Ͽ� ��� �� ������ ���� �� alpha-beta pruning

	if (depth == level) {
		value = (ismax == 1) ? -score : score; //�ִ�ȭ �����, ���� �����̹Ƿ� score * (-1)
	}
	else {
		if (score == 1000 || score == -1000) {
			board[colHeight(select) - 1][select] = ' ';
			return (ismax == 1) ? -(score + revDepth) : (score + revDepth);
		}

		else {
			if (ismax == 1) { // �ִ�ȭ����� ��� (���� ������ ���) select max
				value = -1000 - PERFECT_DEPTH;
				for (int i = 0; i < BOARD_COLS; i++) {
					int temp = funcHeuristic(minimizing, (mark == 'O') ? 'X' : 'O', i, depth + 1, level, alpha, beta);
					if (temp != -3000) {
						value = max(value, temp);
						if (value >= beta) {
							//printf("\n value %d >= %d (beta) �̹Ƿ� sibling Ȯ�� ����",value,beta); //�����
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
					//printf("***temp �� %d ****", temp); //�����
					if (temp != -3000) {
						value = min(value, temp);
						if (value <= alpha) {
							//printf("value %d <= %d (alpha) �̹Ƿ� sibling Ȯ�� ����",value,alpha); //�����
							board[colHeight(select) - 1][select] = ' ';
							return value;

						}
						beta = min(beta, value);
					}
				}
			}
		}

	}

	board[colHeight(select) - 1][select] = ' '; // �Ʊ� �ֺô� �� ����
									 //printf("turn : %d column : %d, score = %d\n", turn, choice, score);
	return value; // ���� ����
}

int findMaxHeuristic(int depth, int level, char mark, int turn) { // �޸���ƽ�ϰ� ���� 7�� �������� �ְ����� ã�� �� col�� ��ȯ�ϴ� �Լ�
	int score[BOARD_COLS];
	int revDepth = level - depth + 1; //depth �� ���� �ٸ� ������ �ΰ��ϱ� ���� depth reverse.
	int max = 0;
	int minimizing = 0; //���� �����϶�, ���� ��忡 ���ؼ� minimize.

	printf(" Column     Score\n");
	// easy(level == 2) �� ��Ȳ�� �׻� level = 2 �� Ž��.
	// hard(level == 12) �� ��Ȳ�� AI �� mark 4�� �� �� ������ 7 �� �ո� Ž��.
	if (turn == 0) {
		for (int i = 0; i < BOARD_COLS; i++) {
			score[i] = 7 - abs(3 - i);
			printf("   %d        %d\n", i + 1, score[i]);
		}
	}
	else if (level != 2 && turn <= 7) { //AI �� mark �� �� �� �� �������� �� �� �ձ����� Ž��. level <= 7 �� ��
		for (int i = 0; i < BOARD_COLS; i++) {
			int reducedLevel = 7;
			score[i] = funcHeuristic(minimizing, mark, i, depth, reducedLevel, -1000 - revDepth, 1000 + revDepth);
			// ���� depth �鿡 ���ؼ� minimize �� ����̹Ƿ� pass minimizing parameter.
			printf("   %d        %d\n", i + 1, score[i]);
		}
	}
	else {
		// level == 2 �̰ų�, AI �� mark 5��° �� ������ 6 �� or 12 �� �ձ��� �� Ž��.
		//printf("(turn %d ����) %d �� �ձ��� ���ٺ��ھ��",turn, level);
		for (int i = 0; i < BOARD_COLS; i++) {
			score[i] = funcHeuristic(minimizing, mark, i, depth, level, -1000 - revDepth, 1000 + revDepth);
			// �� ó���� AI�� �δ� max�̹Ƿ� true, turn�� mark�� parameter�̰� i�� for������ 0~6���� ������
			printf("   %d        %d\n", i + 1, score[i]);
		}
	}

	for (int i = 0; i < BOARD_COLS; i++) { // �ְ����� ã��- ������ ��� ��� ���� ����� �ʺ��� �����ϰ� tie-breaking
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
	for (int i = 0; i < BOARD_COLS; i++) { // �ְ����� ã��- ������ ��� ��� ���� ����� �ʺ��� �����ϰ� tie-breaking
		if (score[i] < score[min]) min = i;
		else if (score[i] == score[min]) {
			if (abs(3 - min) <= abs(3 - i)) min = i;
		}
	}

	if (turn == 0) {
		printf("\nù turn �� ��� winning line ���� ���� column �� �����Ѵ�. ����� ����� column �� �����Ѵ�.\n�ٸ� 4�� ������ �� �����Ƿ� 3, 5 �߿� �ϳ��� �����Ѵ�.\n");
	}
	else if (score[maxcol] >= 1000) {
		int tempLevel = (turn <= 7 && level == 12) ? 7 : level;

		printf("\nsearch ���, AI�� %d �� ���� Ž���ϴ� �������� AI �� �̱�� �Ǵ� ��찡 ���ͼ� %d �� ����.", tempLevel - (score[maxcol] - 1000) + 1, maxcol + 1);
	}
	else if (score[min] <= -1000) {

		int tempLevel = (turn <= 7 && level == 12) ? 7 : level;

		printf("\nsearch ���, AI�� %d �� ���� Ž���ϴ� �������� ������ �̱�� �Ǵ� ��찡 ���ͼ� %d �� ���þ���.", tempLevel - (-1000 - score[min]) + 1, min + 1);

		if (score[maxcol] > -1000) {
			board[colHeight(maxcol)][maxcol] = mark; //���õ� col �� ���� �ΰ� ���� �� ���� ���

			int tempScore = sequenceScore(mark);
			int oppScore = sequenceScore(opp);

			printf("\n���� �������� ���� �ξ��� ��, ���ӵ� �� ������ ���� ������ %d(%c �� ����) - %d(%c �� ����) = \"%d\" �̴�.", tempScore, mark, oppScore, opp, tempScore - oppScore);
			printf("\n �̷��� ��İ� winning Line ������ ���̸� �Բ� ����ϸ� \"%d\" �� �ձ��� �� ���� ��\n \"Column %d\" �� ���� ���� ������ �����ϹǷ� �̸� ����", tempLevel, maxcol + 1);

			board[colHeight(maxcol) - 1][maxcol] = ' ';

		}
	}
	else {
		board[colHeight(maxcol)][maxcol] = mark; //���õ� col �� ���� �ΰ� ���� �� ���� ���

		int tempScore = sequenceScore(mark);
		int oppScore = sequenceScore(opp);

		int tempLevel = (turn <= 7 && level == 12) ? 7 : level;

		printf("\n���� �������� ���� �ξ��� ��, ���ӵ� �� ������ ���� ������ %d(\'%c\' �� ����) - %d(\'%c\' �� ����) = \"%d\" �̴�.", tempScore, mark, oppScore, opp, tempScore - oppScore);
		printf("\n (�� ������� ���� �� + winning Line ���� ����)�� ���� ���� �̿��� \"%d\" �� �ձ��� �� ���� ��\n \"Column %d\" �� ���� ���� ������ �����ϹǷ� �̸� ����", tempLevel, maxcol + 1);

		board[colHeight(maxcol) - 1][maxcol] = ' ';
	}
	printf("\n");
}

int getHint(int yes, int depth, int level, char mark, int turn) {
	int score[BOARD_COLS];
	//parameter: depth, mark
	int minimizing = 0;
	while (yes >= 0) { //hint�� �����Ҷ������� ��밡��
		for (int i = 0; i < BOARD_COLS; i++) {
			score[i] = funcHeuristic(minimizing, mark, i, depth, level, -1000 - (level - depth + 1), 1000 + (level - depth + 1));
		}

		int max = 0;
		for (int i = 0; i < BOARD_COLS; i++) { // �ְ����� ã��- ������ ��� ��� ���� ����� �ʺ��� �����ϰ� tie-breaking
			if (turn == 0 && i == 3) continue;
			if (score[i] > score[max]) max = i;
			else if (score[i] == score[max]) {
				if (abs(3 - max) >= abs(3 - i)) max = i;
			}
		}
		return max; // column ����
	}
	return -1;
}

int sequenceScore(char mark) {
	// �Ϸķ� ���̴� ���� ������ �������� ������ �� ���� �ο�.
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