

__kernel void kernel2(
	const int nrows,
	const int ncols,
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
		double temph = hThickness[(i * ncols) + j];
		int x;
		for (int n = 1; n < 5; n++)
		{
			x = 5 - n;
			temph += sOverflow[x * (nrows * ncols) + Vi[n] * ncols + Vj[n]];
			temph -= sOverflow[n * (nrows * ncols) + i * ncols + j];
		}
		hThickness[i * ncols + j] = temph;
	}
}