#define FLAG_SHOW_SOLUTION  0x01
#define FLAG_SHOW_STABILITY 0x02

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

bool IsCellActivated(uint2 cellNumber)
{
	uint cellNumberAll = cellNumber.y * gBoardSize + cellNumber.x; //Number of cell

	uint compressedCellGroupNumber = cellNumberAll / 32; //Element of Board that contains that cell
	uint compressedCellNumber      = cellNumberAll % 32; //Number of bit of that cell

	return (Board[compressedCellGroupNumber] >> compressedCellNumber) & 1; //Getting the bit of cell
}

bool IsCellActivatedSolution(uint2 cellNumber)
{
	uint cellNumberAll = cellNumber.y * gBoardSize + cellNumber.x; //Number of cell

	uint compressedCellGroupNumber = cellNumberAll / 32; //Element of Board that contains that cell
	uint compressedCellNumber = cellNumberAll % 32; //Number of bit of that cell

	return (Solution[compressedCellGroupNumber] >> compressedCellNumber) & 1; //Getting the bit of cell
}

bool IsCellActivatedStability(uint2 cellNumber)
{
	uint cellNumberAll = cellNumber.y * gBoardSize + cellNumber.x; //Number of cell

	uint compressedCellGroupNumber = cellNumberAll / 32; //Element of Board that contains that cell
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

		float2 cellCoord   = DTid.xy - cellNumber * (float2)gCellSize.xx - (float2)gCellSize.xx / 2;
		bool insideDiamond = (abs(cellCoord.x) + abs(cellCoord.y) <= (gCellSize - 1) / 2);

		bool leftPartColored        = cellNumber.x > 0                                               && IsCellActivated(cellNumber + int2(-1,  0));
		bool rightPartColored       = cellNumber.x < gBoardSize - 1                                  && IsCellActivated(cellNumber + int2( 1,  0));
		bool topPartColored         =                                  cellNumber.y > 0              && IsCellActivated(cellNumber + int2( 0, -1));
		bool bottomPartColored      =                                  cellNumber.y < gBoardSize - 1 && IsCellActivated(cellNumber + int2( 0,  1));
		bool leftTopPartColored     = cellNumber.x > 0              && cellNumber.y > 0              && IsCellActivated(cellNumber + int2(-1, -1));
		bool rightTopPartColored    = cellNumber.x < gBoardSize - 1 && cellNumber.y > 0              && IsCellActivated(cellNumber + int2( 1, -1));
		bool leftBottomPartColored  = cellNumber.y < gBoardSize - 1 && cellNumber.x > 0              && IsCellActivated(cellNumber + int2(-1,  1));
		bool rightBottomPartColored = cellNumber.x < gBoardSize - 1 && cellNumber.y < gBoardSize - 1 && IsCellActivated(cellNumber + int2( 1,  1));

		bool diamondCornerColored      = (leftTopPartColored                   && cellCoord.x <= 0 && cellCoord.y <= 0) || (rightTopPartColored &&                   cellCoord.x >= 0 && cellCoord.y <= 0) || (rightBottomPartColored &&                      cellCoord.x >= 0 && cellCoord.y >= 0) || (leftBottomPartColored &&                      cellCoord.x <= 0 && cellCoord.y >= 0);
		bool diamondEmptyCornerColored = (leftPartColored    && topPartColored && cellCoord.x <= 0 && cellCoord.y <= 0) || (rightPartColored    && topPartColored && cellCoord.x >= 0 && cellCoord.y <= 0) || (rightPartColored       && bottomPartColored && cellCoord.x >= 0 && cellCoord.y >= 0) || (leftPartColored       && bottomPartColored && cellCoord.x <= 0 && cellCoord.y >= 0);

		[flatten]
		if((cellEnabled && (insideDiamond || diamondCornerColored || diamondEmptyCornerColored)) || (!cellEnabled && !insideDiamond && diamondEmptyCornerColored))
		{
			result = gColorEnabled;
		}
		else
		{
			result = gColorNone;
		}

		[flatten]
		if(gFlags & FLAG_SHOW_SOLUTION) //We are showing the solution
		{
			bool cellSolved = IsCellActivatedSolution(cellNumber);

			bool leftPartSolved        = cellNumber.x > 0                                               && IsCellActivatedSolution(cellNumber + int2(-1,  0));
			bool rightPartSolved       = cellNumber.x < gBoardSize - 1                                  && IsCellActivatedSolution(cellNumber + int2( 1,  0));
			bool topPartSolved         =                                  cellNumber.y > 0              && IsCellActivatedSolution(cellNumber + int2( 0, -1));
			bool bottomPartSolved      =                                  cellNumber.y < gBoardSize - 1 && IsCellActivatedSolution(cellNumber + int2( 0,  1));
			bool leftTopPartSolved     = cellNumber.x > 0              && cellNumber.y > 0              && IsCellActivatedSolution(cellNumber + int2(-1, -1));
			bool rightTopPartSolved    = cellNumber.x < gBoardSize - 1 && cellNumber.y > 0              && IsCellActivatedSolution(cellNumber + int2( 1, -1));
			bool leftBottomPartSolved  = cellNumber.y < gBoardSize - 1 && cellNumber.x > 0              && IsCellActivatedSolution(cellNumber + int2(-1,  1));
			bool rightBottomPartSolved = cellNumber.x < gBoardSize - 1 && cellNumber.y < gBoardSize - 1 && IsCellActivatedSolution(cellNumber + int2( 1,  1));

			bool diamondCornerSolved      = (leftTopPartSolved                   && cellCoord.x <= 0 && cellCoord.y <= 0) || (rightTopPartSolved &&                   cellCoord.x >= 0 && cellCoord.y <= 0) || (rightBottomPartSolved &&                      cellCoord.x >= 0 && cellCoord.y >= 0) || (leftBottomPartSolved &&                      cellCoord.x <= 0 && cellCoord.y >= 0);
			bool diamondEmptyCornerSolved = (leftPartSolved    && topPartSolved  && cellCoord.x <= 0 && cellCoord.y <= 0) || (rightPartSolved    && topPartSolved  && cellCoord.x >= 0 && cellCoord.y <= 0) || (rightPartSolved       && bottomPartSolved  && cellCoord.x >= 0 && cellCoord.y >= 0) || (leftPartSolved       && bottomPartSolved  && cellCoord.x <= 0 && cellCoord.y >= 0);

			if((cellSolved && (insideDiamond || diamondCornerSolved || diamondEmptyCornerSolved)) || (!cellSolved && !insideDiamond && diamondEmptyCornerSolved))
			{
				result = gColorSolved;
			}
		}
		else if (gFlags & FLAG_SHOW_STABILITY)
		{
			bool cellStable = IsCellActivatedStability(cellNumber);

			bool leftPartStable        = cellNumber.x > 0                                               && IsCellActivatedStability(cellNumber + int2(-1,  0));
			bool rightPartStable       = cellNumber.x < gBoardSize - 1                                  && IsCellActivatedStability(cellNumber + int2( 1,  0));
			bool topPartStable         =                                  cellNumber.y > 0              && IsCellActivatedStability(cellNumber + int2( 0, -1));
			bool bottomPartStable      =                                  cellNumber.y < gBoardSize - 1 && IsCellActivatedStability(cellNumber + int2( 0,  1));
			bool leftTopPartStable     = cellNumber.x > 0              && cellNumber.y > 0              && IsCellActivatedStability(cellNumber + int2(-1, -1));
			bool rightTopPartStable    = cellNumber.x < gBoardSize - 1 && cellNumber.y > 0              && IsCellActivatedStability(cellNumber + int2( 1, -1));
			bool leftBottomPartStable  = cellNumber.y < gBoardSize - 1 && cellNumber.x > 0              && IsCellActivatedStability(cellNumber + int2(-1,  1));
			bool rightBottomPartStable = cellNumber.x < gBoardSize - 1 && cellNumber.y < gBoardSize - 1 && IsCellActivatedStability(cellNumber + int2( 1,  1));

			bool diamondCornerStable      = (leftTopPartStable                   && cellCoord.x <= 0 && cellCoord.y <= 0) || (rightTopPartStable &&                   cellCoord.x >= 0 && cellCoord.y <= 0) || (rightBottomPartStable &&                      cellCoord.x >= 0 && cellCoord.y >= 0) || (leftBottomPartStable &&                      cellCoord.x <= 0 && cellCoord.y >= 0);
			bool diamondEmptyCornerStable = (leftPartStable     && topPartStable && cellCoord.x <= 0 && cellCoord.y <= 0) || (rightPartStable     && topPartStable && cellCoord.x >= 0 && cellCoord.y <= 0) || (rightPartStable        && bottomPartStable && cellCoord.x >= 0 && cellCoord.y >= 0) || (leftPartStable        && bottomPartStable && cellCoord.x <= 0 && cellCoord.y >= 0);

			if((cellStable && (insideDiamond || diamondCornerStable || diamondEmptyCornerStable)) || (!cellStable && !insideDiamond && diamondEmptyCornerStable))
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