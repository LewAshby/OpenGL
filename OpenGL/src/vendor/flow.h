#pragma once
/*
This program implements a simple model based on the Cellular Automata computational paradigm that simulates the propagation of a non-inertial fluid (e.g. water or a landslide debris) over a topographic surface.

The input data is represented by two files in Ascii Grid GIS format, with a header providing information about domain dimensions (columns and rows), cell size, geographical coordinates (of the bottom left cell of the grid), and a no-data value (used to label the cells with a lack of information), and a regurlar grid of numerical values representing the information associated to the square cells (for instance topographic elevation or mass thickness), as shown below for a map of 10x10 cells:

ncols         10
nrows         10
xllcorner     2487289.5023187
yllcorner     4519797.0771414
cellsize      10.0
NODATA_value  -9999
9.00 8.00 7.00 6.00 5.00 4.00 3.00 2.00 1.00 0.00
8.00 7.00 6.00 5.00 4.00 3.00 2.00 1.00 0.00 1.00
7.00 6.00 5.00 4.00 3.00 2.00 1.00 0.00 1.00 2.00
6.00 5.00 4.00 3.00 2.00 1.00 0.00 1.00 2.00 3.00
5.00 4.00 3.00 2.00 1.00 0.00 1.00 2.00 3.00 4.00
4.00 3.00 2.00 1.00 0.00 1.00 2.00 3.00 4.00 5.00
3.00 2.00 1.00 0.00 1.00 2.00 3.00 4.00 5.00 6.00
2.00 1.00 0.00 1.00 2.00 3.00 4.00 5.00 6.00 7.00
1.00 0.00 1.00 2.00 3.00 4.00 5.00 6.00 7.00 8.00
0.00 1.00 2.00 3.00 4.00 5.00 6.00 7.00 8.00 9.00

Each cell of the square grid interacts with a small set of neighboring cells adjacent to it. In this model, the considered stencil is the von Neumann one that, besides the central cell, takes into account the adjacent cells along the North, Est, West and South directions. An index is assigned to each cell: 0 to the central one, 1 to the north neighbor, and so on, as shown below.

              |1:(-1, 0)|
    |2:( 0,-1)|0:( 0, 0)|3:( 0, 1)|
              |4:( 1, 0)|
    Figure 1. The von Neuman neighborhood stencil. Cells IDs (from 0 to 4) and relative (row,column) coordinates with respect to the central cell (having relative coordinates (0,0)) are shown.

The cell state is defined by the following information (substates):
  - topographic altitude, z;
  - mass thickness, h;
  - values of the four outflows from the central cell towards
    its neighboring cells, o[i] (i = 1,...,4).

Each substate layer is coded as a linearized matrix of type double*. In particular:
  - Sz is the layer representing the z data
  - Sh is the layer representing the h data
  - So[0] is a fictitious layer (memory is not allocated) used to let the other flow indices
          to correspond to the proper neighboring cell (since the central cell has index 0)
  - So[1] is the layer representing the amount of h that otflows towards the neighbor n. 1
  - So[2] is the layer representing the amount of h that otflows towards the neighbor n. 2
  - So[3] is the layer representing the amount of h that otflows towards the neighbor n. 3
  - So[4] is the layer representing the amount of h that otflows towards the neighbor n. 4
Two macros (get and set) are used to access grid values based on the cell's coordinate. Please, note that the number of columns must also be provided to the macros, since the linarized matrices are arrangeb by rows (and the number of columns represents the number of elements in each row).

The initial configuration of the system is defined as follows:
 - a topographic map is read from a file to initialize the Sz substate layer
 - a mass thickness map is read from a file to initialize the Sh substate layer
 - the outflow substate layers are initilized to zero

Boundary conditions are very simplyfied and consist in ignoring (i.e. not evaluating the state transition for) the cells belonging to the grid boundaries.

At each iteration step, three basic sub-steps (ake kernels, local processes, or elementary processes) are computed:

  * kernel1: flow computation
    Computes the outflows from the central cell to the neigboring cells (by applying a simplyfied algorithm knonw as the minimization algorithm of the differences). The algorithm takes in intput the total head (H=z+h) of the cells belonging to the neighborhood  and evaluates the outflows by minimizing unbalance conditions. Computed outflows are therefore stored into the outflow substate layers So[i] (i=1,...,4).

  * kernel2: mass balance
    New thickness values are evaluated by subtracting the outgoing flows from the central cell and by adding the ingoing flows coming from the adjacent neighbors. The Sh layer is updated.

  * kernel3: outflow So[i] (i=1,...,4) layers are set to zero.
   */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define STRLEN 32
