

__kernel void kernel3(
	const int nrows,
	const int ncols,
	const int nneighbors,
	const float noDataValue,
	__global double* z,
	__global double* So
)
{
	int i = get_global_id(0);
	int j = get_global_id(1);

	if (z[(i * ncols) + j] != noDataValue && i < nrows - 1 && j < ncols - 1 && i>0 && j>0)
	{
		for (int n = 1; n < nneighbors; n++)
			So[n * (nrows * ncols) + i * ncols + j] = 0.0;
	}
}