#ifndef VERTEX
#define VERTEX

// Grid class represents a vertex in a grid environment
template <typename E>
class Grid {
    public:
        E row;
        E column;
    
        // Compare grids based on rows and columns (top-left grid is lowest, bottom-right grid is highest)
        bool operator < (const Grid<E> g) const {
            if (row < g.row) {
                return true;
            } else if (row > g.row) {
                return false;
            }
            return column < g.column;
        }

        // Default constructor for a grid
        Grid() {}

        // Constructor that sets rows and columns
        Grid(E row, E col) {
            this->row = row;
            this->column = col;
        }
};

#endif