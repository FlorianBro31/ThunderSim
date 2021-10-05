#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 700

#define DIVISION 10

#define MAX_CELL_X (WINDOW_WIDTH/DIVISION)
#define MAX_CELL_Y (WINDOW_HEIGHT/DIVISION)

#define P 54 //Probability of generating an UP or DOWN border, 54 sweet spot
#define Q 50 //Probability of generating a RIGHT or LEFT border, 50 sweet spot

#define EMPTY 0b0000
#define UP    0b0001
#define DOWN  0b0010			//		       1
#define RIGHT 0b0100			//  	 ____________
#define LEFT  0b1000			//		|			 |
								//		|			 |
typedef struct pixel			//	8	|	  0 	 |    4
{							    //		|			 |
	union Border				//		|____________|
	{							//
		uint8_t border;			//			   2
		struct
		{
			uint8_t up:1;
			uint8_t down:1;
			uint8_t right:1;
			uint8_t left:1;
		};
	}Border;
	int value;		
}pixel;								

pixel** grille;

void InitGrid();
void DestroyGrid();
void CreateMaze();
int SolveMaze();
void PrintGrid(int param);
SDL_Rect* FillMazeBuffer(int *c);
SDL_Rect* FillSolutionBuffer(int *c);
//TODO: r to reload new maze, allow custom dimension
int main()
{
	if(SDL_Init(SDL_INIT_EVERYTHING)!=0)
	{
		printf("Failed to load SDL : %s\n",SDL_GetError());
		return EXIT_FAILURE;
	}
	SDL_Window* fenetre;
	SDL_Renderer* renderer;
	SDL_Rect* borders_rects=NULL;
	SDL_Rect* solution_rects=NULL;
	SDL_Event event;
	SDL_bool running=SDL_TRUE;
	Uint32 starting_tick;
	int solving_counter = 0;
	int counter_borders_rects = 0;
	int counter_solution_rects = 0;
	fenetre = SDL_CreateWindow("ThunderSim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
	renderer = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_SOFTWARE);
	
	InitGrid();
	printf("x_max : %d, y_max = %d\n",MAX_CELL_X, MAX_CELL_Y);
	do
	{
		CreateMaze();
		solving_counter++;
	} while (SolveMaze() != SDL_TRUE);
	//PrintGrid(0);
	if(solving_counter==1)
		printf("Maze solved in %d attempt\n", solving_counter);
	else
		printf("Maze solved in %d attempts\n", solving_counter);
	borders_rects = FillMazeBuffer(&counter_borders_rects);
	//PrintGrid(1);
	solution_rects = FillSolutionBuffer(&counter_solution_rects);
	while (running)
	{
		starting_tick = SDL_GetTicks();
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				running = SDL_FALSE;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
				case SDLK_r:
					DestroyGrid();
					InitGrid();
					free(borders_rects);
					borders_rects = NULL;
					free(solution_rects);
					solution_rects = NULL;
					solving_counter = 0;
					counter_borders_rects = 0;
					counter_solution_rects = 0;
					printf("x_max : %d, y_max = %d\n",MAX_CELL_X, MAX_CELL_Y);
					do
					{
						CreateMaze();
						solving_counter++;
					} while (SolveMaze() != SDL_TRUE);
					//PrintGrid(0);
					if(solving_counter==1)
						printf("Maze solved in %d attempt\n", solving_counter);
					else
						printf("Maze solved in %d attempts\n", solving_counter);
					borders_rects = FillMazeBuffer(&counter_borders_rects);
					//PrintGrid(1);
					solution_rects = FillSolutionBuffer(&counter_solution_rects);
					break;
				default:
					break;
				}
			default:
				break;
			}
		}
		
		SDL_SetRenderDrawColor(renderer,0,0,0,SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer,255,255,255,SDL_ALPHA_OPAQUE);
		SDL_RenderFillRects(renderer, borders_rects, counter_borders_rects);
		SDL_RenderFillRects(renderer, solution_rects, counter_solution_rects);
		SDL_RenderPresent(renderer);

		if ((1000/30) > SDL_GetTicks() - starting_tick)
        {
            SDL_Delay(1000/30 - (SDL_GetTicks()-starting_tick));
        }
	}
	free(borders_rects);
	borders_rects = NULL;
	free(solution_rects);
	solution_rects = NULL;
	DestroyGrid();
	SDL_DestroyWindow(fenetre);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();

	return EXIT_SUCCESS;
}

