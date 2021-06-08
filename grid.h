#pragma once

namespace Tmpl8
{

class Grid
{

  public:
    Grid();
	~Grid() = default;
    void add(Tank* tank);
    void handleCollisions();
    void move(Tank* tank);
    void handleUnit(vector<Tank*> &tank_ptrs, vector<Tank*> &oTank_ptrs);
    void handleCell(int x, int y);

    static const int NUM_CELLS = 80;
    static const int CELL_SIZE = 25;

  private:
    vector<vector<vector<Tank*>>> cells;

};
} // namespace Tmpl8