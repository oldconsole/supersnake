
#include "snes.h"

extern unsigned char logo_tiles[];
extern unsigned char logo_palette[];
extern unsigned char logo_map[];
extern unsigned char level_tiles[];
extern unsigned char level1_palette[];
extern unsigned char level2_palette[];
extern unsigned char level3_palette[];
extern unsigned char level4_palette[];
extern unsigned char level5_palette[];
extern unsigned char level6_palette[];
extern unsigned char level7_palette[];
extern unsigned char level8_palette[];
extern char level1_map_csv[];
extern char level2_map_csv[];
extern char level3_map_csv[];
extern char level4_map_csv[];
extern char level5_map_csv[];
extern char level6_map_csv[];
extern char level7_map_csv[];
extern char level8_map_csv[];
extern char level1_collision_csv[];
extern char level2_collision_csv[];
extern char level3_collision_csv[];
extern char level4_collision_csv[];
extern char level5_collision_csv[];
extern char level6_collision_csv[];
extern char level7_collision_csv[];
extern char level8_collision_csv[];

#define TILES_HORZ 32
#define TILES_VERT 32

#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3

#define TILE_SNAKE 32
#define TILE_FOOD 33

#define TILE_EMPTY 0
#define TILE_FULL 1

#define TRUE 1
#define FALSE 0

#define PHASE_INIT 0
#define PHASE_LOGO 10
#define PHASE_INITGAME 20
#define PHASE_MOVE 30
#define PHASE_COLLIDE 40
#define PHASE_NEXTLEVEL 50

#define BLINK_COUNT 10

#define LEVEL_FOOD_COUNT 20

static int g_lives = 3;
static long long g_points = 0ll;
static int g_level = 0;
static unsigned char *g_map = NULL;
static unsigned char *g_collision = NULL;
static char g_snakeDirection = DOWN;
static int g_phase = PHASE_LOGO;
static int g_foodX = 0;
static int g_foodY = 0;
static char g_foodDraw = FALSE;
static char g_levelFoodEaten = 0;

typedef struct Level_Tag {
	unsigned char *tiles;
	unsigned char *palette;
	char *map_csv;
	char *collision_csv;
}Level;

Level g_levels[] = {
					{level_tiles, level1_palette, level1_map_csv, level1_collision_csv}, 
					{level_tiles, level2_palette, level2_map_csv, level2_collision_csv},
					{level_tiles, level3_palette, level3_map_csv, level3_collision_csv},
					{level_tiles, level4_palette, level4_map_csv, level4_collision_csv},
					{level_tiles, level5_palette, level5_map_csv, level5_collision_csv},
					{level_tiles, level6_palette, level6_map_csv, level6_collision_csv},
					{level_tiles, level7_palette, level7_map_csv, level7_collision_csv},
					{level_tiles, level8_palette, level8_map_csv, level8_collision_csv}
				};

typedef struct SnakeSection_Tag {
	struct SnakeSection_Tag *pPrev;
	struct SnakeSection_Tag *pNext;
	int x;
	int y;
	unsigned int tileBelow;	
}SnakeSection;

SnakeSection *g_pSnakeSections = NULL;

typedef struct Font_Tag {
	char *chars;
	int numChars;
	int startIndex;
}Font;

Font g_fontNormal = {"0123456789abcdefghijklmnopqrstuvwxyz", 33, 64};

const char *LongLongToStr(long long val) {
	static char str[16];
	int numbers[10];
	int minus = 0;
	int i;
	int j = 0;
	long long r = 1000000000ll;	
	if (val < 0ll) {
		minus = 1;
		val = -val;
	}
	memset(numbers, 0, sizeof(int) * 10);
	for (i = 0; i < 10; i++) {
		while(val >= r) {
			numbers[j]++;
			val -= r;
		}
		r /= 10ll;
		j++;
	}
	i = 0;
	if (minus)
		str[i++] = '-';
	for (j = 0; j < 10; j++)
		str[i + j] = numbers[j] + '0';
	str[i + j] = '\0';
	return str;
}

