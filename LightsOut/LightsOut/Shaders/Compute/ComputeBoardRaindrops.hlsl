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

		int2 leftCell        = cellNumber + int2(-1,  0);
		int2 rightCell       = cellNumber + int2( 1,  0);
		int2 topCell         = cellNumber + int2( 0, -1);
		int2 bottomCell      = cellNumber + int2( 0,  1);
		int2 leftTopCell     = cellNumber + int2(-1, -1);
		int2 rightTopCell    = cellNumber + int2( 1, -1);
		int2 leftBottomCell  = cellNumber + int2(-1,  1);
		int2 rightBottomCell = cellNumber + int2( 1,  1);

		bool nonLeftEdge        = cellNumber.x > 0;
		bool nonRightEdge       = cellNumber.x < gBoardSize - 1;
		bool nonTopEdge         =                                  cellNumber.y > 0;
		bool nonBottomEdge      =                                  cellNumber.y < gBoardSize - 1;
		bool nonLeftTopEdge     = cellNumber.x > 0              && cellNumber.y > 0;
		bool nonRightTopEdge    = cellNumber.x < gBoardSize - 1 && cellNumber.y > 0;
		bool nonLeftBottomEdge  = cellNumber.x > 0              && cellNumber.y < gBoardSize - 1;
		bool nonRightBottomEdge = cellNumber.x < gBoardSize - 1 && cellNumber.y < gBoardSize - 1;

		if(gFlags & FLAG_TOROID_RENDER)
		{
			nonLeftEdge        = true;
			nonRightEdge       = true;
			nonTopEdge         = true;
			nonBottomEdge      = true;
			nonLeftTopEdge     = true;
			nonRightTopEdge    = true;
			nonLeftBottomEdge  = true;
			nonRightBottomEdge = true;

			const uint maxCheckDistance = 1; //Different for different render modes

			uint2 leftCellU        = (uint2)(leftCell        + gBoardSize.xx * maxCheckDistance);
			uint2 rightCellU       = (uint2)(rightCell       + gBoardSize.xx * maxCheckDistance);
			uint2 topCellU         = (uint2)(topCell         + gBoardSize.xx * maxCheckDistance);
			uint2 bottomCellU      = (uint2)(bottomCell      + gBoardSize.xx * maxCheckDistance);
			uint2 leftTopCellU     = (uint2)(leftTopCell     + gBoardSize.xx * maxCheckDistance);
			uint2 rightTopCellU    = (uint2)(rightTopCell    + gBoardSize.xx * maxCheckDistance);
			uint2 leftBottomCellU  = (uint2)(leftBottomCell  + gBoardSize.xx * maxCheckDistance);
			uint2 rightBottomCellU = (uint2)(rightBottomCell + gBoardSize.xx * maxCheckDistance);

			leftCell        = (int2)(leftCellU        % gBoardSize.xx);
			rightCell       = (int2)(rightCellU       % gBoardSize.xx);
			topCell         = (int2)(topCellU         % gBoardSize.xx);
			bottomCell      = (int2)(bottomCellU      % gBoardSize.xx);
			leftTopCell     = (int2)(leftTopCellU     % gBoardSize.xx);
			rightTopCell    = (int2)(rightTopCellU    % gBoardSize.xx);
			leftBottomCell  = (int2)(leftBottomCellU  % gBoardSize.xx);
			rightBottomCell = (int2)(rightBottomCellU % gBoardSize.xx);
		}

		bool leftPartColored        = nonLeftEdge        && IsCellActivated(leftCell);
		bool rightPartColored       = nonRightEdge       && IsCellActivated(rightCell);
		bool topPartColored         = nonTopEdge         && IsCellActivated(topCell);
		bool bottomPartColored      = nonBottomEdge      && IsCellActivated(bottomCell);
		bool leftTopPartColored     = nonLeftTopEdge     && IsCellActivated(leftTopCell);
		bool rightTopPartColored    = nonRightTopEdge    && IsCellActivated(rightTopCell);
		bool leftBottomPartColored  = nonLeftBottomEdge  && IsCellActivated(leftBottomCell);
		bool rightBottomPartColored = nonRightBottomEdge && IsCellActivated(rightBottomCell);

		bool circleEdgeColored        = (leftPartColored                      && cellCoord.x <= 0                    ) || (                       topPartColored &&                     cellCoord.y <= 0) || (rightPartColored       &&                      cellCoord.x >= 0)                     || (                         bottomPartColored &&                     cellCoord.y >= 0);
		bool circleCornerColored      = (leftTopPartColored                   && cellCoord.x <= 0 && cellCoord.y <= 0) || (rightTopPartColored &&                   cellCoord.x >= 0 && cellCoord.y <= 0) || (rightBottomPartColored &&                      cellCoord.x >= 0 && cellCoord.y >= 0) || (leftBottomPartColored &&                      cellCoord.x <= 0 && cellCoord.y >= 0);
		bool circleEmptyCornerColored = (leftPartColored    && topPartColored && cellCoord.x <= 0 && cellCoord.y <= 0) || (rightPartColored    && topPartColored && cellCoord.x >= 0 && cellCoord.y <= 0) || (rightPartColored       && bottomPartColored && cellCoord.x >= 0 && cellCoord.y >= 0) || (leftPartColored       && bottomPartColored && cellCoord.x <= 0 && cellCoord.y >= 0);

		[flatten]
		if((cellEnabled && (insideCircle || circleEdgeColored || circleCornerColored)) || (!cellEnabled && !insideCircle && circleEmptyCornerColored))
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

			bool leftPartSolved        = nonLeftEdge        && IsCellActivatedSolution(leftCell);
			bool rightPartSolved       = nonRightEdge       && IsCellActivatedSolution(rightCell);
			bool topPartSolved         = nonTopEdge         && IsCellActivatedSolution(topCell);
			bool bottomPartSolved      = nonBottomEdge      && IsCellActivatedSolution(bottomCell);
			bool leftTopPartSolved     = nonLeftTopEdge     && IsCellActivatedSolution(leftTopCell);
			bool rightTopPartSolved    = nonRightTopEdge    && IsCellActivatedSolution(rightTopCell);
			bool leftBottomPartSolved  = nonLeftBottomEdge  && IsCellActivatedSolution(leftBottomCell);
			bool rightBottomPartSolved = nonRightBottomEdge && IsCellActivatedSolution(rightBottomCell);

			bool circleEdgeSolved        = (leftPartSolved                     && cellCoord.x <= 0                    ) || (                      topPartSolved &&                     cellCoord.y <= 0) || (rightPartSolved       &&                     cellCoord.x >= 0)                     || (                        bottomPartSolved &&                     cellCoord.y >= 0);
			bool circleCornerSolved      = (leftTopPartSolved                  && cellCoord.x <= 0 && cellCoord.y <= 0) || (rightTopPartSolved &&                  cellCoord.x >= 0 && cellCoord.y <= 0) || (rightBottomPartSolved &&                     cellCoord.x >= 0 && cellCoord.y >= 0) || (leftBottomPartSolved &&                     cellCoord.x <= 0 && cellCoord.y >= 0);
			bool circleEmptyCornerSolved = (leftPartSolved    && topPartSolved && cellCoord.x <= 0 && cellCoord.y <= 0) || (rightPartSolved    && topPartSolved && cellCoord.x >= 0 && cellCoord.y <= 0) || (rightPartSolved       && bottomPartSolved && cellCoord.x >= 0 && cellCoord.y >= 0) || (leftPartSolved       && bottomPartSolved && cellCoord.x <= 0 && cellCoord.y >= 0);

			if((cellSolved && (insideCircle || circleEdgeSolved || circleCornerSolved)) || (!cellSolved && !insideCircle && circleEmptyCornerSolved))
			{
				result = gColorSolved;
			}
		}
		else if (gFlags & FLAG_SHOW_STABILITY)
		{
			bool cellStable = IsCellActivatedStability(cellNumber);

			bool leftPartStable        = nonLeftEdge        && IsCellActivatedStability(leftCell);
			bool rightPartStable       = nonRightEdge       && IsCellActivatedStability(rightCell);
			bool topPartStable         = nonTopEdge         && IsCellActivatedStability(topCell);
			bool bottomPartStable      = nonBottomEdge      && IsCellActivatedStability(bottomCell);
			bool leftTopPartStable     = nonLeftTopEdge     && IsCellActivatedStability(leftTopCell);
			bool rightTopPartStable    = nonRightTopEdge    && IsCellActivatedStability(rightTopCell);
			bool leftBottomPartStable  = nonLeftBottomEdge  && IsCellActivatedStability(leftBottomCell);
			bool rightBottomPartStable = nonRightBottomEdge && IsCellActivatedStability(rightBottomCell);

			bool circleEdgeStable        = (leftPartStable                     && cellCoord.x <= 0                    ) || (                      topPartStable &&                     cellCoord.y <= 0) || (rightPartStable       &&                     cellCoord.x >= 0)                     || (                        bottomPartStable &&                     cellCoord.y >= 0);
			bool circleCornerStable      = (leftTopPartStable                  && cellCoord.x <= 0 && cellCoord.y <= 0) || (rightTopPartStable &&                  cellCoord.x >= 0 && cellCoord.y <= 0) || (rightBottomPartStable &&                     cellCoord.x >= 0 && cellCoord.y >= 0) || (leftBottomPartStable &&                     cellCoord.x <= 0 && cellCoord.y >= 0);
			bool circleEmptyCornerStable = (leftPartStable    && topPartStable && cellCoord.x <= 0 && cellCoord.y <= 0) || (rightPartStable    && topPartStable && cellCoord.x >= 0 && cellCoord.y <= 0) || (rightPartStable       && bottomPartStable && cellCoord.x >= 0 && cellCoord.y >= 0) || (leftPartStable       && bottomPartStable && cellCoord.x <= 0 && cellCoord.y >= 0);

			if((cellStable && (insideCircle || circleEdgeStable || circleCornerStable)) || (!cellStable && !insideCircle && circleEmptyCornerStable))
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