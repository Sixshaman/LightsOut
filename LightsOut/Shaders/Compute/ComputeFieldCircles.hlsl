cbuffer cbParams: register(b0)
{
	uint gFieldSize;
	uint gCellSize;
	int  gSolutionVisible;
	int  gStabilityVisible;

	float4 gColorNone;
	float4 gColorEnabled;
	float4 gColorSolved;
	float4 gColorBetween;
};

Buffer<uint> Field:     register(t0); //Field in compressed uint32_t-format
Buffer<uint> Solution:  register(t1); //Solution in compressed uint32_t-format
Buffer<uint> Stability: register(t2); //Stability in compressed uint32_t-format

RWTexture2D<float4> Result: register(u0); //Drawn field

bool IsCellActivated(uint2 cellNumber)
{
	uint cellNumberAll = cellNumber.y * gFieldSize + cellNumber.x; //Number of cell

	uint compressedCellGroupNumber = cellNumberAll / 32; //Element of Field that contains that cell
	uint compressedCellNumber      = cellNumberAll % 32; //Number of bit of that cell

	return (Field[compressedCellGroupNumber] >> compressedCellNumber) & 1; //Getting the bit of cell
}

bool IsCellActivatedSolution(uint2 cellNumber)
{
	uint cellNumberAll = cellNumber.y * gFieldSize + cellNumber.x; //Number of cell

	uint compressedCellGroupNumber = cellNumberAll / 32; //Element of Field that contains that cell
	uint compressedCellNumber = cellNumberAll % 32; //Number of bit of that cell

	return (Solution[compressedCellGroupNumber] >> compressedCellNumber) & 1; //Getting the bit of cell
}

bool IsCellActivatedStability(uint2 cellNumber)
{
	uint cellNumberAll = cellNumber.y * gFieldSize + cellNumber.x; //Number of cell

	uint compressedCellGroupNumber = cellNumberAll / 32; //Element of Field that contains that cell
	uint compressedCellNumber      = cellNumberAll % 32; //Number of bit of that cell

	return (Stability[compressedCellGroupNumber] >> compressedCellNumber) & 1; //Getting the bit of cell
}

[numthreads(16, 16, 1)]
void main(uint3 DTid: SV_DispatchThreadID)
{
	float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);

	[flatten]
	if((DTid.x % gCellSize) && (DTid.y % gCellSize)) //Inside the cell
	{
		uint2 cellNumber = DTid.xy / gCellSize.xx;

		bool cellEnabled = IsCellActivated(cellNumber);

		float2 cellCoord    = DTid.xy - cellNumber * (float2)gCellSize.xx - (float2)gCellSize.xx / 2;
		float  circleRadius = (gCellSize - 1) / 2;

		bool insideCircle = (dot(cellCoord, cellCoord) < circleRadius * circleRadius);

		bool leftPartColored   = cellNumber.x > 0              && IsCellActivated(cellNumber + int2(-1,  0));
		bool rightPartColored  = cellNumber.x < gFieldSize - 1 && IsCellActivated(cellNumber + int2( 1,  0));
		bool topPartColored    = cellNumber.y > 0              && IsCellActivated(cellNumber + int2( 0, -1));
		bool bottomPartColored = cellNumber.y < gFieldSize - 1 && IsCellActivated(cellNumber + int2( 0,  1));

		bool circleEdgeColored = (leftPartColored && cellCoord.x <= 0) || (topPartColored && cellCoord.y <= 0) || (rightPartColored && cellCoord.x >= 0) || (bottomPartColored && cellCoord.y >= 0);

		[flatten]
		if(cellEnabled && (insideCircle || circleEdgeColored))
		{
			result = gColorEnabled;
		}
		else
		{
			result = gColorNone;
		}

		[flatten]
		if(gSolutionVisible) //We are showing the solution
		{
			bool cellSolved = IsCellActivatedSolution(cellNumber);

			bool leftPartSolved   = cellNumber.x > 0              && IsCellActivatedSolution(cellNumber + int2(-1,  0));
			bool rightPartSolved  = cellNumber.x < gFieldSize - 1 && IsCellActivatedSolution(cellNumber + int2( 1,  0));
			bool topPartSolved    = cellNumber.y > 0              && IsCellActivatedSolution(cellNumber + int2( 0, -1));
			bool bottomPartSolved = cellNumber.y < gFieldSize - 1 && IsCellActivatedSolution(cellNumber + int2( 0,  1));

			bool circleEdgeSolved = (leftPartSolved && cellCoord.x <= 0) || (topPartSolved && cellCoord.y <= 0) || (rightPartSolved && cellCoord.x >= 0) || (bottomPartSolved && cellCoord.y >= 0);

			if(cellSolved && (insideCircle || circleEdgeSolved))
			{
				result = gColorSolved;
			}
		}
		else if (gStabilityVisible)
		{
			bool cellStable = IsCellActivatedStability(cellNumber);

			bool leftPartStable   = cellNumber.x > 0              && IsCellActivatedStability(cellNumber + int2(-1,  0));
			bool rightPartStable  = cellNumber.x < gFieldSize - 1 && IsCellActivatedStability(cellNumber + int2( 1,  0));
			bool topPartStable    = cellNumber.y > 0              && IsCellActivatedStability(cellNumber + int2( 0, -1));
			bool bottomPartStable = cellNumber.y < gFieldSize - 1 && IsCellActivatedStability(cellNumber + int2( 0,  1));

			bool circleEdgeStable = (leftPartStable && cellCoord.x <= 0) || (topPartStable && cellCoord.y <= 0) || (rightPartStable && cellCoord.x >= 0) || (bottomPartStable && cellCoord.y >= 0);

			if(cellStable && (insideCircle || circleEdgeStable))
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