int StrToInt(const char *str) {
	int len;
	int i;
	int num = 0;
	int base = 1;
	char minus = 0;
	if (str == NULL)
		return 0;
	while (*str == ' ' || *str == '\t')
		str++;
	if (*str == '\0')
		return 0;
    if (*str == '-') {
		minus = 1;
		str++;
	}
	len = strlen(str);
	for (i = len - 1; i >= 0; i--) {
		if (str[i] < '0' || str[i] > '9')
			break;
		num += (str[i] - '0') * base;
		base *= 10;
	}
	return (minus) ? -num : num;
}

void WriteHeader(Font *pFont, int x, int y, const char *str) {
	int startSprite = 0;
	int i;
	int spriteIndex = -1;
	int j;	
	int len;
	if (str == NULL)
		return;
	len = strlen(str);
	if (len == 0)
		return;
	for (i = 0; i < len; i++) {
		spriteIndex = -1;		
		switch (str[i]) {
			case '\n':
				y += 8;
				continue;
			case ' ':
			case '\t':
				x += 8;
				continue;
			default:
				for (j = 0; j < pFont->numChars; j++) {
					if (str[i] == pFont->chars[j]) {
						spriteIndex = j;
						startSprite++;
						break;
					}
				}
				break;
		}
		if (spriteIndex != -1) {
			setsprite(startSprite, x, y, pFont->startIndex + spriteIndex, 31);
			x += 8;
		}
	}
}

unsigned int MapGetXY(int xp, int yp) {
	return (g_map[yp * 64 + xp * 2] | (g_map[yp * 64 + xp * 2 + 1] << 8));
}

void MapSetXY(int xp, int yp, unsigned int tile) {
	g_map[yp * 64 + xp * 2] = (unsigned char)(tile & 0xff);
	g_map[yp * 64 + xp * 2 + 1] = (unsigned char)((tile & 0x300) >> 8);
}

void MapCreate(unsigned char *csv) {
    int tile = 0;
	int xp;
	int yp;
	char num[8];
	int i;
	g_map = (unsigned char *)malloc(sizeof(unsigned char) * 64 * 32);
	for (yp = 0; yp < 32; yp++) {
		for (xp = 0; xp < 64; xp += 2) {
			i = 0;
			while (*csv != ',') {
				num[i++] = *csv;
				csv++;
			}
			csv++;		
			num[i] = '\0';
			tile = StrToInt(num);
			g_map[yp * 64 + xp] = (unsigned char)(tile & 0xff);
			g_map[yp * 64 + xp + 1] = (unsigned char)((tile & 0x300) >> 8);			
		}
	}
}

void MapRelease() {
	if (g_map != NULL) {
		free(g_map);
		g_map = NULL;
	}
}

unsigned int CollisionGetXY(int xp, int yp) {
	return g_collision[yp * 32 + xp];
}

void CollisionCreate(unsigned char *csv) {
	int xp;
	int yp;
	char num[8];
	int i;
	g_collision = (unsigned char *)malloc(sizeof(unsigned char) * 32 * 32);
	for (yp = 0; yp < 32; yp++) {
		for (xp = 0; xp < 32; xp++) {
			i = 0;
			while (*csv != ',') {
				num[i++] = *csv;
				csv++;
			}
			csv++;		
			num[i] = '\0';
			g_collision[yp * 32 + xp] = (unsigned char)StrToInt(num);
		}
	}
}

void CollisionRelease() {
	if (g_collision != NULL) {
		free(g_collision);
		g_collision = NULL;
	}
}

