#pragma once

namespace Tmpl8
{
//forward declarations
class Tank;
class Rocket;
class Smoke;
class Particle_beam;

class Game
{
  public:
    void set_target(Surface* surface) { screen = surface; }
    void init();
    void shutdown();
    void update(float deltaTime);
    void draw();
    void tick(float deltaTime);
    void merge(vector<int>& left, vector<int>& right, vector<int>& sorted);
    void merge_sort(vector<int>& unsorted);
    void measure_performance();
    void updateRockets(int i);
    void updateTanks(int i);

    Tank& find_closest_enemy(Tank& current_tank);

    void mouse_up(int button)
    { /* implement if you want to detect mouse button presses */
    }

    void mouse_down(int button)
    { /* implement if you want to detect mouse button presses */
    }

    void mouse_move(int x, int y)
    { /* implement if you want to detect mouse movement */
    }

    void key_up(int key)
    { /* implement if you want to handle keys */
    }

    void key_down(int key)
    { /* implement if you want to handle keys */
    }

  private:
    Surface* screen;

    vector<Tank> tanks;
    vector<int> blue_tanks_health;
    vector<int> red_tanks_health;
    vector<Rocket> rockets;
    vector<Smoke> smokes;
    vector<Explosion> explosions;
    vector<Particle_beam> particle_beams;
    Grid grid;

    Font* frame_count_font;
    long long frame_count = 0;

    bool lock_update = false;
};

}; // namespace Tmpl8