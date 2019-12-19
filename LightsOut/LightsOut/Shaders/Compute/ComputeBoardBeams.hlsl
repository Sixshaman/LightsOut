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

		float2 cellCoord          = DTid.xy - cellNumber * (float2)gCellSize.xx - (float2)gCellSize.xx / 2;
		bool insideCentralDiamond = (abs(cellCoord.x) + abs(cellCoord.y) <= (gCellSize - 1) / 2);

		bool insideHorizontalBeamLeft  = (abs(cellCoord.y) <= 0.707f * (gCellSize - 1) / 2 && cellCoord.x <= 0);
		bool insideHorizontalBeamRight = (abs(cellCoord.y) <= 0.707f * (gCellSize - 1) / 2 && cellCoord.x >= 0);
		bool insideVerticalBeamTop     = (abs(cellCoord.x) <= 0.707f * (gCellSize - 1) / 2 && cellCoord.y <= 0);
		bool insideVerticalBeamBottom  = (abs(cellCoord.x) <= 0.707f * (gCellSize - 1) / 2 && cellCoord.y >= 0);

		bool insideCentralOctagon = insideCentralDiamond && (insideHorizontalBeamLeft || insideHorizontalBeamRight) && (insideVerticalBeamTop || insideVerticalBeamBottom);

		bool insideTopLeftBeamCorner     = insideVerticalBeamTop    && insideHorizontalBeamLeft  && !insideCentralDiamond;
		bool insideTopRightBeamCorner    = insideVerticalBeamTop    && insideHorizontalBeamRight && !insideCentralDiamond;
		bool insideBottomRightBeamCorner = insideVerticalBeamBottom && insideHorizontalBeamRight && !insideCentralDiamond;
		bool insideBottomLeftBeamCorner  = insideVerticalBeamBottom && insideHorizontalBeamLeft  && !insideCentralDiamond;

		bool insideTopBeamEnd    = insideVerticalBeamTop     && !insideHorizontalBeamLeft  && !insideHorizontalBeamRight && insideCentralDiamond;
		bool insideBottomBeamEnd = insideVerticalBeamBottom  && !insideHorizontalBeamRight && !insideHorizontalBeamLeft  && insideCentralDiamond;
		bool insideLeftBeamEnd   = insideHorizontalBeamLeft  && !insideVerticalBeamTop     && !insideVerticalBeamBottom  && insideCentralDiamond;
		bool insideRightBeamEnd  = insideHorizontalBeamRight && !insideVerticalBeamBottom  && !insideVerticalBeamTop     && insideCentralDiamond;

		bool insideTopLeftEdge     = insideVerticalBeamTop     && !insideHorizontalBeamLeft  && !insideCentralDiamond && cellCoord.x <= 0;
		bool insideTopRightEdge    = insideVerticalBeamTop     && !insideHorizontalBeamRight && !insideCentralDiamond && cellCoord.x >= 0;
		bool insideRightTopEdge    = insideHorizontalBeamRight && !insideVerticalBeamTop     && !insideCentralDiamond && cellCoord.y <= 0;
		bool insideRightBottomEdge = insideHorizontalBeamRight && !insideVerticalBeamBottom  && !insideCentralDiamond && cellCoord.y >= 0;
		bool insideBottomRightEdge = insideVerticalBeamBottom  && !insideHorizontalBeamRight && !insideCentralDiamond && cellCoord.x >= 0;
		bool insideBottomLeftEdge  = insideVerticalBeamBottom  && !insideHorizontalBeamLeft  && !insideCentralDiamond && cellCoord.x <= 0;
		bool insideLeftBottomEdge  = insideHorizontalBeamLeft  && !insideVerticalBeamBottom  && !insideCentralDiamond && cellCoord.y >= 0;
		bool insideLeftTopEdge     = insideHorizontalBeamLeft  && !insideVerticalBeamTop     && !insideCentralDiamond && cellCoord.y <= 0;

		bool insideTopLeftCorner     = !insideVerticalBeamTop    && !insideHorizontalBeamLeft  && cellCoord.x <= 0 && cellCoord.y <= 0;
		bool insideTopRightCorner    = !insideVerticalBeamTop    && !insideHorizontalBeamRight && cellCoord.x >= 0 && cellCoord.y <= 0;
		bool insideBottomRightCorner = !insideVerticalBeamBottom && !insideHorizontalBeamRight && cellCoord.x >= 0 && cellCoord.y >= 0;
		bool insideBottomLeftCorner  = !insideVerticalBeamBottom && !insideHorizontalBeamLeft  && cellCoord.x <= 0 && cellCoord.y >= 0;

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

		bool surroundUncolored = !leftPartColored && !rightPartColored && !topPartColored && !bottomPartColored && !leftTopPartColored && !rightTopPartColored && !leftBottomPartColored && !rightBottomPartColored;

		bool beamEmptyCornerColored = !insideCentralDiamond && ((leftPartColored && topPartColored && !leftTopPartColored && cellCoord.x <= 0 && cellCoord.y <= 0) || (rightPartColored && topPartColored && !rightTopPartColored && cellCoord.x >= 0 && cellCoord.y <= 0) || (rightPartColored && bottomPartColored && !rightBottomPartColored && cellCoord.x >= 0 && cellCoord.y >= 0) || (leftPartColored && bottomPartColored && !leftBottomPartColored && cellCoord.x <= 0 && cellCoord.y >= 0));
		
		bool beamCornerColored = (insideTopLeftBeamCorner && (topPartColored || leftPartColored || leftTopPartColored || (bottomPartColored && rightPartColored) || (rightBottomPartColored && !leftBottomPartColored && !bottomPartColored && !rightPartColored && !rightTopPartColored))) || (insideTopRightBeamCorner && (topPartColored || rightPartColored || rightTopPartColored || (bottomPartColored && leftPartColored) || (leftBottomPartColored && !rightBottomPartColored && !bottomPartColored && !leftPartColored && !leftTopPartColored))) || (insideBottomRightBeamCorner && (bottomPartColored || rightPartColored || rightBottomPartColored || (topPartColored && leftPartColored) || (leftTopPartColored && !rightTopPartColored && !topPartColored && !leftPartColored && !leftBottomPartColored))) || (insideBottomLeftBeamCorner && (bottomPartColored || leftPartColored || leftBottomPartColored || (topPartColored && rightPartColored) || (rightTopPartColored && !leftTopPartColored && !topPartColored && !rightPartColored && !rightBottomPartColored)));
		bool beamEndColored    = (insideTopBeamEnd && (topPartColored || (leftTopPartColored && !leftPartColored) || (rightTopPartColored && !rightPartColored) || (leftBottomPartColored && rightBottomPartColored && !leftPartColored && !rightPartColored) || (bottomPartColored && !leftPartColored && !rightPartColored) || surroundUncolored)) || (insideRightBeamEnd && (rightPartColored || (rightTopPartColored && !topPartColored) || (rightBottomPartColored && !bottomPartColored) || (leftTopPartColored && leftBottomPartColored && !topPartColored && !bottomPartColored) || (leftPartColored && !topPartColored && !bottomPartColored) || surroundUncolored)) || (insideBottomBeamEnd && (bottomPartColored || (rightBottomPartColored && !rightPartColored) || (leftBottomPartColored && !leftPartColored) || (rightTopPartColored && leftTopPartColored && !rightPartColored && !leftPartColored) || (topPartColored && !rightPartColored && !leftPartColored) || surroundUncolored)) || (insideLeftBeamEnd && (leftPartColored || (leftBottomPartColored && !bottomPartColored) || (leftTopPartColored && !topPartColored) || (rightBottomPartColored && rightTopPartColored && !bottomPartColored && !topPartColored) || (rightPartColored && !bottomPartColored && !topPartColored) || surroundUncolored));
		bool cellEdgeColored   = (insideTopLeftEdge && (topPartColored || (leftTopPartColored && !leftPartColored))) || (insideTopRightEdge && (topPartColored || (rightTopPartColored && !rightPartColored))) || (insideRightTopEdge && (rightPartColored || (rightTopPartColored && !topPartColored))) || (insideRightBottomEdge && (rightPartColored || (rightBottomPartColored && !bottomPartColored))) || (insideBottomRightEdge && (bottomPartColored || (rightBottomPartColored && !rightPartColored))) || (insideBottomLeftEdge && (bottomPartColored || (leftBottomPartColored && !leftPartColored))) || (insideLeftBottomEdge && (leftPartColored || (leftBottomPartColored && !bottomPartColored))) || (insideLeftTopEdge && (leftPartColored || (leftTopPartColored && !topPartColored)));
		bool cellCornerColored = (insideTopLeftCorner && (leftTopPartColored && !leftPartColored && !topPartColored)) || (insideTopRightCorner && (rightTopPartColored && !rightPartColored && !topPartColored)) || (insideBottomRightCorner && (rightBottomPartColored && !rightPartColored && !bottomPartColored)) || (insideBottomLeftCorner && (leftBottomPartColored && !leftPartColored && !bottomPartColored));

		[flatten]
		if((cellEnabled  && (insideCentralOctagon || beamCornerColored || beamEndColored || cellEdgeColored || cellCornerColored)) || (!cellEnabled && beamEmptyCornerColored))
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

			bool surroundUnsolved = !leftPartSolved && !rightPartSolved && !topPartSolved && !bottomPartSolved && !leftTopPartSolved && !rightTopPartSolved && !leftBottomPartSolved && !rightBottomPartSolved;

			bool beamEmptyCornerSolved = !insideCentralDiamond && ((leftPartSolved && topPartSolved && !leftTopPartSolved && cellCoord.x <= 0 && cellCoord.y <= 0) || (rightPartSolved && topPartSolved && !rightTopPartSolved && cellCoord.x >= 0 && cellCoord.y <= 0) || (rightPartSolved && bottomPartSolved && !rightBottomPartSolved && cellCoord.x >= 0 && cellCoord.y >= 0) || (leftPartSolved && bottomPartSolved && !leftBottomPartSolved && cellCoord.x <= 0 && cellCoord.y >= 0));

			bool beamCornerSolved = (insideTopLeftBeamCorner && (topPartSolved || leftPartSolved || leftTopPartSolved || (bottomPartSolved && rightPartSolved) || (rightBottomPartSolved && !leftBottomPartSolved && !bottomPartSolved && !rightPartSolved && !rightTopPartSolved))) || (insideTopRightBeamCorner && (topPartSolved || rightPartSolved || rightTopPartSolved || (bottomPartSolved && leftPartSolved) || (leftBottomPartSolved && !rightBottomPartSolved && !bottomPartSolved && !leftPartSolved && !leftTopPartSolved))) || (insideBottomRightBeamCorner && (bottomPartSolved || rightPartSolved || rightBottomPartSolved || (topPartSolved && leftPartSolved) || (leftTopPartSolved && !rightTopPartSolved && !topPartSolved && !leftPartSolved && !leftBottomPartSolved))) || (insideBottomLeftBeamCorner && (bottomPartSolved || leftPartSolved || leftBottomPartSolved || (topPartSolved && rightPartSolved) || (rightTopPartSolved && !leftTopPartSolved && !topPartSolved && !rightPartSolved && !rightBottomPartSolved)));
			bool beamEndSolved = (insideTopBeamEnd && (topPartSolved || (leftTopPartSolved && !leftPartSolved) || (rightTopPartSolved && !rightPartSolved) || (leftBottomPartSolved && rightBottomPartSolved && !leftPartSolved && !rightPartSolved) || (bottomPartSolved && !leftPartSolved && !rightPartSolved) || surroundUnsolved)) || (insideRightBeamEnd && (rightPartSolved || (rightTopPartSolved && !topPartSolved) || (rightBottomPartSolved && !bottomPartSolved) || (leftTopPartSolved && leftBottomPartSolved && !topPartSolved && !bottomPartSolved) || (leftPartSolved && !topPartSolved && !bottomPartSolved) || surroundUnsolved)) || (insideBottomBeamEnd && (bottomPartSolved || (rightBottomPartSolved && !rightPartSolved) || (leftBottomPartSolved && !leftPartSolved) || (rightTopPartSolved && leftTopPartSolved && !rightPartSolved && !leftPartSolved) || (topPartSolved && !rightPartSolved && !leftPartSolved) || surroundUnsolved)) || (insideLeftBeamEnd && (leftPartSolved || (leftBottomPartSolved && !bottomPartSolved) || (leftTopPartSolved && !topPartSolved) || (rightBottomPartSolved && rightTopPartSolved && !bottomPartSolved && !topPartSolved) || (rightPartSolved && !bottomPartSolved && !topPartSolved) || surroundUnsolved));
			bool cellEdgeSolved = (insideTopLeftEdge && (topPartSolved || (leftTopPartSolved && !leftPartSolved))) || (insideTopRightEdge && (topPartSolved || (rightTopPartSolved && !rightPartSolved))) || (insideRightTopEdge && (rightPartSolved || (rightTopPartSolved && !topPartSolved))) || (insideRightBottomEdge && (rightPartSolved || (rightBottomPartSolved && !bottomPartSolved))) || (insideBottomRightEdge && (bottomPartSolved || (rightBottomPartSolved && !rightPartSolved))) || (insideBottomLeftEdge && (bottomPartSolved || (leftBottomPartSolved && !leftPartSolved))) || (insideLeftBottomEdge && (leftPartSolved || (leftBottomPartSolved && !bottomPartSolved))) || (insideLeftTopEdge && (leftPartSolved || (leftTopPartSolved && !topPartSolved)));
			bool cellCornerSolved = (insideTopLeftCorner && (leftTopPartSolved && !leftPartSolved && !topPartSolved)) || (insideTopRightCorner && (rightTopPartSolved && !rightPartSolved && !topPartSolved)) || (insideBottomRightCorner && (rightBottomPartSolved && !rightPartSolved && !bottomPartSolved)) || (insideBottomLeftCorner && (leftBottomPartSolved && !leftPartSolved && !bottomPartSolved));

			[flatten]
			if((cellSolved && (insideCentralOctagon || beamCornerSolved || beamEndSolved || cellEdgeSolved || cellCornerSolved)) || (!cellSolved && beamEmptyCornerSolved))
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

			bool surroundUnstable = !leftPartStable && !rightPartStable && !topPartStable && !bottomPartStable && !leftTopPartStable && !rightTopPartStable && !leftBottomPartStable && !rightBottomPartStable;

			bool beamEmptyCornerStable = !insideCentralDiamond && ((leftPartStable && topPartStable && !leftTopPartStable && cellCoord.x <= 0 && cellCoord.y <= 0) || (rightPartStable && topPartStable && !rightTopPartStable && cellCoord.x >= 0 && cellCoord.y <= 0) || (rightPartStable && bottomPartStable && !rightBottomPartStable && cellCoord.x >= 0 && cellCoord.y >= 0) || (leftPartStable && bottomPartStable && !leftBottomPartStable && cellCoord.x <= 0 && cellCoord.y >= 0));
		
			bool beamCornerStable = (insideTopLeftBeamCorner && (topPartStable || leftPartStable || leftTopPartStable || (bottomPartStable && rightPartStable) || (rightBottomPartStable && !leftBottomPartStable && !bottomPartStable && !rightPartStable && !rightTopPartStable))) || (insideTopRightBeamCorner && (topPartStable || rightPartStable || rightTopPartStable || (bottomPartStable && leftPartStable) || (leftBottomPartStable && !rightBottomPartStable && !bottomPartStable && !leftPartStable && !leftTopPartStable))) || (insideBottomRightBeamCorner && (bottomPartStable || rightPartStable || rightBottomPartStable || (topPartStable && leftPartStable) || (leftTopPartStable && !rightTopPartStable && !topPartStable && !leftPartStable && !leftBottomPartStable))) || (insideBottomLeftBeamCorner && (bottomPartStable || leftPartStable || leftBottomPartStable || (topPartStable && rightPartStable) || (rightTopPartStable && !leftTopPartStable && !topPartStable && !rightPartStable && !rightBottomPartStable)));
			bool beamEndStable    = (insideTopBeamEnd && (topPartStable || (leftTopPartStable && !leftPartStable) || (rightTopPartStable && !rightPartStable) || (leftBottomPartStable && rightBottomPartStable && !leftPartStable && !rightPartStable) || (bottomPartStable && !leftPartStable && !rightPartStable) || surroundUnstable)) || (insideRightBeamEnd && (rightPartStable || (rightTopPartStable && !topPartStable) || (rightBottomPartStable && !bottomPartStable) || (leftTopPartStable && leftBottomPartStable && !topPartStable && !bottomPartStable) || (leftPartStable && !topPartStable && !bottomPartStable) || surroundUnstable)) || (insideBottomBeamEnd && (bottomPartStable || (rightBottomPartStable && !rightPartStable) || (leftBottomPartStable && !leftPartStable) || (rightTopPartStable && leftTopPartStable && !rightPartStable && !leftPartStable) || (topPartStable && !rightPartStable && !leftPartStable) || surroundUnstable)) || (insideLeftBeamEnd && (leftPartStable || (leftBottomPartStable && !bottomPartStable) || (leftTopPartStable && !topPartStable) || (rightBottomPartStable && rightTopPartStable && !bottomPartStable && !topPartStable) || (rightPartStable && !bottomPartStable && !topPartStable) || surroundUnstable));
			bool cellEdgeStable   = (insideTopLeftEdge && (topPartStable || (leftTopPartStable && !leftPartStable))) || (insideTopRightEdge && (topPartStable || (rightTopPartStable && !rightPartStable))) || (insideRightTopEdge && (rightPartStable || (rightTopPartStable && !topPartStable))) || (insideRightBottomEdge && (rightPartStable || (rightBottomPartStable && !bottomPartStable))) || (insideBottomRightEdge && (bottomPartStable || (rightBottomPartStable && !rightPartStable))) || (insideBottomLeftEdge && (bottomPartStable || (leftBottomPartStable && !leftPartStable))) || (insideLeftBottomEdge && (leftPartStable || (leftBottomPartStable && !bottomPartStable))) || (insideLeftTopEdge && (leftPartStable || (leftTopPartStable && !topPartStable)));
			bool cellCornerStable = (insideTopLeftCorner && (leftTopPartStable && !leftPartStable && !topPartStable)) || (insideTopRightCorner && (rightTopPartStable && !rightPartStable && !topPartStable)) || (insideBottomRightCorner && (rightBottomPartStable && !rightPartStable && !bottomPartStable)) || (insideBottomLeftCorner && (leftBottomPartStable && !leftPartStable && !bottomPartStable));

			[flatten]
			if((cellStable && (insideCentralOctagon || beamCornerStable || beamEndStable || cellEdgeStable || cellCornerStable)) || (!cellStable && beamEmptyCornerStable))
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