void SnakeCreate(int length) {
	SnakeSection *head;
	SnakeSection *section;
	SnakeSection *sectionPrev;
	int i;
	head = (SnakeSection *)malloc(sizeof(SnakeSection));
	if (g_level == 6) {
		head->x = TILES_HORZ / 2 + TILES_HORZ / 4;
		head->y = TILES_VERT / 2 + TILES_VERT / 8;		
	}
	else {
		head->x = TILES_HORZ / 2;
		head->y = TILES_VERT / 2;		
	}
	head->tileBelow = MapGetXY(head->x, head->y);
	head->pNext = NULL;
	head->pPrev = NULL;
	g_pSnakeSections = head;
	sectionPrev = head;
	for (i = 0; i < length; i++) {
		section = (SnakeSection *)malloc(sizeof(SnakeSection));
		section->x = sectionPrev->x;
		section->y = sectionPrev->y - 1;
		section->tileBelow = MapGetXY(section->x, section->y);
		section->pNext = NULL;
		section->pPrev = sectionPrev;
		sectionPrev->pNext = section;
		sectionPrev = section;
	}
}

void SnakeRelease() {
	SnakeSection *section = g_pSnakeSections;
	SnakeSection *sectionNext = section->pNext;
	while (section) {
		free(section);
		section = sectionNext;
		sectionNext = section->pNext;
	}
	g_pSnakeSections = NULL;
}

char SnakeCheckCollision() {
	SnakeSection *head = g_pSnakeSections;
	SnakeSection *section;
	if (CollisionGetXY(head->x, head->y) != TILE_EMPTY)
		return TRUE;
	section = head->pNext;
	while (section) {
		if (section->x == head->x && section->y == head->y)
			return TRUE;
		section = section->pNext;
	}
	return FALSE;
}

char SnakeCheckFoodCollision() {
	SnakeSection *head = g_pSnakeSections;
	if (head == NULL)
		return FALSE;
	if (head->x == g_foodX && head->y == g_foodY)
		return TRUE;
	return FALSE;
}

void SnakeMove() {
	SnakeSection *section = g_pSnakeSections;
	int oldx;
	int oldy;
	int oldxtemp;
	int oldytemp;
	unsigned int oldtile;
	unsigned int oldtiletemp;
	while (section) {
		if (section->pPrev == NULL) {
			oldx = section->x;
			oldy = section->y;
			oldtile = section->tileBelow;
			switch (g_snakeDirection) {
				case LEFT:
					section->x--;
					break;
				case RIGHT:
					section->x++;
					break;
				case UP:
					section->y--;
					break;
				case DOWN:		
					section->y++;
				default:
					break;
			}
			section->tileBelow = MapGetXY(section->x, section->y);
		}
		else {
			oldxtemp = section->x;
			oldytemp = section->y;
			oldtiletemp = section->tileBelow;
			section->x = oldx;
			section->y = oldy;
			section->tileBelow = oldtile;
			oldx = oldxtemp;
			oldy = oldytemp;
			oldtile = oldtiletemp;
		}
		section = section->pNext;
	}
}

void SnakeErase() {
	SnakeSection *section = g_pSnakeSections;
	while (section) {
		MapSetXY(section->x, section->y, section->tileBelow);
		section = section->pNext;
	}
}

void SnakeDraw() {
	SnakeSection *section = g_pSnakeSections;
	while (section) {
		MapSetXY(section->x, section->y, TILE_SNAKE);
		section = section->pNext;
	}
}

void SnakeGrow(int length) {
	SnakeSection *foot = g_pSnakeSections;
	SnakeSection *section;
	SnakeSection *sectionPrev;
	int i;
	while (foot->pNext)
		foot = foot->pNext;
	sectionPrev = foot;
	for (i = 0; i < length; i++) {
		section = (SnakeSection *)malloc(sizeof(SnakeSection));
		section->x = foot->x;
		section->y = foot->y;
		section->tileBelow = foot->tileBelow;
		section->pNext = NULL;
		section->pPrev = sectionPrev;
		sectionPrev->pNext = section;
		sectionPrev = section;
	}
}

int SnakeLength() {
	SnakeSection *section = g_pSnakeSections;
	int length = 0;
	while (section) {
		length++;
		section = section->pNext;
	}
	return length;
}

