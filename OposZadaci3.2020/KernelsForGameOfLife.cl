__kernel void gameOfLife(__global unsigned char* mat,
						 const int n)
{
	int row = get_global_id(0);
	int col = get_global_id(1);

	if (row == 0 || row == n - 1 || col == 0 || col == n - 1)
	{
		return;//
	}

	int liveNeighbours = 0;
	int isAlive = !(mat[(row)*n + (col)]);

	mat[(row)*n + (col)] = 128 * isAlive;

	liveNeighbours += !(mat[(row - 1) * n + (col)]);
	liveNeighbours += !(mat[(row)*n + (col - 1)]);
	liveNeighbours += !(mat[(row)*n + (col + 1)]);
	liveNeighbours += !(mat[(row + 1) * n + (col)]);

	if ((isAlive && liveNeighbours == 2) || (liveNeighbours == 3))
	{
		mat[(row)*n + (col)] = 0;
	}
	else
	{
		mat[(row)*n + (col)] = 255;
	}
}