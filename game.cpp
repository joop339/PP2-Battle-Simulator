#include "precomp.h" // include (only) this in every .cpp file

#define NUM_TANKS_BLUE 1279
#define NUM_TANKS_RED 1279

#define TANK_MAX_HEALTH 1000
#define ROCKET_HIT_VALUE 60
#define PARTICLE_BEAM_HIT_VALUE 50

#define TANK_MAX_SPEED 1.5

#define HEALTH_BARS_OFFSET_X 0
#define HEALTH_BAR_HEIGHT 70
#define HEALTH_BAR_WIDTH 1
#define HEALTH_BAR_SPACING 0

#define MAX_FRAMES 2000

//Global performance timer
#define REF_PERFORMANCE 39568 //UPDATE THIS WITH YOUR REFERENCE PERFORMANCE (see console after 2k frames)
static timer perf_timer;
static float duration;

//Load sprite files and initialize sprites
static Surface* background_img = new Surface("assets/Background_Grass.png");
static Surface* tank_red_img = new Surface("assets/Tank_Proj2.png");
static Surface* tank_blue_img = new Surface("assets/Tank_Blue_Proj2.png");
static Surface* rocket_red_img = new Surface("assets/Rocket_Proj2.png");
static Surface* rocket_blue_img = new Surface("assets/Rocket_Blue_Proj2.png");
static Surface* particle_beam_img = new Surface("assets/Particle_Beam.png");
static Surface* smoke_img = new Surface("assets/Smoke.png");
static Surface* explosion_img = new Surface("assets/Explosion.png");

static Sprite background(background_img, 1);
static Sprite tank_red(tank_red_img, 12);
static Sprite tank_blue(tank_blue_img, 12);
static Sprite rocket_red(rocket_red_img, 12);
static Sprite rocket_blue(rocket_blue_img, 12);
static Sprite smoke(smoke_img, 4);
static Sprite explosion(explosion_img, 9);
static Sprite particle_beam_sprite(particle_beam_img, 3);

const static vec2 tank_size(14, 18);
const static vec2 rocket_size(25, 24);

const static float tank_radius = 8.5f;
const static float rocket_radius = 10.f;

std::vector<std::future<void>> futs;

const unsigned int threadCount = thread::hardware_concurrency(); //Asign threadCount based on available threads on this pc
ThreadPool tp(threadCount);

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::init()
{
	frame_count_font = new Font("assets/digital_small.png", "ABCDEFGHIJKLMNOPQRSTUVWXYZ:?!=-0123456789.");

	tanks.reserve(NUM_TANKS_BLUE + NUM_TANKS_RED);

	uint rows = (uint)sqrt(NUM_TANKS_BLUE + NUM_TANKS_RED);
	uint max_rows = 12;

	float start_blue_x = tank_size.x + 10.0f;
	float start_blue_y = tank_size.y + 80.0f;

	float start_red_x = 980.0f;
	float start_red_y = 100.0f;

	float spacing = 15.0f;

	//Spawn blue tanks
	for (int i = 0; i < NUM_TANKS_BLUE; i++)
	{
		Tank tankBlue = Tank(start_blue_x + ((i % max_rows) * spacing), start_blue_y + ((i / max_rows) * spacing), BLUE, &tank_blue, &smoke, 1200, 600, tank_radius, TANK_MAX_HEALTH, TANK_MAX_SPEED);
		tanks.push_back(tankBlue);
		grid.add(&tankBlue);
	}
	//Spawn red tanks
	for (int i = 0; i < NUM_TANKS_RED; i++)
	{
		Tank tankRed = Tank(start_red_x + ((i % max_rows) * spacing), start_red_y + ((i / max_rows) * spacing), RED, &tank_red, &smoke, 80, 80, tank_radius, TANK_MAX_HEALTH, TANK_MAX_SPEED);
		tanks.push_back(tankRed);
		grid.add(&tankRed);
	}

	// Spawn Threads.
	for (int i = 0; i < threadCount; i++)
	{
		futs.push_back(tp.enqueue([i]() { std::cout << "Hello from thread " << i << std::endl; }));
	}

	particle_beams.push_back(Particle_beam(vec2(SCRWIDTH / 2, SCRHEIGHT / 2), vec2(100, 50), &particle_beam_sprite, PARTICLE_BEAM_HIT_VALUE));
	particle_beams.push_back(Particle_beam(vec2(80, 80), vec2(100, 50), &particle_beam_sprite, PARTICLE_BEAM_HIT_VALUE));
	particle_beams.push_back(Particle_beam(vec2(1200, 600), vec2(100, 50), &particle_beam_sprite, PARTICLE_BEAM_HIT_VALUE));
}

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::shutdown()
{
}

