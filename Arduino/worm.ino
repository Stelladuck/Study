// 아두이노 기판에서 기동하며 시위치로 조작하는 지렁이 게임

#include <LiquidCrystal.h>

// Initialize the library by associating any needed LCD interface pin with the arduino pin number it is connected to.
// "rs" is register pin which controls where in the LCD's memory one's writing data to.
// "en" is enable pin which enables writing to the registers.
// "d4" "d5" "d6" "d7" are data pins.
// rs = 4 && en = 6 && d4 = 11 && d5 = 12 && d6 = 13 && d7 = 14

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2; LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int const switchPinL = A1; int const switchPinR = A0; int switchValL; int switchValR;
long min_L = 0;  long max_L = 255;
long min_R = 767;long max_R = 1023;          

// BYTES FOR GRAPHICAL VIEW OF LETTERS
byte LT[8] = { B00111, B01111, B11111, B11111, B11111, B11111, B11111, B11111 };	// Curvy LEFT TOP of letter.
byte LL[8] = { B11111, B11111, B11111, B11111, B11111, B11111, B01111, B00111 };	// Curvy LEFT LOW of letter.
byte RT[8] = { B11100, B11110, B11111, B11111, B11111, B11111, B11111, B11111 };	// Curvy RIGHT TOP of letter.
byte RL[8] = { B11111, B11111, B11111, B11111, B11111, B11111, B11110, B11100 };	// Curvy RIGHT LOW of letter.
byte UB[8] = { B11111, B11111, B11111, B00000, B00000, B00000, B00000, B00000 };	// UPPER BLOCK of letter.
byte MB[8] = { B11111, B11111, B11111, B00000, B00000, B00000, B11111, B11111 };	// MIDDLE BLOCK of letter.
byte LB[8] = { B00000, B00000, B00000, B00000, B00000, B11111, B11111, B11111 };	// LOWER BLOCK of letter.
byte FB[8] = { B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111 };	// FULL BLOCK of letter.
byte TU[8] = { B11111, B11111, B11111, B01110, B01110, B01110, B01110, B01110 };	// THIN UPPER BLOCK of letter
byte TM[8] = { B11111, B11111, B11111, B01110, B01110, B11111, B11111, B11111 };	// THIN BOTTOM BLOCK of letter
byte TL[8] = { B01110, B01110, B01110, B01110, B01110, B11111, B11111, B11111 };	// THIN BOTTOM BLOCK of letter
byte TB[8] = { B01110, B01110, B01110, B01110, B01110, B01110, B01110, B01110 };	// THIN BLOCK of letter.
byte LS[8] = { B00111, B01111, B11111, B11111, B11111, B11111, B01111, B00111 };	// Half of LEFT SPHERE of letter.
byte RS[8] = { B11100, B11110, B11111, B11111, B11111, B11111, B11110, B11100 };	// Half of RIHGT SPHERE of letter.
byte EB[8] = { B00000, B00000, B00000, B00000, B00000, B00000, B00000, B00000 };	// EMPTY BLOCK of letter

// VARIABLES
bool pixel[16][80];	// Predefine the pixel array whose elements are having boolean value.
byte my_block[8];	// Predefined byte in order to save one full block.
bool triggered;		// Triggering for LCD to display worm.
int collected;		// The number of collected apples will be saved here.
long pr,pc;		// Indicates pixel row and pixel column of apple.

int game_speed;		// Determine the game speed.
bool game_started;	// Determine whether the game has been started.
bool game_finish;	// Determine whether the game is over.
bool dir_changed;	// Determine whether the direction of the head is changed
int prev_dir;		// Stores the previous direction of the head.
int new_dir;		// Stores the current direction of the head.
int i,j;          	// This variables will be used as a iteration number.

int switch_state_L = 0;	// This variable stores the digital value of the left button.
int switch_state_R = 0;	// This variable stores the digital value of the right button.

/* Nested struct with pointer is necessary, since the worm moves dynamically for every time incident,
it is difficult to express through an array. Moreover, due to the limitation of memory within the Arudino integrated circuit,
saving all values for every time incident requires uncessary memory allocation which leads to the lack of memory.*/