#define LONGSTRLEN 256

#define VON_NEUMANN_NEIGHBORS	5

#define set(M, columns, i, j, value) ( (M)[(((i)*(columns)) + (j))] = (value) )
#define get(M, columns, i, j) ( M[(((i)*(columns)) + (j))] )


void init(int r, int c, double* M[])
{
    M[0] = NULL;
    for (int n = 1; n < VON_NEUMANN_NEIGHBORS; n++)
    {
        M[n] = (double*)malloc(sizeof(double) * r * c);

        for (int i = 0; i < r; i++)
            for (int j = 0; j < c; j++)
                set(M[n], c, i, j, 0.0);
    }
}

//-----------------------------------------------------------------------------

void initNeighborhood(Neighborhood V, int i, int j)
{
    /*
     * Evaluates the coordinates of the cells belonging to the
     * von Neumann Neigborhoods of the cell of coordinates (i,j)
     * The von Neumann's relative corrdinates are defined as:
     *
     *           |1:(-1, 0)|
     * |2:( 0,-1)|0:( 0, 0)|3:( 0, 1)|
     *           |4:( 1, 0)|
     *
     */

    V[0].i = i;   // central cell's row
    V[0].j = j;   // central cell's column
    V[1].i = i - 1; // row of the neighbor at north
    V[1].j = j;   // column of the neighbor at north
    V[2].i = i;   // row of the neighbor at east
    V[2].j = j - 1; // column of the neighbor at east
    V[3].i = i;   // row of the neighbor at west
    V[3].j = j + 1; // column of the neighbor at west
    V[4].i = i + 1; // row of the neighbor at south
    V[4].j = j;   // column of the neighbor at south
}