void FoodPlot() {
	SnakeSection *section;
	char found;
	while(1) {
		g_foodX = rand() % TILES_HORZ;
		g_foodY = rand() % TILES_VERT;
		if (CollisionGetXY(g_foodX, g_foodY) != TILE_EMPTY)
			continue;
		found = 0;
		section = g_pSnakeSections;
		while (section) {
			if (g_foodX == section->x && g_foodY == section->y) {
				found = 1;
				break;
			}
			section = section->pNext;
		}
		if (!found)
			break;
	}
	setsprite(0, g_foodX * 8, g_foodY * 8, TILE_FOOD, 31);
}

void Render() {
	char str[64];
	setmap(0, g_map);
	sprintf(str, "level %d    lives %d    %s", g_level + 1, g_lives, LongLongToStr(g_points));
	WriteHeader(&g_fontNormal, 0, 0, str);							
}

int main() {
	char drawSnake = 0;
	int blinked = 0;
	int js;
	int i;
	snesc_init();
	enablescreen();
	while(1) {
		switch (g_phase) {
			case PHASE_LOGO:
				g_lives = 3;
				g_points = 0;
				g_level = 0;			
				settiles(0, logo_tiles, 32768);
				setpalette(logo_palette);
				setmap(0, logo_map);
				for (i = 0; i < 64; i++)
					setsprite(i, 0, 0, 0, 31);
				g_phase++;
				break;
			case PHASE_LOGO + 1:
				js = getjoystatus(0);
				if (js & START_BUTTON)
					g_phase = PHASE_INITGAME;
				break;
			case PHASE_INITGAME:
				MapRelease();
				MapCreate(g_levels[g_level].map_csv);
				CollisionRelease();
				CollisionCreate(g_levels[g_level].collision_csv);				
				settiles(0, g_levels[g_level].tiles, 4096);
				setpalette(g_levels[g_level].palette);
				SnakeRelease();
				SnakeCreate(4);				
				g_snakeDirection = DOWN;
				g_levelFoodEaten = 0;
				g_phase = PHASE_MOVE;
				FoodPlot();
				Render();
				break;
			case PHASE_MOVE:
				js = getjoystatus(0);
				if (g_snakeDirection != LEFT && js & RIGHT_BUTTON)
					g_snakeDirection = RIGHT;
				else if (g_snakeDirection != RIGHT && js & LEFT_BUTTON)
					g_snakeDirection = LEFT;
				else if (g_snakeDirection != DOWN && js & UP_BUTTON)
					g_snakeDirection = UP;
				else if (g_snakeDirection != UP && js & DOWN_BUTTON)
					g_snakeDirection = DOWN;
				SnakeErase();
				SnakeMove();
				SnakeDraw();
				if (SnakeCheckCollision()) {
					blinked = 0;
					drawSnake = 1;
					g_phase = PHASE_COLLIDE;
					break;
				}
				if (SnakeCheckFoodCollision()) {
					FoodPlot();
					SnakeGrow(4);
					g_points += SnakeLength() * 10;
					g_levelFoodEaten++;
				}
				if (g_levelFoodEaten >= LEVEL_FOOD_COUNT) {
					g_phase = PHASE_NEXTLEVEL;
					break;
				}
				Render();
				break;
			case PHASE_COLLIDE:
				SnakeErase();
				if (drawSnake)
					SnakeDraw();
				drawSnake = !drawSnake;
				blinked++;
				if (blinked >= BLINK_COUNT) {
					SnakeErase();
					SnakeRelease();
					SnakeCreate(4);				
					g_lives--;
					if (g_lives > 0) {
						g_snakeDirection = DOWN;
						g_phase = PHASE_MOVE;
						break;
					}
					g_phase = PHASE_LOGO;
				}
				Render();				
				break;
			case PHASE_NEXTLEVEL:
				g_level++;
				if (g_level == 8)
					g_level = 0;
				g_phase = PHASE_INITGAME;
				break;
		}
		resettimer();
		clearjoy(0);
		delay(5);
		sync(1);
	}
}