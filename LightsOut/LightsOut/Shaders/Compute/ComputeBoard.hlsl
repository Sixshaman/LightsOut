#define FLAG_SHOW_SOLUTION  0x01
#define FLAG_SHOW_STABILITY 0x02
#define FLAG_TOROID_RENDER  0x04

cbuffer cbParams: register(b0)
{
	uint gBoardSize;
	uint gCellSize;
	uint gUnused;
	uint gFlags;

	float4 gColorNone;
	float4 gColorEnabled;
	float4 gColorSolved;
	float4 gColorBetween;
};

Buffer<uint> Board:     register(t0); //Board in compressed uint32_t-format
Buffer<uint> Solution:  register(t1); //Solution in compressed uint32_t-format
Buffer<uint> Stability: register(t2); //Stability in compressed uint32_t-format

RWTexture2D<float4> Result: register(u0); //Drawn Board

[numthreads(16, 16, 1)]
void main(uint3 DTid: SV_DispatchThreadID)
{
	float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);

	[flatten]
	if((DTid.x % gCellSize) && (DTid.y % gCellSize)) //Inside the cell
	{
		uint2 cellNumber = DTid.xy / gCellSize.xx;

		uint cellNumberAll = cellNumber.y * gBoardSize + cellNumber.x; //Number of cell

		uint compressedCellGroupNumber = cellNumberAll / 32; //Element of Board that contains that cell
		uint compressedCellNumber      = cellNumberAll % 32; //Number of bit of that cell

		bool cellEnabled = (Board[compressedCellGroupNumber] >> compressedCellNumber) & 1; //Getting the bit of cell

		[flatten]
		if(cellEnabled)
		{
			result = gColorEnabled;
		}
		else
		{
			result = gColorNone;
		}

		[flatten]
		if(gFlags & FLAG_SHOW_SOLUTION) //Show the solution
		{
			bool cellSolved = (Solution[compressedCellGroupNumber] >> compressedCellNumber) & 1; //Getting the bit of cell

			[flatten]
			if(cellSolved)
			{
				result = gColorSolved;
			}
		}
		else if(gFlags & FLAG_SHOW_STABILITY)
		{
			bool cellStable = (Stability[compressedCellGroupNumber] >> compressedCellNumber) & 1; //Getting the bit of cell

			[flatten]
			if (cellStable)
			{
				result = float4(1.0f, 1.0f, 1.0f, 1.0f) - gColorEnabled;
			}
		}
	}
	else //Edge of the cell
	{
		result = gColorBetween;
	}

	Result[DTid.xy] = result;
}