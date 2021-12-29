#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <thread>
#include <cmath>
#include <time.h>
#include <iostream>

#define DEG_TO_RAD (M_PI / 180)
#define PADDLE_RATIO 4.141732283464567


struct Point {
	int x;
	int y;
};

struct Point window_center;
int window_width;
int window_height;

int random(int min, int max) {
	srand(time(NULL));
	return min + (rand() % static_cast<int>(max - min + 1));
}

class Angle {
	public:
		int angle;
		
		Angle() {
			switch (random(1, 4)) {
				case 1:
					angle = 360 - random(15, 45) - 360;
					break;
				case 2:
					angle = 360 - random(135, 165) - 360; 
					break;
				case 3:
					angle = 360 - random(195, 225) - 360;
					break;
				case 4:
					angle =  360 - random(315, 345);
					break;
			}
		}
		
};

static Point get_coords_from_angle(Point start, int angle, int length) {
	Point end_coords;
	
	end_coords.x = start.x + round(length * cos(angle * DEG_TO_RAD));
	end_coords.y = start.y + round(length * -(sin(angle * DEG_TO_RAD)));

	return end_coords;
}

static bool within_5(int x, int y) {
	int z = y - x;
	return ((z < 5) && (z > -5));
}

void init(SDL_Window*& window, SDL_Surface*& screen_surface, SDL_Renderer*& renderer) {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_DisplayMode DM;
	SDL_GetCurrentDisplayMode(0, &DM);
	window_width = DM.w;
	window_height = DM.h;
	window_center.x = round(window_width/2);
	window_center.y = round(window_height/2);
	window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_ALLOW_HIGHDPI);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	screen_surface = SDL_GetWindowSurface(window);
}

class Paddle {
	public:
		SDL_Rect position;
		SDL_Texture* paddle;
		SDL_Renderer* renderer;
		Paddle(bool left_or_right, SDL_Renderer*& renderer_argument) {
			if (!left_or_right) {
				position.h = (int) (window_height / 20 * 2.5);
				position.w = (int) (position.h / PADDLE_RATIO);
				
				position.x = 0;
				position.y = (int) (window_center.y - (position.h / PADDLE_RATIO));
			}
			else {
				position.h = (int) (window_height / 20 * 2.5);
				position.w = (int) (position.h / PADDLE_RATIO);
				
				position.x = window_width - position.w;
				position.y = (int) (window_center.y - (position.h / PADDLE_RATIO));
			}
			
			renderer = renderer_argument;
			
			paddle = SDL_CreateTextureFromSurface(renderer, IMG_Load("Pong Paddle.png"));
			
			SDL_RenderCopy(renderer, paddle, NULL, &position);
		}
		
		void re_init(bool left_or_right) {
			if (!left_or_right) {
				position.h = (int) (window_height / 20 * 2.5);
				position.w = (int) (position.h / PADDLE_RATIO);
				
				position.x = 0;
				position.y = (int) (window_center.y - (position.h / PADDLE_RATIO));
			}
			else {
				position.h = (int) (window_height / 20 * 2.5);
				position.w = (int) (position.h / PADDLE_RATIO);
				
				position.x = window_width - position.w;
				position.y = (int) (window_center.y - (position.h / PADDLE_RATIO));
			}
			
			SDL_RenderCopy(renderer, paddle, NULL, &position);
		}
		
		void move(bool up_or_down) {
			if (!up_or_down) {
				if (!within_5(position.y, 0)){
					position.y -= 3;
				}
			}
			
			else {
				if (!within_5(position.y + position.h, window_height)){
					position.y += 3;
				}
			}
		}
		
		void render() {
			SDL_RenderCopy(renderer, paddle, NULL, &position);
		}
		
		~Paddle() {
			SDL_DestroyTexture(paddle);
			paddle = NULL;
		}
};

class Ball {
	public:
		SDL_Rect position;
		Angle angle;
		SDL_Renderer *renderer;
		SDL_Texture *ball;
		
		int velocity = 3;
		