struct POSITION {
	int row;	// For Arduino Uno lcd, there are total 16 pixel rows.
	int col;	// For Arduino Uno lcd, there are total 80 pixel columns.
	int dir;	// (0 : RIGHT) && (1 : ABOVE) && (2 : LEFT ) && (3 : BELOW).
	// Purpose of this nested struct (*next) is to save an information of the pixel next to the tail.
	struct POSITION *next;
	/* Pictorial example:
	┌────┐┌────┐┌────┐┌────┐┌────┐┌────┐┌────┐┌────┐┌────┐
	│tail││    ││    ││    ││    ││    ││    ││    ││head│
	└────┘└────┘└────┘└────┘└────┘└────┘└────┘└────┘└────┘
	First 'tail' is copied to dummy struct pointer 'p'. Then the second pixel (pixel next to tail) will be saved into 'p->next'.
	Then third will be saved as '(p->next)->next' and so on. For the last '((p->next)-> ... )->next' should be empty and the last 'p' 		should be the head, since head moves independently and no more body element in front of the head. */
}; typedef POSITION position;	// Type declaration : POSITION.
	position *head;		// Position of head (pointer).
	position *tail;		// Position of tail (pointer).


// FUNCTION : GENERATING INITIAL WORM WHEN GAME IS STARTED
void Initial_Worm (int n) {	// The initial length of the worm is 'n'.
	for (i = 0; i < 16; i++) { for (j = 0; j < 80; j++) { pixel[i][j] = false; }}	// Initialize all the pixel to be false.
	position *p;	// dummy struct pointer 'p'.
	position *q;	// dummy struct pointer 'q'.
	tail = (position*) malloc( sizeof( position ) );				// Allocate the memory space for tail.
	// We want to have the initial worm (almost) in the center of the LCD.
	tail->row = 7;	// Initial pixel row for tail.
	tail->col = 40;	// Initial pixel col for tail.
	tail->dir = 2;	// Initial direction is 'left'.
	pixel[tail->row][tail->col] = true;	// Switch on the pixel.
	p = tail;	// Copy all the infomration of tail into dummy 'p'.
	for (i = 0; i < n - 1; i++) {	// Turn on the pixel from tail to head.
		q = (position*) malloc( sizeof( position ) );	// Allocate memory space for dummy struct pointer 'q'.
		q->row = p->row;		// Let the worm be heading towards the left Initially. Hence row is equivalent.
		q->col = p->col - 1;		// Let the worm be heading towards the left Initially. Hence column has to decrease by one.
		q->dir = p->dir;		// Let the worm be heading towards the left Initially. Hence direction is conserved.
		pixel[q->row][q->col] = true;	// Switch on the corresponding pixel.
		p->next = q;			// Since 'q' contains the information of the pixel next to 'p', copy the information.
		p = q;				// Save 'q' as 'p'
	}
	if (n > 1) {			// When the length of worm is larger than one; after escaping the above for loop:
		p->next = NULL;		// Make 'p.next' be empty 
		head  = p;		// The last 'p' will be set as 'head' of the worm.
	} else {			// When the length of worm is one; after escaping the above for loop:
		tail->next = NULL;	// Make 'tail.next' be empty, so that there is no consecutive pixel after tail.
		head = tail;		// Let 'head' and 'tail' be equivalent.
	}
}

// FUNCTION : EXTEND THE WORM WHEN THE APPLE IS EATEN BY WORM.
void Stretch_Worm() {
	position *p; p = (position*) malloc( sizeof( position ) );	// Define dummy point struct 'p'.
	p->row = tail->row; p->col = tail->col; p->dir = tail->dir;	// Update dummy struct point.
	p->next = tail;	// Save current tail as a second last element of the body.
	tail = p;	// Tail is pushed back.
}

// FUNCTION : GENERATE APPLE.
void Apple_Generator() {
	position *p; p = tail;
	bool new_apple = true;
	while (new_apple) {
		pr = random(16);	// Generate apple on random pixel row.
		pc = random(80);	// Generate apple on random pixel column.
		new_apple = false;	// Since apple is generated, switch off 'new_apple' for considering another case.
		while (p->next != NULL && !new_apple)  {	// When it is not the head and the apple is not generated:
			// So when the tail reaches to 'pixel[pr][pc]', it generates new apple.
			if (p->row == pr && p->col == pc) new_apple = true;
			p = p->next;
		}
	} if (collected < 11 && game_started) Stretch_Worm();
}

