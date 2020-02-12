

__kernel void kernel1(
	const int nrows,
	const int ncols,
	const float dFactor,
	const int nneighbors,
	const float noDataValue,
	__global double* z,
	__global double* hThickness,
	__global double* sOverflow
)
{
	int i = get_global_id(0);
	int j = get_global_id(1);

	if (z[(i * ncols) + j] != noDataValue && i < nrows - 1 && j < ncols - 1 && i > 0 && j > 0)
	{

		//Minimization algorithm: flows computation

		/*  The coordinates of the cells belonging to the neigborgood of the current (central)
	   cell (with (i,j) coordinates) are evaluated based on the von Neumann stencil.
	   */
		int Vi[5];
		int Vj[5];
		Vi[0] = i;   // central cell's row
		Vj[0] = j;   // central cell's column
		Vi[1] = i - 1; // row of the neighbor at north
		Vj[1] = j;   // column of the neighbor at north
		Vi[2] = i;   // row of the neighbor at east
		Vj[2] = j - 1; // column of the neighbor at east
		Vi[3] = i;   // row of the neighbor at west
		Vj[3] = j + 1; // column of the neighbor at west
		Vi[4] = i + 1; // row of the neighbor at south
		Vj[4] = j;   // column of the neighbor at south


		/*	Cells that are eliminated cannot receive flows from the central one.
			Note that even the central cell can be either eliminated or not eliminated.
			In the last case, a part of its mass thickness remains within the cell.
			This amount can be tougth as a residual flow.
			*/
		bool eliminated[5] = { false, false, false, false, false };

		/*	m is a quantity referred to the central cell indicating the mobile part of h,
			i.e. the amount of h that can be distributed to the nighbours
			In this example it is always m = h
			*/
		double m;
		// H = z + h, except for the central cell where H = z
		double h[5];
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
		m = hThickness[(i * ncols) + j];
		h[0] = z[(i * ncols) + j];
		for (int n = 1; n < nneighbors; n++)
			h[n] = z[(Vi[n] * ncols) + Vj[n]] + hThickness[(Vi[n] * ncols) + Vj[n]];

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
			for (int n = 0; n < nneighbors; n++)
				if (eliminated[n] == false)
				{
					average += h[n];
					counter++;
				}
			average = average / counter;

			for (int n = 0; n < nneighbors; n++)
				if ((average <= h[n]) && (eliminated[n] == false))
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
		for (int n = 1; n < nneighbors; n++)
			if (eliminated[n] == false)
			{
				flow = (average - h[n]) * dFactor;
				sOverflow[n * (nrows * ncols) + i * ncols + j] = flow;
			}
	}
}