// -----------------------------------------------------------
// Iterates through all tanks and returns the closest enemy tank for the given tank
// -----------------------------------------------------------
Tank& Game::find_closest_enemy(Tank& current_tank)
{

	float closest_distance = numeric_limits<float>::infinity();
	int closest_index = 0;
	for (int i = 0; i < tanks.size(); i++) //N
	{
		if (tanks.at(i).allignment != current_tank.allignment && tanks.at(i).active)// 1
		{
			float sqr_dist = fabsf((tanks.at(i).get_position() - current_tank.get_position()).sqr_length());// 1
			if (sqr_dist < closest_distance) //1
			{
				closest_distance = sqr_dist; //1
				closest_index = i; //1
			}
		}
	}


	return tanks.at(closest_index);

}

bool CompareAllignment(Tank& tank1, Tank& tank2)
{
	if (tank1.allignment == BLUE && tank2.allignment == RED)
	{
		return true;
	}
	return false;
}

// -----------------------------------------------------------
// Update the game state:
// Move all objects
// Update sprite frames
// Collision detection
// Targeting etc..
// -----------------------------------------------------------
void Game::update(float deltaTime)
{
	//Update tanks
	grid.handleCollisions();

	sort(tanks.begin(), tanks.end(), CompareAllignment);
	blue_tanks_health.clear();
	red_tanks_health.clear();

	//Add bluetank healthbars to list
	for (int i = 0; i < NUM_TANKS_BLUE; i++)
	{
		blue_tanks_health.push_back(tanks[i].health);
	}

	//Add Red tank healthbars to list
	for (int i = NUM_TANKS_BLUE; i < NUM_TANKS_BLUE + NUM_TANKS_RED; i++)
	{
		red_tanks_health.push_back(tanks[i].health);
	}

	for (Tank& tank : tanks)
	{
		if (tank.active)
		{
			//Move tanks according to speed and nudges (see above) also reload
			tank.tick();
			grid.move(&tank);

			//Shoot at closest target if reloaded
			if (tank.rocket_reloaded())
			{
				Tank& target = find_closest_enemy(tank);

				rockets.push_back(Rocket(tank.position, (target.get_position() - tank.position).normalized() * 3, rocket_radius, tank.allignment, ((tank.allignment == RED) ? &rocket_red : &rocket_blue)));

				tank.reload_rocket();
			}
		}
	}

	//Update smoke plumes
	for (Smoke& smoke : smokes)
	{
		smoke.tick();
	}

	//Update rockets
	for (Rocket& rocket : rockets) //N
	{
		rocket.tick(); //1

		//Outer most bounds of rocket using collision radius
		int rocket_grid_left = (rocket.position.x - rocket.collision_radius) / grid.CELL_SIZE; //1
		int rocket_grid_right = (rocket.position.x + rocket.collision_radius) / grid.CELL_SIZE; //1
		int rocket_grid_top = (rocket.position.y - rocket.collision_radius) / grid.CELL_SIZE; //1
		int rocket_grid_bottom = (rocket.position.y + rocket.collision_radius) / grid.CELL_SIZE; //1

		int grid_max = grid.CELL_SIZE * grid.NUM_CELLS;


		//check if rocket is on the grid
		if (rocket.position.x < grid_max - rocket.collision_radius && rocket.position.x > 0 &&
			rocket.position.y < grid_max - rocket.collision_radius && rocket.position.y > 0)

			//make sure to check for collisions within the current cell and one cell around the rocket.
			for (int x = rocket_grid_left; x <= rocket_grid_right; x++) //1
				for (int y = rocket_grid_top; y <= rocket_grid_bottom; y++) //1
					for (Tank* tank : grid.cells[x][y]) //M
					{
						if (tank->active && (tank->allignment != rocket.allignment) && rocket.intersects(tank->position, tank->collision_radius)) //1
						{
							explosions.push_back(Explosion(&explosion, tank->position));

							if (tank->hit(ROCKET_HIT_VALUE)) //1
							{
								smokes.push_back(Smoke(smoke, tank->position - vec2(0, 48)));
							}

							rocket.active = false;
							break;
						}
					}
	}

	//Remove exploded rockets with remove erase idiom
	rockets.erase(std::remove_if(rockets.begin(), rockets.end(), [](const Rocket& rocket) { return !rocket.active; }), rockets.end());

	//Update particle beams
	for (Particle_beam& particle_beam : particle_beams) //N
	{
		particle_beam.tick(tanks); //1

		// positions of the cells where particle_beam collision has to be detected.
		int particle_beam_left = particle_beam.min_position.x / grid.CELL_SIZE; //1
		int particle_beam_right = (particle_beam.min_position.x + particle_beams[0].max_position.x) / grid.CELL_SIZE; //1
		int particle_beam_top = particle_beam.min_position.y / grid.CELL_SIZE; //1
		int particle_beam_bottom = (particle_beam.min_position.y + particle_beams[0].max_position.y) / grid.CELL_SIZE; //1

		//Damage all tanks within the collision area.
		for (int x = particle_beam_left; x <= particle_beam_right; x++)
			for (int y = particle_beam_top; y <= particle_beam_bottom; y++)
				for (Tank* tank : grid.cells[x][y]) //M
				{
					if (tank->active && particle_beam.rectangle.intersects_circle(tank->get_position(), tank->get_collision_radius())) //1
					{
						if (tank->hit(particle_beam.damage)) //1
						{
							smokes.push_back(Smoke(smoke, tank->position - vec2(0, 48))); //1
						}
					}
				}
	}

	//Update explosion sprites and remove when done with remove erase idiom
	for (Explosion& explosion : explosions)
	{
		explosion.tick();
	}

	explosions.erase(std::remove_if(explosions.begin(), explosions.end(), [](const Explosion& explosion) { return explosion.done(); }), explosions.end());
}

