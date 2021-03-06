#include "precomp.h"

namespace Tmpl8
{

Grid::Grid()
{
    // Fill grid with empty vectors.
    for (int y = 0; y < NUM_CELLS; y++)
    {
        vector<vector<Tank*>> rows;
;
        for (int x = 0; x < NUM_CELLS; x++)
        {
            vector<Tank*> cols;
            rows.push_back(cols);
        }
		cells.push_back(rows);

    }
};

void Grid::add(Tank* tank)
{
    // Determine which grid cell it's in.
    int cellX = (int)(tank->position.x / (float)Grid::CELL_SIZE);
    int cellY = (int)(tank->position.y / (float)Grid::CELL_SIZE);
    // Add to the list for the cell it's in.
    cells[cellX][cellY].push_back(tank);
}

void Grid::handleCollisions()
{
    for (int x = 0; x < NUM_CELLS; x++)
    {
        for (int y = 0; y < NUM_CELLS; y++)
        {

			handleCell(x, y);
            
        }
    }
}

void Grid::handleCell(int x, int y)
{
    //Handle current unit
    //vector<Tank*> current = cells[x][y];
    handleUnit(cells[x][y], cells[x][y]);

    // Also try the neighboring cells.
    if (x > 0 && y > 0) handleUnit(cells[x][y], cells[(int)x - 1][(int)y - 1]);
    //if (x > 0 && y > 0) handleUnit(cells[x][y], cells[(int)x + 1][(int)y + 1]);
    if (x < NUM_CELLS -1 && y < NUM_CELLS - 1) handleUnit(cells[x][y], cells[(int)x + 1][(int)y + 1]);
    if (x > 0) handleUnit(cells[x][y], cells[(int)x - 1][y]);
    if (x > 0) handleUnit(cells[x][y], cells[(int)x + 1][y]);
    if (y > 0) handleUnit(cells[x][y], cells[x][(int)y - 1]);
    if (y > 0) handleUnit(cells[x][y], cells[x][(int)y + 1]);
    if (x > 0 && y < NUM_CELLS - 1)
    {
        handleUnit(cells[x][y], cells[(int)x - 1][(int)y + 1]);
    }
    if (x < NUM_CELLS && y > 0) 
    {
        handleUnit(cells[x][y], cells[(int)x + 1][(int)y - 1]);
    }
}

void Grid::handleUnit(vector<Tank*> &tank_ptrs, vector<Tank*> &oTank_ptrs)
{
        for (Tank* tank : tank_ptrs)
        {
            if (tank->active)
            {
                for (Tank* o_tank : oTank_ptrs)
                {

                    if (tank == o_tank) continue;
                    vec2 dir = tank->get_position() - o_tank->get_position();
                    float dir_squared_len = dir.sqr_length();

                    float col_squared_len = (tank->get_collision_radius() + o_tank->get_collision_radius());
                    col_squared_len *= col_squared_len;

                    if (dir_squared_len < col_squared_len)
                    {
                        tank->push(dir.normalized(), 1.f);
                    }
                    
                }
            }
        }
}

void Grid::move(Tank* tank)
{
    // after every tick check position
    // See which cell it was in.
    int oldCellX = (int)(tank->old_position.x / (float)Grid::CELL_SIZE);
    int oldCellY = (int)(tank->old_position.y / (float)Grid::CELL_SIZE);

	//std::cout << tank->old_position.x << " " << tank->old_position.y << std::endl;

    // See which cell it's moving to.
    int cellX = (int)(tank->position.x / (float)Grid::CELL_SIZE);
    int cellY = (int)(tank->position.y / (float)Grid::CELL_SIZE);

	//std::cout << tank->position.x <<" "<< tank->position.y << std::endl;

    // If it didn't change cells, we're done.
    if (oldCellX == cellX && oldCellY == cellY) return;

    if (oldCellX != cellX || oldCellY != cellY)
    {
        // erase from old cell
        cells[oldCellX][oldCellY].erase(std::remove(cells[oldCellX][oldCellY].begin(), cells[oldCellX][oldCellY].end(), tank), cells[oldCellX][oldCellY].end());
        // Add it back to the grid at its new cell.
        add(tank);
    }
}

} // namespace Tmpl8