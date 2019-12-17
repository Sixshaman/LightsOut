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

		float2 cellCoord    = DTid.xy - cellNumber * (float2)gCellSize.xx - (float2)gCellSize.xx / 2;
		float  circleRadius = (gCellSize - 1) / 2;

		bool insideCircle = (dot(cellCoord, cellCoord) < circleRadius * circleRadius);

		int2 leftCell   = cellNumber + int2(-1,  0);
		int2 rightCell  = cellNumber + int2( 1,  0);
		int2 topCell    = cellNumber + int2( 0, -1);
		int2 bottomCell = cellNumber + int2( 0,  1);

		bool nonLeftEdge   = cellNumber.x > 0;
		bool nonRightEdge  = cellNumber.x < gBoardSize - 1;
		bool nonTopEdge    = cellNumber.y > 0;
		bool nonBottomEdge = cellNumber.y < gBoardSize - 1;

		if(gFlags & FLAG_TOROID_RENDER)
		{
			nonLeftEdge   = true;
			nonRightEdge  = true;
			nonTopEdge    = true;
			nonBottomEdge = true;

			const uint maxCheckDistance = 1; //Different for different render modes

			uint2 leftCellU   = (uint2)(leftCell   + gBoardSize.xx * maxCheckDistance);
			uint2 rightCellU  = (uint2)(rightCell  + gBoardSize.xx * maxCheckDistance);
			uint2 topCellU    = (uint2)(topCell    + gBoardSize.xx * maxCheckDistance);
			uint2 bottomCellU = (uint2)(bottomCell + gBoardSize.xx * maxCheckDistance);

			leftCell   = (int2)(leftCellU   % gBoardSize.xx);
			rightCell  = (int2)(rightCellU  % gBoardSize.xx);
			topCell    = (int2)(topCellU    % gBoardSize.xx);
			bottomCell = (int2)(bottomCellU % gBoardSize.xx);
		}

		bool leftPartColored   = nonLeftEdge   && IsCellActivated(leftCell);
		bool rightPartColored  = nonRightEdge  && IsCellActivated(rightCell);
		bool topPartColored    = nonTopEdge    && IsCellActivated(topCell);
		bool bottomPartColored = nonBottomEdge && IsCellActivated(bottomCell);

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
		if(gFlags & FLAG_SHOW_SOLUTION) //We are showing the solution
		{
			bool cellSolved = IsCellActivatedSolution(cellNumber);

			bool leftPartSolved   = nonLeftEdge   && IsCellActivatedSolution(leftCell);
			bool rightPartSolved  = nonRightEdge  && IsCellActivatedSolution(rightCell);
			bool topPartSolved    = nonTopEdge    && IsCellActivatedSolution(topCell);
			bool bottomPartSolved = nonBottomEdge && IsCellActivatedSolution(bottomCell);

			bool circleEdgeSolved = (leftPartSolved && cellCoord.x <= 0) || (topPartSolved && cellCoord.y <= 0) || (rightPartSolved && cellCoord.x >= 0) || (bottomPartSolved && cellCoord.y >= 0);

			if(cellSolved && (insideCircle || circleEdgeSolved))
			{
				result = gColorSolved;
			}
		}
		else if (gFlags & FLAG_SHOW_STABILITY)
		{
			bool cellStable = IsCellActivatedStability(cellNumber);

			bool leftPartStable   = nonLeftEdge   && IsCellActivatedStability(leftCell);
			bool rightPartStable  = nonRightEdge  && IsCellActivatedStability(rightCell);
			bool topPartStable    = nonTopEdge    && IsCellActivatedStability(topCell);
			bool bottomPartStable = nonBottomEdge && IsCellActivatedStability(bottomCell);

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