void InitGrid()
{
	grille = malloc(sizeof(pixel*)*MAX_CELL_X);
	for (int i = 0; i < MAX_CELL_X; i++)
	{
		grille[i] = malloc(sizeof(pixel)*MAX_CELL_Y);
	}
}

void DestroyGrid()
{
	for (int i = 0; i < MAX_CELL_X; i++)
	{
		free(grille[i]);
		grille[i] = NULL;
	}
	grille = NULL;
}

void CreateMaze()
{
	uint8_t choices_phase1[] = {UP, DOWN};
	uint8_t choices_phase2[] = {RIGHT, LEFT};
	time_t t;
	srand((unsigned int) time(&t));
	for (int x = 0; x < MAX_CELL_X; x++)
	{
		for (int y = 0; y < MAX_CELL_Y; y++)
		{
			grille[x][y].value = -1;
			if((rand()%100+1) <= P)
			{
				grille[x][y].Border.border = choices_phase1[rand()%2];
			}
			else
			{
				grille[x][y].Border.border = EMPTY;
			}
			
		}
	}
	for (int x = 0; x < MAX_CELL_X; x++)
	{
		for (int y = 0; y < MAX_CELL_Y; y++)
		{
			if ((rand()%100+1) <= Q)
			{
				grille[x][y].Border.border = grille[x][y].Border.border | choices_phase2[rand()%2];
			}
			else
			{
				grille[x][y].Border.border = grille[x][y].Border.border | EMPTY;
			}	
		}
	}
	//PrintGrid(0);
}

int SolveMaze()
{
	int counter_potential_path = 0;
	int counter_potential_old_path = 0;
	uint32_t* potential_path = NULL;
	uint32_t* old_potential_path = NULL;
	int actual_value = 0;
	SDL_bool exit_found = SDL_FALSE;
	for (uint32_t x = 0; x < MAX_CELL_X; x++)
	{	//Check the last bit
		if(grille[x][0].Border.up ^ 1) //If there is no ceiling, we can start at that cell
		{
			counter_potential_path++;
			grille[x][0].value = actual_value;
			potential_path = (uint32_t*) realloc(potential_path, sizeof(uint32_t)*counter_potential_path);
			potential_path[counter_potential_path-1] = 0 | (x << 16);
			//printf("%d,",potential_path[counter_potential_path-1]);
		}
	}
	//printf("\n");
	while (potential_path !=NULL && !exit_found)
	{
		old_potential_path = malloc(sizeof(uint32_t)*counter_potential_path);
		for (int i = 0; i < counter_potential_path; i++)
		{
			old_potential_path[i] = potential_path[i];
		}
		free(potential_path);
		potential_path = NULL;
		counter_potential_old_path = counter_potential_path;
		counter_potential_path = 0;
		actual_value++;
		//printf("Entering boucle\n");
		for (int i = 0; i < counter_potential_old_path; i++)
		{
			uint32_t x = old_potential_path[i] >> 16;
			uint16_t y = old_potential_path[i];
			//printf("searching for x : %d, y : %d\n",x,y);
			if (grille[x][y].Border.down ^ 1)//Down is open
			{
				if (y<MAX_CELL_Y-1)
				{
					if ((grille[x][y+1].Border.up ^ 1) && grille[x][y+1].value == -1)//We can go down
					{
						counter_potential_path++;
						grille[x][y+1].value = actual_value;
						potential_path = (uint32_t*) realloc(potential_path, sizeof(uint32_t)*counter_potential_path);
						potential_path[counter_potential_path-1] = (y+1) | (x<<16);
					}
				}
				else if(y==MAX_CELL_Y-1)//We must have reached an exit
				{
					printf("Solved ! \n");
					exit_found = SDL_TRUE;
					break;
				}
			}
			if((grille[x][y].Border.right ^ 1) && x<MAX_CELL_X-1)//Right path is open
			{
				if((grille[x+1][y].Border.left ^ 1) && grille[x+1][y].value == -1)
				{
					counter_potential_path++;
					grille[x+1][y].value = actual_value;
					potential_path = (uint32_t*) realloc(potential_path, sizeof(uint32_t)*counter_potential_path);
					potential_path[counter_potential_path-1] = y | ((x+1) << 16);
				}
			}
			if((grille[x][y].Border.left ^ 1) && x>0)//Left path is open
			{
				if((grille[x-1][y].Border.right ^ 1) && grille[x-1][y].value == -1)
				{
					counter_potential_path++;
					grille[x-1][y].value = actual_value;
					potential_path = (uint32_t*) realloc(potential_path, sizeof(uint32_t)*counter_potential_path);
					potential_path[counter_potential_path-1] = y | ((x-1) << 16);
				}
			}
			if((grille[x][y].Border.up ^ 1) && y>0)//UP is open
			{
				if((grille[x][y-1].Border.down ^ 1) && grille[x][y-1].value == -1)
				{
					counter_potential_path++;
					grille[x][y-1].value = actual_value;
					potential_path = (uint32_t*) realloc(potential_path, sizeof(uint32_t)*counter_potential_path);
					potential_path[counter_potential_path-1] = (y-1) | (x<<16);
				}
			}
		}
		free(old_potential_path);
		old_potential_path = NULL;
	}
	if(potential_path != NULL)
	{
		free(potential_path);
		potential_path = NULL;
	}
	if(exit_found);
		//PrintGrid(1);
	return exit_found;
}