void kernel1_outflow_computation(int c, int i, int j, double* Sz, double* Sh, double* So[], double dumping_factor, Neighborhood& V)
{
    //Minimization algorithm: flows computation

    /*  The coordinates of the cells belonging to the neigborgood of the current (central)
        cell (with (i,j) coordinates) are evaluated based on the von Neumann stencil.
        */
    initNeighborhood(V, i, j);

    /*	Cells that are eliminated cannot receive flows from the central one.
        Note that even the central cell can be either eliminated or not eliminated.
        In the last case, a part of its mass thickness remains within the cell.
        This amount can be tougth as a residual flow.
        */
    bool eliminated[VON_NEUMANN_NEIGHBORS] = { false, false, false, false, false };

    /*	m is a quantity referred to the central cell indicating the mobile part of h,
        i.e. the amount of h that can be distributed to the nighbours
        In this example it is always m = h
        */
    double m;
    // H = z + h, except for the central cell where H = z
    double H[VON_NEUMANN_NEIGHBORS];
    // H average over the not eliminated cells
    double average;

    /*	Other variables:
        counter is the counter of not eliminated cells
        n is the for loop index
        again represents the continuation ctriterion in the do while cicle
        */
    int counter;
    bool again;

    /*	Here starts the algorithm. m is initialised to the whole lava thickness inside
        the cell and H[0] to the unmovable part of the central cell, i.e. the topographic
        altitude cell_z. The remaining elements of H are inizialised to the total heights
        of the neighbouring cells: H[i] = news[i-1]_z + news[i-1]_h.
        Note that the index i-1 is justified by the fact that news is a 4-element array,
        while H is a 5-element array.
        */
    m = get(Sh, c, i, j);
    H[0] = get(Sz, c, i, j);
    for (int n = 1; n < VON_NEUMANN_NEIGHBORS; n++)
        H[n] = get(Sz, c, V[n].i, V[n].j) + get(Sh, c, V[n].i, V[n].j);

    /*	Here is the main loop of the algorithm. It evaluates the equilibrium condition in the
        neighbourhood by computing an average height and eliminating those cells whose total height (H)
        is greater than the computed average. When no cells are eliminated during an iteration, the
        loop stops and the equilibrium condition is obtained by considering outflows computed as:
        (average - H[i]) * dumping_factor
        where dumping_factor is a relaxation rate parameter used to make the simulation more realistic.
        */
    do
    {
        again = false;
        counter = 0;
        average = m;
        for (int n = 0; n < VON_NEUMANN_NEIGHBORS; n++)
            if (eliminated[n] == false)
            {
                average += H[n];
                counter++;
            }
        average = average / counter;

        for (int n = 0; n < VON_NEUMANN_NEIGHBORS; n++)
            if ((average <= H[n]) && (eliminated[n] == false))
            {
                eliminated[n] = true;
                again = true;
            }
    } while (again);

    /* Here, outflows are evaluated and correspongid substates updated.
     * Note that So[0] is not updated deliberately. In fact, So[0] is a
     * ficticious flow introduced since the central cell belongs to
     * its own neighborhood and has index 0.
    */
    double flow;
    for (int n = 1; n < VON_NEUMANN_NEIGHBORS; n++)
        if (eliminated[n] == false)
        {
            flow = (average - H[n]) * dumping_factor;
            set(So[n], c, i, j, flow);
        }
}

void kernel2_mass_balance(int c, int i, int j, double* Sh, double* So[], Neighborhood& V)
{
    /*  The coordinates of the cells belonging to the neigborgood of the current (central)
        cell (with (x,y) coordinates) are evaluated based on the von Neumann stencil.
        */
    initNeighborhood(V, i, j);

    /* outgoing flows are subtracted from the original cell mass
     * while ingoing flows are added
     *
     *  n |  VON_NEUMANN_NEIGHBORS-n
     *  --|-------------------------
     *  0 |  5  // Overflow! It is not considered deliberately!
     *  1 |  4
     *  2 |  3
     *  3 |  2
     *  4 |  1
     *
     */
    double h = get(Sh, c, i, j);
    for (int n = 1; n < VON_NEUMANN_NEIGHBORS; n++)
    {
        h += get(So[VON_NEUMANN_NEIGHBORS - n], c, V[n].i, V[n].j);
        h -= get(So[n], c, i, j);
    }
    set(Sh, c, i, j, h);
}

void kernel3_outflow_reset(int c, int i, int j, double* So[], Neighborhood& V)
{
    for (int n = 1; n < VON_NEUMANN_NEIGHBORS; n++)
        set(So[n], c, i, j, 0.0);
}

void globalTransitionFunction(double* Sz, double* Sh, double* So[], double dumping_factor, int r, int c, Neighborhood& V, double nodata)
{
    for (int i = 1; i < r - 2; i++)
        for (int j = 1; j < r - 2; j++)
            if (get(Sz, c, i, j) != nodata)
                kernel1_outflow_computation(c, i, j, Sz, Sh, So, dumping_factor, V);

    for (int i = 1; i < r - 2; i++)
        for (int j = 1; j < r - 2; j++)
            if (get(Sz, c, i, j) != nodata)
                kernel2_mass_balance(c, i, j, Sh, So, V);

    for (int i = 1; i < r - 2; i++)
        for (int j = 1; j < r - 2; j++)
            if (get(Sz, c, i, j) != nodata)
                kernel3_outflow_reset(c, i, j, So, V);
}

//-----------------------------------------------------------------------------