// FUNCTION : MOVEMENT OF HEAD AND DETECT COLLISIONS.
void Head_Movement() {
	switch(head->dir) {
		case 0: head->col++; break;	// 0 is for right. Hence column increases.
		case 1: head->row++; break;	// 1 is for above. Hence row increases.
		case 2: head->col--; break;	// 2 is for left. Hence column decreases.
		case 3: head->row--; break;	// 3 is for below. Hence row decreases.
		default : break;
	}
	if (head->col >= 80) { game_finish = true; }	// Wall collision : If worm head hits the most right of lcd, then game finishes.
	if (head->col < 0) { game_finish = true; }	// Wall collision : If worm head hits the most left of lcd, then game finishes.
	if (head->row >= 16) { game_finish = true; }	// Wall collision : If worm head hits the most bottom of lcd, then game finishes.
	if (head->row < 0) { game_finish = true; }	// Wall collision : If worm head hits the most above of lcd, then game finishes.

	position *p; p = tail;	// Copy all the infomration of tail into dummy 'p'.
	// Detect self collision
	while (p != head && !game_finish) { if (p->row == head->row && p->col == head->col) { game_finish = true; } p = p->next; }
	// Case when game is over
	if (game_finish) { Game_Over(); } else {
		// If game is not over
		pixel[head->row][head->col] = true; 	// The pixel of the position of head is switched on.
		if (head->row == pr && head->col == pc) { collected++; Apple_Generator(); }	// And checks whether head has eaten an apple.
	}
}

// FUNCTION : TOTAL MOVEMENT OF WORM FOR EACH INCIDENT OF TIME.
void Worm_Movement() {
	position *p; p = tail;		// Copy all the infomration of tail into dummy 'p'.
	// The main idea is to call all the information of 'p->next'. 
	pixel[p->row][p->col] = false;	// Since worm moves, the pixel corresponding to the 'tail' has to be turned off.
	while (p->next != NULL) {	// While 'p->next' is not empty (if it is empty, which is then 'head')
		p->row = p->next->row;	// Since we need next pixel information, call 'p->next->row' and save it to 'p->row'.
		p->col = p->next->col;	// Since we need next pixel information, call 'p->next->col' and save it to 'p->col'.
		p->dir = p->next->dir;	// Since we need next pixel information, call 'p->next->dir' and save it to 'p->dir'.
		p = p->next;		// Lastly we update 'p' as 'p->next' in order to call the information of 'p->next->next'.
	} Head_Movement();		// Move head. The function also detects collision as well.
}

// FUNCTION : EXPOSE WORM ON THE LCD.
void Expose_To_LCD () {
	int counter = 0;		// Purpose of this 'counter' is to assign name to 'my_block'.
	if (!game_finish) {
		pixel[pr][pc] = true;	// The position of apple is set to be true.
		for(int r = 0; r < 2; r++) {
			for(int c = 0; c < 16; c++) { triggered = false;	// The LCD Exposure is set to be false at first.
				for(int i = 0; i < 8; i++) { byte b = B00000;	// Initital byte
					// If the pixel is switched on, the exposure is triggered.
					if (pixel[r*8+i][c*5+0]) { b+=B10000; triggered = true; }
					// If the pixel is switched on, the exposure is triggered.
					if (pixel[r*8+i][c*5+1]) { b+=B01000; triggered = true; }
					// If the pixel is switched on, the exposure is triggered.
					if (pixel[r*8+i][c*5+2]) { b+=B00100; triggered = true; }
					// If the pixel is switched on, the exposure is triggered.
					if (pixel[r*8+i][c*5+3]) { b+=B00010; triggered = true; }
					// If the pixel is switched on, the exposure is triggered.
					if (pixel[r*8+i][c*5+4]) { b+=B00001; triggered = true; }
					my_block[i] = b;	// Save it as a block.
				}
				if (triggered) {	// When it is allowed to expose on LCD panel:
					lcd.createChar(counter, my_block);	// Create character.
					lcd.setCursor(c,r);		// Set cursor on lcd panel.
					lcd.write(byte(counter));	// Write 
					counter++;	// Increase 'counter' to set another name for next 'my_block' character.
				} else { lcd.setCursor(c,r); lcd.write(254); }	// Fill the empty space. 
			}
		} 
	}
}

// FUNCTION : CLEAR ALL THE MEMORY
void Deallocate_All() {
	position *p;	// dummy struct pointer.
	position *q;	// dummy struct pointer.
	p = tail;	// Copy all the infomration of tail into dummy 'p'.
	while ( p != NULL ) {
		q = p;		// Ready to clear the memory allocation for p.
		p = p->next;	// Copy p.next into p.
		free(q);	// Clear the memory allocation of p.
	} tail = NULL; head = tail;	// After escaping loop, make sure the tail and the head are empty.
}