bool CompareHealth(Tank& tank1, Tank& tank2)
{
	return tank1.health < tank2.health;
}

void Game::draw()
{
	// clear the graphics window
	screen->clear(0);

	//Draw background
	background.draw(screen, 0, 0);

	//Draw sprites
	for (int i = 0; i < NUM_TANKS_BLUE + NUM_TANKS_RED; i++)
	{
		tanks.at(i).draw(screen);

		vec2 tank_pos = tanks.at(i).get_position();
		// tread marks
		if ((tank_pos.x >= 0) && (tank_pos.x < SCRWIDTH) && (tank_pos.y >= 0) && (tank_pos.y < SCRHEIGHT))
			background.get_buffer()[(int)tank_pos.x + (int)tank_pos.y * SCRWIDTH] = sub_blend(background.get_buffer()[(int)tank_pos.x + (int)tank_pos.y * SCRWIDTH], 0x808080);
	}

	for (Rocket& rocket : rockets)
	{
		rocket.draw(screen);
	}

	for (Smoke& smoke : smokes)
	{
		smoke.draw(screen);
	}

	for (Particle_beam& particle_beam : particle_beams)
	{
		particle_beam.draw(screen);
	}

	for (Explosion& explosion : explosions)
	{
		explosion.draw(screen);
	}

	//Sort tankbar health before drawing.
	merge_sort(red_tanks_health);
	merge_sort(blue_tanks_health);

	//Draw sorted health bars (using only usefull information instead of every object in tanks.)
	for (int t = 0; t < 2; t++)
	{
		const int NUM_TANKS = ((t < 1) ? NUM_TANKS_BLUE : NUM_TANKS_RED);

		const int begin = ((t < 1) ? 0 : NUM_TANKS_BLUE);
		if (t == 0)
		{
			for (int i = 0; i < NUM_TANKS; i++)
			{
				int health_bar_start_x = i * (HEALTH_BAR_WIDTH + HEALTH_BAR_SPACING) + HEALTH_BARS_OFFSET_X;
				int health_bar_start_y = 0;
				int health_bar_end_x = health_bar_start_x + HEALTH_BAR_WIDTH;
				int health_bar_end_y = HEALTH_BAR_HEIGHT;

				screen->bar(health_bar_start_x, health_bar_start_y, health_bar_end_x, health_bar_end_y, REDMASK);
				screen->bar(health_bar_start_x, health_bar_start_y + (int)((double)HEALTH_BAR_HEIGHT * (1 - ((double)blue_tanks_health[i] / (double)TANK_MAX_HEALTH))), health_bar_end_x, health_bar_end_y, GREENMASK);
			}
		}
		if (t == 1)
		{
			for (int i = 0; i < NUM_TANKS; i++)
			{
				int health_bar_start_x = i * (HEALTH_BAR_WIDTH + HEALTH_BAR_SPACING) + HEALTH_BARS_OFFSET_X;
				int health_bar_start_y = (SCRHEIGHT - HEALTH_BAR_HEIGHT) - 1;
				int health_bar_end_x = health_bar_start_x + HEALTH_BAR_WIDTH;
				int health_bar_end_y = SCRHEIGHT - 1;

				screen->bar(health_bar_start_x, health_bar_start_y, health_bar_end_x, health_bar_end_y, REDMASK);
				screen->bar(health_bar_start_x, health_bar_start_y + (int)((double)HEALTH_BAR_HEIGHT * (1 - ((double)red_tanks_health[i] / (double)TANK_MAX_HEALTH))), health_bar_end_x, health_bar_end_y, GREENMASK);
			}
		}
	}
}