void PrintGrid(int param)
{
	if(param == 0)
	{
		for (int y = 0; y < MAX_CELL_Y; y++)
		{
			for (int x = 0; x < MAX_CELL_X; x++)
			{
				printf("%2d,",grille[x][y].Border.border);
			}
			printf("\n");
		}
	}
	else if (param == 1)
	{
		for (int y = 0; y < MAX_CELL_Y; y++)
		{
			for (int x = 0; x < MAX_CELL_X; x++)
			{
				printf("%2d,",grille[x][y].value);
			}
			printf("\n");
		}
	}	
}

SDL_Rect* FillMazeBuffer(int* counter)
{
	//int counter_borders_rects = 0;
	SDL_Rect* borders_rects = NULL;
	for (int x = 0; x < MAX_CELL_X; x++)
	{
		for (int y = 0; y < MAX_CELL_Y; y++)
		{
			if(x<MAX_CELL_X-1)
			{
				if(grille[x][y].Border.right & grille[x+1][y].Border.left)
				{
					grille[x][y].Border.right = 0;
				}
			}
			if(y<MAX_CELL_Y-1)
			{
				if(grille[x][y].Border.down & grille[x][y+1].Border.up)
				{
					grille[x][y].Border.down = 0;
				}
			}
			if(grille[x][y].Border.up)
			{
				(*counter)++;
				borders_rects = (SDL_Rect*) realloc(borders_rects, sizeof(SDL_Rect)*(*counter));
				borders_rects[(*counter)-1].h = 1;
				borders_rects[(*counter)-1].w = DIVISION;
				borders_rects[(*counter)-1].x = DIVISION*x;
				borders_rects[(*counter)-1].y = DIVISION*y;
			}
			if(grille[x][y].Border.down)
			{
				(*counter)++;
				borders_rects = (SDL_Rect*) realloc(borders_rects, sizeof(SDL_Rect)*(*counter));
				borders_rects[(*counter)-1].h = 1;
				borders_rects[(*counter)-1].w = DIVISION;
				borders_rects[(*counter)-1].x = DIVISION*x;
				if(y==MAX_CELL_Y-1)
					borders_rects[(*counter)-1].y = (DIVISION*y)+DIVISION-1;
				else
					borders_rects[(*counter)-1].y = (DIVISION*y)+DIVISION;
			}
			if(grille[x][y].Border.right)
			{
				(*counter)++;
				borders_rects = (SDL_Rect*) realloc(borders_rects, sizeof(SDL_Rect)*(*counter));
				borders_rects[(*counter)-1].h = DIVISION;
				borders_rects[(*counter)-1].w = 1;
				if(x==MAX_CELL_X -1)
					borders_rects[(*counter)-1].x = (DIVISION*x)+DIVISION-1;
				else
					borders_rects[(*counter)-1].x = (DIVISION*x)+DIVISION;
				borders_rects[(*counter)-1].y = DIVISION*y;
			}
			if(grille[x][y].Border.left)
			{
				(*counter)++;
				borders_rects = (SDL_Rect*) realloc(borders_rects, sizeof(SDL_Rect)*(*counter));
				borders_rects[(*counter)-1].h = DIVISION;
				borders_rects[(*counter)-1].w = 1;
				borders_rects[(*counter)-1].x = DIVISION*x;
				borders_rects[(*counter)-1].y = DIVISION*y;
			}
		}
	}
	printf("done\n");
	//PrintGrid(0);
	return borders_rects;
}