// FUNCTION :: INITIALIZING THE NECESSARY COMPONENTS FOR SETUP FUNCTION
void Initialization() {
	game_finish = false;	// 'game_finish' is set false initially.
	game_started = false;	// 'game_started; is set false initially.
	lcd.clear(); lcd.setCursor(0,0);

	lcd.createChar( 1 , LT );	lcd.createChar( 2 , LL );	lcd.createChar( 3 , RT );
	lcd.createChar( 4 , RL );	lcd.createChar( 5 , UB );	lcd.createChar( 6 , MB );
	lcd.createChar( 7 , LB );	lcd.createChar( 8 , FB );	lcd.createChar( 9 , TU );
	lcd.createChar( 10 , TM );	lcd.createChar( 11, TU );	lcd.createChar( 12, TB );
	lcd.createChar( 13, LS );	lcd.createChar( 14, RS );	lcd.createChar( 15, EB );
}

// FUNCTION :: GENERATING STARTING BACKGROUND
void Starting_Background() {
	// It will show the word WORM in capital letter.
	lcd.setCursor(0,0);	// set cursor to column 0, line 0 (first row)
	lcd.write(8);	lcd.write(10);	lcd.write(8);	lcd.write(15);	// Above half of 'W'
	lcd.write(1);	lcd.write(5);	lcd.write(3);	lcd.write(15);	// Above half of 'O'
	lcd.write(8);	lcd.write(6);	lcd.write(14);	lcd.write(15);	// Above half of 'R'
	lcd.write(8);	lcd.write(11);	lcd.write(8);	lcd.write(15);	// Above half of 'M'
	lcd.setCursor(0, 1);	// set cursor to colum 0, line 1 (second row)
	lcd.write(2);	lcd.write(12);	lcd.write(4);	lcd.write(15);	// Below half of 'W'
	lcd.write(2);	lcd.write(7);	lcd.write(4);	lcd.write(15);	// Below half of 'O'
	lcd.write(8);	lcd.write(15); lcd.write(3);	lcd.write(15);	// Below half of 'R'
	lcd.write(8);	lcd.write(10);	lcd.write(8);	lcd.write(15);	// Below half of 'M'
}

void Before_Game_Start() {
	game_finish = false; game_started = false;	// Initialize variables.
	lcd.clear(); lcd.setCursor(0,0);		// Initialize LCD.
	Initialization(); Starting_Background();	// Initialize opening background.
	collected = 0; game_speed = 5;			// Initialize game environment.
	Initial_Worm(3);				// Initialize worm with worm length 3.
}

void Game_Over() { delay(1000);		// Some delay.
	lcd.clear(); Deallocate_All();	// Clear LCD and deallocate the memory.
	lcd.setCursor(3,0); lcd.print("You die!");			// You die.
	lcd.setCursor(3,1); lcd.print("Score: "); lcd.print(collected);	// Show the number of collected apples.
	delay(1000);	// Some delay,
}

// FUNCTION :: ASSIGNING DIRECTION VIA POTMETER
int get_dir( int swtich_state_L, int switch_state_R , int prev_dir ) {
  int k = prev_dir;	// Dummy variable to save the new direction.
  if ( switch_state_L == LOW && switch_state_R == HIGH ) { k = ( k + 1 ) % 4; }
  else if ( switch_state_L == HIGH && switch_state_R == LOW ) { k = (k - 1) % 4; }
  else { k = prev_dir; }
  return k;
}

/* ------------------------------------------------------------------------------------------------------------------------------------------------------- */

void setup() {
	lcd.begin(16, 2);
	Initialization();
	Before_Game_Start();
	delay(1500);
	Serial.begin(9600);
}

void loop() {
	game_started = true;
	prev_dir = 2;	// Initially it is moving in left.
	// If game is started and game is not finished:
	while (game_started && !game_finish) {
		dir_changed = false;	// Direction has not been changed initially.
    switchValL = analogRead(switchPinL); Serial.print(switchValL);
    switchValR = analogRead(switchPinR); Serial.println(switchValR);
    switch_state_L = digitalRead(7);
    switch_state_R = digitalRead(8);
		new_dir = get_dir(switch_state_L,switch_state_R, prev_dir); 	// Get the new direction depends on potVal.
		if (new_dir != prev_dir) {
			delay(100);		// Considering a debouncing time.
			head->dir = new_dir;	// Update direction.
			prev_dir = new_dir;
			dir_changed = true;
			delay(100); Worm_Movement(); Expose_To_LCD();	// Appear on LCD.
		} else if (!dir_changed) { delay(100); Worm_Movement(); Expose_To_LCD(); }
	}
	// If game is over:
	if (game_finish) { Game_Over(); Before_Game_Start(); }
}