		Ball(SDL_Renderer*& renderer_argument) {
			int ball_size = (int) (window_height / 20);
			
			position.x = window_center.x - (int)(ball_size/2);
			position.y = window_center.y - (int)(ball_size/2);
			position.w = ball_size;
			position.h = ball_size;
			
			renderer = renderer_argument;
			
			SDL_Surface* ball_surface = IMG_Load("Pong Ball.png");
			
			SDL_SetColorKey(ball_surface, true, SDL_MapRGB(ball_surface->format, 0, 0, 0));
			
			ball = SDL_CreateTextureFromSurface(renderer, ball_surface);
			
			SDL_RenderCopy(renderer, ball, NULL, &position);
		}
		
		void re_init() {
			int ball_size = (int) (window_height / 20);
			
			position.x = window_center.x - (int)(ball_size/2);
			position.y = window_center.y - (int)(ball_size/2);
			position.w = ball_size;
			position.h = ball_size;
			
			SDL_RenderCopy(renderer, ball, NULL, &position);
			
			velocity = 3;
		}
		 
		void move() {			
			Point new_coords = get_coords_from_angle(Point{position.x, position.y}, angle.angle, velocity);
			
			position.x = new_coords.x;
			position.y = new_coords.y;
			
			SDL_RenderCopy(renderer, ball, NULL, &position);
		}
		
		void render() {
			SDL_RenderCopy(renderer, ball, NULL, &position);
		}
		
		short bounce_death(Paddle &paddle_1, Paddle &paddle_2) {
			
			short returner = 0;
			
			if (position.y <= 0) {
				
				angle.angle = 0 - angle.angle;
				returner = 1;
			}
			if (position.y + position.h >= window_height) {
				angle.angle = 0 - angle.angle;
				returner = 1;
			}
			if ((position.x <= paddle_1.position.w) and ((position.y + position.h >= paddle_1.position.y) and (position.y <= paddle_1.position.y + paddle_1.position.h))) {
				angle.angle = 180 - angle.angle;
				returner =  1;
			}
			if ((position.x + position.w >= paddle_2.position.x) and ((position.y + position.h >= paddle_2.position.y) and (position.y <= paddle_2.position.y + paddle_2.position.h))) {
				angle.angle = 180 - angle.angle;
				returner =  1;
			}
			if (returner == 0) {
				if (position.x <= 0) {
					returner = 2;
				}
				if (position.x - position.w >= window_width) {
					returner = 3;
				}
			}
			
			return returner;
		}
		
		~Ball() {
			SDL_DestroyTexture(ball);
			ball = NULL;
		}
};



void load_text(std::string text, SDL_Renderer*& renderer, SDL_Texture*& texture, SDL_Rect& position, bool mode) {
	TTF_Font* font = TTF_OpenFont("Pong Font.ttf", 100);
	
	SDL_Surface *surfaceMessage = TTF_RenderText_Blended(font, text.c_str(), SDL_Color{255, 255, 255});
	
    texture = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
	
	position.x = !mode ? window_center.x - (int) (window_center.x/2) : window_center.x + (int) (window_center.x/2);
	position.y = window_center.y - (int) (window_height/2);
	
	position.w = surfaceMessage->w;
	position.h = surfaceMessage->h;
	
	std::cout << position.h << std::endl;
	
	SDL_FreeSurface(surfaceMessage);
	surfaceMessage = NULL;
	
	TTF_CloseFont(font);
	font = NULL;
}

void display_victory(bool who_won, SDL_Renderer*& renderer, SDL_Texture*& texture, SDL_Rect &position) {

	TTF_Font* font = TTF_OpenFont("Pong Font.ttf", 72);
	
	SDL_Surface *surfaceMessage = TTF_RenderText_Blended(font, !who_won ? "The Paddle on the Right WON!" : "The Paddle on the Left WON!", SDL_Color{255, 255, 255});
	
    texture = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
	
	position.x = window_center.x - (int) (surfaceMessage->w/2);
	position.y = window_center.y - (int) (surfaceMessage->h/2);
	
	position.w = surfaceMessage->w;
	position.h = surfaceMessage->h;
	
	SDL_FreeSurface(surfaceMessage);
	surfaceMessage = NULL;
	
	TTF_CloseFont(font);
	font = NULL;
}

void close(SDL_Surface*& screen_surface, SDL_Window*& window, SDL_Renderer*& renderer) {
	SDL_FreeSurface(screen_surface);
	screen_surface = NULL;
	
	SDL_DestroyWindow(window);
	window = NULL;
	
	SDL_DestroyRenderer(renderer);
	renderer = NULL;
	
	SDL_Quit();
	IMG_Quit();
	TTF_Quit();
	
	exit(0);
}
	