SDL_Rect* FillSolutionBuffer(int* counter)
{
	SDL_Rect* solution_rects;
	uint32_t** buffer_paths = NULL;
	int counter_paths = 0;
	int previous_max_value = 0;
	SDL_bool found = SDL_FALSE;
	buffer_paths = malloc(sizeof(uint32_t*)*MAX_CELL_X);
	for (uint32_t x = 0; x < MAX_CELL_X; x++)
	{
		if((grille[x][MAX_CELL_Y-1].value != -1) && (grille[x][MAX_CELL_Y-1].Border.down == 0))
		{
			//printf("value_sol : %d\n", grille[x][MAX_CELL_Y-1].value);
			counter_paths++;
			buffer_paths[counter_paths-1] = (uint32_t*) malloc(sizeof(uint32_t));
			buffer_paths[counter_paths-1][0] = (x <<16)| MAX_CELL_Y-1; 
			//printf("counter : %d\n", counter_paths);
		}
	}
	for (int i = 0; i < counter_paths; i++)
	{
		uint16_t y = buffer_paths[i][0]; //starting x
		uint32_t x = buffer_paths[i][0] >> 16; //starting y
		int max_value = grille[x][y].value; //starting value
		//printf("Max value : %d\n",max_value);
		buffer_paths[i] = (uint32_t*) realloc(buffer_paths[i],sizeof(uint32_t)*(max_value+1));
		for (int u = max_value; u > 0; u--)
		{
			//printf("u:%d, u-max:%d\n",u,max_value-u);
			found = SDL_FALSE;
			if(y>0 && !found)
			{
				if(grille[x][y-1].value == u-1 && (grille[x][y].Border.up | grille[x][y-1].Border.down) == 0)
				{
					//printf("y-1\n");
					found = SDL_TRUE;
					y = y-1;
					buffer_paths[i][1+(max_value-u)] = (x<<16) | y;
				}
			}
			
			if(x<MAX_CELL_X-1 && !found)
			{
				if(grille[x+1][y].value == u-1 && (grille[x][y].Border.right | grille[x+1][y].Border.left) == 0)
				{
					//printf("x+1\n");
					found = SDL_TRUE;
					x=x+1;
					buffer_paths[i][1+(max_value-u)] = (x<<16) | y;
				}
			}

			if(x>0 && !found)
			{
				if(grille[x-1][y].value == u-1 && (grille[x][y].Border.left | grille[x-1][y].Border.right) == 0)
				{
					//printf("x-1\n");
					found = SDL_TRUE;
					x = x-1;
					buffer_paths[i][1+(max_value-u)] = (x<<16) | y;
				}
			}

			if(y<MAX_CELL_Y-1 && !found)
			{
				if(grille[x][y+1].value == u-1 && (grille[x][y].Border.down | grille[x][y+1].Border.up) == 0)
				{
					//printf("y+1\n");
					found = SDL_TRUE;
					y = y+1;
					buffer_paths[i][1+(max_value-u)] = (x<<16) | y;
				}
			}
			//printf("x:%d, y:%d\n", x, y);
		}
		
	}
	int* counter_coords = malloc(sizeof(int)*counter_paths);
	for (int i = 0; i < counter_paths; i++)
	{
		uint16_t y = buffer_paths[i][0];
		uint32_t x = buffer_paths[i][0] >> 16;
		counter_coords[i] = grille[x][y].value;
	}
	int small = counter_coords[0];
	int pos = 0;
	for (int i = 1; i < counter_paths; i++)
	{
		if(counter_coords[i] < small)
		{
			small = counter_coords[i];
			pos=i;
		}
	}
	(*counter) = (small+1)*3;
	//printf("pos : %d, value : %d\n",pos, small);

	solution_rects = malloc(sizeof(SDL_Rect)*((small+1)*3));
	int counter_sol_rects = 1;
	for (int i = 0; i <= small; i++)//Not very pretty, can probably be rewritten
	{
		//printf("i:%d, c:%d\n",i,counter_sol_rects);
		
		uint16_t y = buffer_paths[pos][i];
		uint16_t previous_y = -1;
		uint16_t next_y = -1;
		uint32_t x = buffer_paths[pos][i] >> 16;
		uint32_t previous_x = -1;
		uint32_t next_x = -1;

		if(i != small)
		{
			next_y = buffer_paths[pos][i+1];
			next_x = buffer_paths[pos][i+1] >> 16;
		}
		else
		{
			solution_rects[counter_sol_rects+1].h = DIVISION/3;
			solution_rects[counter_sol_rects+1].w = DIVISION/3;
			solution_rects[counter_sol_rects+1].x = (DIVISION*x) + DIVISION/3;
			solution_rects[counter_sol_rects+1].y = (DIVISION*y);
		}
		
		if (i != 0)
		{
			previous_y = buffer_paths[pos][i-1];
			previous_x = buffer_paths[pos][i-1] >> 16;
		}
		else
		{
			solution_rects[counter_sol_rects-1].h = DIVISION/3;
			solution_rects[counter_sol_rects-1].w = DIVISION/3;
			solution_rects[counter_sol_rects-1].x = (DIVISION*x) + DIVISION/3;
			solution_rects[counter_sol_rects-1].y = (DIVISION*y) + (DIVISION/3)*2;
			if(next_y == y-1)//And we go up
			{
				solution_rects[counter_sol_rects+1].h = DIVISION/3;
				solution_rects[counter_sol_rects+1].w = DIVISION/3;
				solution_rects[counter_sol_rects+1].x = (DIVISION*x) + DIVISION/3;
				solution_rects[counter_sol_rects+1].y = (DIVISION*y);
			}
			else if(next_x == x+1)//And we go right
			{
				solution_rects[counter_sol_rects+1].h = DIVISION/3;
				solution_rects[counter_sol_rects+1].w = DIVISION/3;
				solution_rects[counter_sol_rects+1].x = (DIVISION*x) + (DIVISION/3)*2;
				solution_rects[counter_sol_rects+1].y = (DIVISION*y) + DIVISION/3;
			}
			else if (next_x == x-1)//And we go left
			{
				solution_rects[counter_sol_rects+1].h = DIVISION/3;
				solution_rects[counter_sol_rects+1].w = DIVISION/3;
				solution_rects[counter_sol_rects+1].x = (DIVISION*x);
				solution_rects[counter_sol_rects+1].y = (DIVISION*y) + DIVISION/3;
			}
		}
		
		if(previous_y == y+1)//If we come from the bottom
		{
			solution_rects[counter_sol_rects-1].h = DIVISION/3;
			solution_rects[counter_sol_rects-1].w = DIVISION/3;
			solution_rects[counter_sol_rects-1].x = (DIVISION*x) + DIVISION/3;
			solution_rects[counter_sol_rects-1].y = (DIVISION*y) + (DIVISION/3)*2;
			if(next_y == y-1)//And we go up
			{
				solution_rects[counter_sol_rects+1].h = DIVISION/3;
				solution_rects[counter_sol_rects+1].w = DIVISION/3;
				solution_rects[counter_sol_rects+1].x = (DIVISION*x) + DIVISION/3;
				solution_rects[counter_sol_rects+1].y = (DIVISION*y);
			}
			else if(next_x== x+1)//And we go right
			{
				solution_rects[counter_sol_rects+1].h = DIVISION/3;
				solution_rects[counter_sol_rects+1].w = DIVISION/3;
				solution_rects[counter_sol_rects+1].x = (DIVISION*x) + (DIVISION/3)*2;
				solution_rects[counter_sol_rects+1].y = (DIVISION*y) + DIVISION/3;
			}
			else if (next_x == x-1)//And we go left
			{
				solution_rects[counter_sol_rects+1].h = DIVISION/3;
				solution_rects[counter_sol_rects+1].w = DIVISION/3;
				solution_rects[counter_sol_rects+1].x = (DIVISION*x);
				solution_rects[counter_sol_rects+1].y = (DIVISION*y) + DIVISION/3;
			}	
		}
		else if (previous_y == y-1)//If we come from uptop
		{
			solution_rects[counter_sol_rects-1].h = DIVISION/3;
			solution_rects[counter_sol_rects-1].w = DIVISION/3;
			solution_rects[counter_sol_rects-1].x = (DIVISION*x) + DIVISION/3;
			solution_rects[counter_sol_rects-1].y = (DIVISION*y);
			if(next_y == y+1)//And we go down
			{
				solution_rects[counter_sol_rects+1].h = DIVISION/3;
				solution_rects[counter_sol_rects+1].w = DIVISION/3;
				solution_rects[counter_sol_rects+1].x = (DIVISION*x) + (DIVISION/3);
				solution_rects[counter_sol_rects+1].y = (DIVISION*y) + (DIVISION/3)*2;
			}
			else if ((buffer_paths[pos][i+1]>>16) == x+1)//And we go right
			{
				solution_rects[counter_sol_rects+1].h = DIVISION/3;
				solution_rects[counter_sol_rects+1].w = DIVISION/3;
				solution_rects[counter_sol_rects+1].x = (DIVISION*x) + (DIVISION/3)*2;
				solution_rects[counter_sol_rects+1].y = (DIVISION*y) + (DIVISION/3);
			}
			else if ((buffer_paths[pos][i+1]>>16) == x-1)//And we go left
			{
				solution_rects[counter_sol_rects+1].h = DIVISION/3;
				solution_rects[counter_sol_rects+1].w = DIVISION/3;
				solution_rects[counter_sol_rects+1].x = (DIVISION*x);
				solution_rects[counter_sol_rects+1].y = (DIVISION*y) + (DIVISION/3);
			}
		}
		else if(previous_x == x-1)//If we come from the left
		{
			solution_rects[counter_sol_rects-1].h = DIVISION/3;
			solution_rects[counter_sol_rects-1].w = DIVISION/3;
			solution_rects[counter_sol_rects-1].x = (DIVISION*x);
			solution_rects[counter_sol_rects-1].y = (DIVISION*y) + DIVISION/3;
			if (next_y == y+1)//And we go down
			{
				solution_rects[counter_sol_rects+1].h = DIVISION/3;
				solution_rects[counter_sol_rects+1].w = DIVISION/3;
				solution_rects[counter_sol_rects+1].x = (DIVISION*x) + (DIVISION/3);
				solution_rects[counter_sol_rects+1].y = (DIVISION*y) + (DIVISION/3)*2;
			}
			else if(next_y == y-1)//And we go up
			{
				solution_rects[counter_sol_rects+1].h = DIVISION/3;
				solution_rects[counter_sol_rects+1].w = DIVISION/3;
				solution_rects[counter_sol_rects+1].x = (DIVISION*x) + DIVISION/3;
				solution_rects[counter_sol_rects+1].y = (DIVISION*y);
			}
			else if (next_x == x+1)//And we go right
			{
				solution_rects[counter_sol_rects+1].h = DIVISION/3;
				solution_rects[counter_sol_rects+1].w = DIVISION/3;
				solution_rects[counter_sol_rects+1].x = (DIVISION*x) + (DIVISION/3)*2;
				solution_rects[counter_sol_rects+1].y = (DIVISION*y) + (DIVISION/3);
			}
		}
		else if(previous_x == x+1)//If we come from the right
		{
			solution_rects[counter_sol_rects-1].h = DIVISION/3;
			solution_rects[counter_sol_rects-1].w = DIVISION/3;
			solution_rects[counter_sol_rects-1].x = (DIVISION*x) + (DIVISION/3)*2;
			solution_rects[counter_sol_rects-1].y = (DIVISION*y) + DIVISION/3;
			if(next_y == y+1)//And we go down
			{
				solution_rects[counter_sol_rects+1].h = DIVISION/3;
				solution_rects[counter_sol_rects+1].w = DIVISION/3;
				solution_rects[counter_sol_rects+1].x = (DIVISION*x) + (DIVISION/3);
				solution_rects[counter_sol_rects+1].y = (DIVISION*y) + (DIVISION/3)*2;
			}
			else if(next_y == y-1)//And we go up
			{
				solution_rects[counter_sol_rects+1].h = DIVISION/3;
				solution_rects[counter_sol_rects+1].w = DIVISION/3;
				solution_rects[counter_sol_rects+1].x = (DIVISION*x) + DIVISION/3;
				solution_rects[counter_sol_rects+1].y = (DIVISION*y);
			}
			else if(next_x == x-1)//And we go left
			{
				solution_rects[counter_sol_rects+1].h = DIVISION/3;
				solution_rects[counter_sol_rects+1].w = DIVISION/3;
				solution_rects[counter_sol_rects+1].x = (DIVISION*x);
				solution_rects[counter_sol_rects+1].y = (DIVISION*y) + (DIVISION/3);
			}
		}
		solution_rects[counter_sol_rects].h = DIVISION/3;
		solution_rects[counter_sol_rects].w = DIVISION/3;
		solution_rects[counter_sol_rects].x = (DIVISION*x) + DIVISION/3;
		solution_rects[counter_sol_rects].y = (DIVISION*y) + DIVISION/3;
		counter_sol_rects = counter_sol_rects + 3;
	}
	for (int i = 0; i < counter_paths; i++)
	{
		free(buffer_paths[i]);
		buffer_paths[i]=NULL;
	}
	buffer_paths = NULL;
	return solution_rects;
	
}