void Tmpl8::Game::merge(vector<int>& left, vector<int>& right, vector<int>& sorted)
{
	int i = 0;
	int j = 0;
	int k = 0;

	while (j < left.size() && k < right.size()) // both lists are not empty
	{
		// check for smallest first elem
		// add smallest to result list
		// remove smallest from origin
		if (left[j] < right[k]) // left[0] < right[0]
		{
			sorted[i] = left[j];
			j++;
		}
		else
		{
			sorted[i] = right[k];
			k++;
		}
		i++;
	}
	while (j < left.size())
	{
		sorted[i] = left[j];
		j++;
		i++;
	}
	while (k < right.size())
	{
		sorted[i] = right[k];
		k++;
		i++;
	}


}

void Tmpl8::Game::merge_sort(vector<int>& unsorted)
{
	if (unsorted.size() <= 1) // if list is empty or just one elem
	{
		return; // return cos it's already sorted
	}

	vector<int> left;
	left.reserve(unsorted.size() / 2);
	vector<int> right;
	right.reserve(unsorted.size() / 2);

	int middle = (unsorted.size() + 1) / 2; // find middle index

	for (int i = 0; i < middle; i++)
	{
		left.push_back(unsorted[i]); // push left side to left
	}

	for (int i = middle; i < unsorted.size(); i++)
	{
		right.push_back(unsorted[i]);
	}

	merge_sort(left); // divide until 0 or 1 elem is left
	merge_sort(right); // divide until 0 or 1 elem is left
	merge(left, right, unsorted); // merge left and right
}

// -----------------------------------------------------------
// When we reach MAX_FRAMES print the duration and speedup multiplier
// Updating REF_PERFORMANCE at the top of this file with the value
// on your machine gives you an idea of the speedup your optimizations give
// -----------------------------------------------------------
void Tmpl8::Game::measure_performance()
{
	char buffer[128];
	if (frame_count >= MAX_FRAMES)
	{
		if (!lock_update)
		{
			duration = perf_timer.elapsed();
			cout << "Duration was: " << duration << " (Replace REF_PERFORMANCE with this value)" << endl;
			lock_update = true;
		}

		frame_count--;
	}

	if (lock_update)
	{
		screen->bar(420, 170, 870, 430, 0x030000);
		int ms = (int)duration % 1000, sec = ((int)duration / 1000) % 60, min = ((int)duration / 60000);
		sprintf(buffer, "%02i:%02i:%03i", min, sec, ms);
		frame_count_font->centre(screen, buffer, 200);
		sprintf(buffer, "SPEEDUP: %4.1f", REF_PERFORMANCE / duration);
		frame_count_font->centre(screen, buffer, 340);
	}
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::tick(float deltaTime)
{
	if (!lock_update)
	{
		update(deltaTime);
	}
	draw();

	measure_performance();

	// print something in the graphics window
	//screen->Print("hello world", 2, 2, 0xffffff);

	// print something to the text window
	//cout << "This goes to the console window." << std::endl;

	//Print frame count
	frame_count++;
	string frame_count_string = "FRAME: " + std::to_string(frame_count);
	frame_count_font->print(screen, frame_count_string.c_str(), 350, 580);
}