int main(int argc, char* argv[]) {
	SDL_Window *window = NULL;
	SDL_Surface *screen_surface = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Texture* number_texture_1 = NULL;
	SDL_Texture* number_texture_2 = NULL;
	SDL_Rect number_position_1;
	SDL_Rect number_position_2;
	SDL_Rect display_text_position;
	SDL_Texture* victory_text;

	init(*&window, *&screen_surface, *&renderer);
	
	Ball ball = (*&renderer);
	Paddle paddle_1(0, *&renderer);
	Paddle paddle_2(1, *&renderer);
	
	load_text("3", *&renderer, *&number_texture_1, number_position_1, 0);
	load_text("3", *&renderer, *&number_texture_2, number_position_2, 1);
	
	const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
	short left_paddle_lives = 3;
	short right_paddle_lives = 3;
	short bounces_in_row = 0;
	
	Second_loop:
		
	
	while (left_paddle_lives > 0 and right_paddle_lives > 0) {
		
		ball.re_init();
		paddle_1.re_init(0);
		paddle_2.re_init(1);
			
			while (true) {
				ball.move();
				
				SDL_PumpEvents();
				
				if (keyboard_state[SDL_SCANCODE_ESCAPE]) {
					close(*&screen_surface, *&window, *&renderer);
				}

				if (keyboard_state[SDL_SCANCODE_W]) {
					paddle_1.move(0);
				}
				if (keyboard_state[SDL_SCANCODE_S]) {
					paddle_1.move(1);
				}
				if (keyboard_state[SDL_SCANCODE_UP]) {
					paddle_2.move(0);
				}
				if (keyboard_state[SDL_SCANCODE_DOWN]) {
					paddle_2.move(1);
				}
				
				switch (ball.bounce_death(paddle_1, paddle_2)) {
					case 0:
						bounces_in_row = 0;
						break;
					case 1:
						ball.velocity++;
						bounces_in_row++;
						break;
					case 2:
						right_paddle_lives--;
						load_text(std::to_string(right_paddle_lives), *&renderer, *&number_texture_1, number_position_1, 0);
						SDL_Delay(100);
						goto Second_loop;
					case 3:
						left_paddle_lives--;
						load_text(std::to_string(left_paddle_lives), *&renderer, *&number_texture_2, number_position_2, 1);
						SDL_Delay(100);
						goto Second_loop;
				}
				
				if (bounces_in_row == 3) {
					if (ball.position.x < window_center.x) {
						right_paddle_lives--;
						load_text(std::to_string(right_paddle_lives), *&renderer, *&number_texture_1, number_position_1, 0);
						SDL_Delay(100);
						goto Second_loop;
					}
					else {
						left_paddle_lives--;
						load_text(std::to_string(left_paddle_lives), *&renderer, *&number_texture_2, number_position_2, 1);
						SDL_Delay(100);
						goto Second_loop;
					}
				}
				
				SDL_RenderClear(renderer);
					
				paddle_1.render();
				paddle_2.render();
				ball.render();
				
				SDL_RenderCopy(renderer, number_texture_1, NULL, &number_position_1);
				SDL_RenderCopy(renderer, number_texture_2, NULL, &number_position_2);
						
				SDL_RenderPresent(renderer);
				
				SDL_Delay(20);
			}
	}
	
	if (right_paddle_lives == 0) {
		display_victory(0, *&renderer, *&victory_text, display_text_position);
	}
	
	else {
		display_victory(1, *&renderer, *&victory_text, display_text_position);
	}
	SDL_RenderClear(renderer);
	
	paddle_1.render();
	paddle_2.render();
	ball.render();
	SDL_RenderCopy(renderer, number_texture_1, NULL, &number_position_1);
	SDL_RenderCopy(renderer, number_texture_2, NULL, &number_position_2);
	SDL_RenderCopy(renderer, victory_text, NULL, &display_text_position);
	
	SDL_RenderPresent(renderer);
	
	SDL_Delay(5000);
	
	close(*&screen_surface, *&window, *&renderer);

	return 0;
}