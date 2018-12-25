cbuffer cbParams: register(b0)
{
	uint gBoardSize;
	uint gCellSize;
	int  gSolutionVisible;
	int  gStabilityVisible;

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
	uint compressedCellNumber      = cellNumberAll % 32; //Number of bit of that cell

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

		int iBoardSize = (int)gBoardSize;
		int iCellSize  = (int)gCellSize;

		float2 cellCoord       = DTid.xy - cellNumber * (float2)gCellSize.xx - (float2)gCellSize.xx / 2;
		float  circleRadius    = (gCellSize - 1) / 2;
		float  circleRadiusBig = (gCellSize - 1);

		float2 cellCoordLeft        = cellCoord + float2( iCellSize,          0);
		float2 cellCoordRight       = cellCoord + float2(-iCellSize,          0);
		float2 cellCoordTop         = cellCoord + float2(         0,  iCellSize);
		float2 cellCoordBottom      = cellCoord + float2(         0, -iCellSize);
		float2 cellCoordLeftTop     = cellCoord + float2( iCellSize,  iCellSize);
		float2 cellCoordRightBottom = cellCoord + float2(-iCellSize, -iCellSize);
		float2 cellCoordRightTop    = cellCoord + float2(-iCellSize,  iCellSize);
		float2 cellCoordLeftBottom  = cellCoord + float2( iCellSize, -iCellSize);

		bool insideCircle      = (dot(           cellCoord,            cellCoord) <    circleRadius *    circleRadius);
		bool insideCircleBigL  = (dot(       cellCoordLeft,        cellCoordLeft) < circleRadiusBig * circleRadiusBig);
		bool insideCircleBigR  = (dot(      cellCoordRight,       cellCoordRight) < circleRadiusBig * circleRadiusBig);
		bool insideCircleBigT  = (dot(        cellCoordTop,         cellCoordTop) < circleRadiusBig * circleRadiusBig);
		bool insideCircleBigB  = (dot(     cellCoordBottom,      cellCoordBottom) < circleRadiusBig * circleRadiusBig);
		bool insideCircleBigLT = (dot(    cellCoordLeftTop,     cellCoordLeftTop) < circleRadiusBig * circleRadiusBig);
		bool insideCircleBigRB = (dot(cellCoordRightBottom, cellCoordRightBottom) < circleRadiusBig * circleRadiusBig);
		bool insideCircleBigRT = (dot(   cellCoordRightTop,    cellCoordRightTop) < circleRadiusBig * circleRadiusBig);
		bool insideCircleBigLB = (dot( cellCoordLeftBottom,  cellCoordLeftBottom) < circleRadiusBig * circleRadiusBig);

		bool leftPartColored         = cellNumber.x > 0                                               && IsCellActivated(cellNumber + int2(-1,  0));
		bool rightPartColored        = cellNumber.x < iBoardSize - 1                                  && IsCellActivated(cellNumber + int2( 1,  0));
		bool topPartColored          =                                  cellNumber.y > 0              && IsCellActivated(cellNumber + int2( 0, -1));
		bool bottomPartColored       =                                  cellNumber.y < iBoardSize - 1 && IsCellActivated(cellNumber + int2( 0,  1));
		bool leftTopPartColored      = cellNumber.x > 0              && cellNumber.y > 0              && IsCellActivated(cellNumber + int2(-1, -1));
		bool rightTopPartColored     = cellNumber.x < iBoardSize - 1 && cellNumber.y > 0              && IsCellActivated(cellNumber + int2( 1, -1));
		bool leftBottomPartColored   = cellNumber.y < iBoardSize - 1 && cellNumber.x > 0              && IsCellActivated(cellNumber + int2(-1,  1));
		bool rightBottomPartColored  = cellNumber.x < iBoardSize - 1 && cellNumber.y < iBoardSize - 1 && IsCellActivated(cellNumber + int2( 1,  1));
		bool left2PartColored        = cellNumber.x > 1                                               && IsCellActivated(cellNumber + int2(-2,  0));
		bool right2PartColored       = cellNumber.x < iBoardSize - 2                                  && IsCellActivated(cellNumber + int2( 2,  0));
		bool top2PartColored         =                                  cellNumber.y > 1              && IsCellActivated(cellNumber + int2( 0, -2));
		bool bottom2PartColored      =                                  cellNumber.y < iBoardSize - 2 && IsCellActivated(cellNumber + int2( 0,  2));
		
		bool circleEdgeColored = (leftPartColored && cellCoord.x <= 0) || (topPartColored && cellCoord.y <= 0) || (rightPartColored && cellCoord.x >= 0) || (bottomPartColored && cellCoord.y >= 0);
		
		bool circleCornerColored      = (leftTopPartColored                   && cellCoord.x <= 0 && cellCoord.y <= 0) || (rightTopPartColored &&                   cellCoord.x >= 0 && cellCoord.y <= 0) || (rightBottomPartColored &&                      cellCoord.x >= 0 && cellCoord.y >= 0) || (leftBottomPartColored &&                      cellCoord.x <= 0 && cellCoord.y >= 0);
		bool circleEmptyCornerColored = (leftPartColored    && topPartColored && cellCoord.x <= 0 && cellCoord.y <= 0) || (rightPartColored    && topPartColored && cellCoord.x >= 0 && cellCoord.y <= 0) || (rightPartColored       && bottomPartColored && cellCoord.x >= 0 && cellCoord.y >= 0) || (leftPartColored       && bottomPartColored && cellCoord.x <= 0 && cellCoord.y >= 0);
		
		bool slimEmptyCenterColored = (topPartColored && bottomPartColored && !insideCircleBigL && !insideCircleBigR) || (leftPartColored && rightPartColored && !insideCircleBigT && !insideCircleBigB);
		bool slimEdgeColored        = (left2PartColored && !insideCircleBigLT && !insideCircleBigLB && cellCoord.x <= -0.707f * gCellSize / 2) || (right2PartColored && !insideCircleBigRT && !insideCircleBigRB && cellCoord.x >= 0.707f * gCellSize / 2) || (top2PartColored && !insideCircleBigRT && !insideCircleBigLT && cellCoord.y <= -0.707f * gCellSize / 2) || (bottom2PartColored && !insideCircleBigLB && !insideCircleBigRB && cellCoord.y >= 0.707f * gCellSize / 2);

		[flatten]
		if(cellEnabled)
		{
			if(insideCircle || circleEdgeColored || circleCornerColored || slimEdgeColored)
			{
				result = gColorEnabled;
			}
			else
			{
				result = gColorNone;
			}
		}
		else
		{
			if((!insideCircle && circleEmptyCornerColored) || slimEmptyCenterColored)
			{
				result = gColorEnabled;
			}
			else
			{
				result = gColorNone;
			}
		}

		[flatten]
		if(gSolutionVisible) //We are showing the solution
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
			bool left2PartSolved       = cellNumber.x > 1                                               && IsCellActivatedSolution(cellNumber + int2(-2,  0));
			bool right2PartSolved      = cellNumber.x < iBoardSize - 2                                  && IsCellActivatedSolution(cellNumber + int2( 2,  0));
			bool top2PartSolved        =                                  cellNumber.y > 1              && IsCellActivatedSolution(cellNumber + int2( 0, -2));
			bool bottom2PartSolved     =                                  cellNumber.y < iBoardSize - 2 && IsCellActivatedSolution(cellNumber + int2( 0,  2));

			bool circleEdgeSolved        = (leftPartSolved                     && cellCoord.x <= 0                    ) || (                      topPartSolved &&                     cellCoord.y <= 0) || (rightPartSolved       &&                     cellCoord.x >= 0)                     || (                        bottomPartSolved &&                     cellCoord.y >= 0);
			bool circleCornerSolved      = (leftTopPartSolved                  && cellCoord.x <= 0 && cellCoord.y <= 0) || (rightTopPartSolved &&                  cellCoord.x >= 0 && cellCoord.y <= 0) || (rightBottomPartSolved &&                     cellCoord.x >= 0 && cellCoord.y >= 0) || (leftBottomPartSolved &&                     cellCoord.x <= 0 && cellCoord.y >= 0);
			bool circleEmptyCornerSolved = (leftPartSolved    && topPartSolved && cellCoord.x <= 0 && cellCoord.y <= 0) || (rightPartSolved    && topPartSolved && cellCoord.x >= 0 && cellCoord.y <= 0) || (rightPartSolved       && bottomPartSolved && cellCoord.x >= 0 && cellCoord.y >= 0) || (leftPartSolved       && bottomPartSolved && cellCoord.x <= 0 && cellCoord.y >= 0);

			bool slimEmptyCenterSolved = (topPartSolved && bottomPartSolved && !insideCircleBigL && !insideCircleBigR) || (leftPartSolved && rightPartSolved && !insideCircleBigT && !insideCircleBigB);
			bool slimEdgeSolved        = (top2PartSolved && !insideCircleBigLT && !insideCircleBigLB && cellCoord.x <= -0.707f * gCellSize / 2) || (right2PartSolved && !insideCircleBigRT && !insideCircleBigRB && cellCoord.x >= 0.707f * gCellSize / 2) || (top2PartSolved && !insideCircleBigLT && !insideCircleBigRT && cellCoord.y <= -0.707f * gCellSize / 2) || (bottom2PartSolved && !insideCircleBigLB && !insideCircleBigRB && cellCoord.y >= 0.707f * gCellSize / 2);

			if(cellSolved)
			{
				if(insideCircle || circleEdgeSolved || circleCornerSolved || slimEdgeSolved)
				{
					result = gColorSolved;
				}
			}
			else
			{
				if((!insideCircle && circleEmptyCornerSolved) || slimEmptyCenterSolved)
				{
					result = gColorSolved;
				}
			}
		}
		else if (gStabilityVisible)
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
			bool left2PartStable       = cellNumber.x > 1                                               && IsCellActivatedStability(cellNumber + int2(-2,  0));
			bool right2PartStable      = cellNumber.x < iBoardSize - 2                                  && IsCellActivatedStability(cellNumber + int2( 2,  0));
			bool top2PartStable        =                                  cellNumber.y > 1              && IsCellActivatedStability(cellNumber + int2( 0, -2));
			bool bottom2PartStable     =                                  cellNumber.y < iBoardSize - 2 && IsCellActivatedStability(cellNumber + int2( 0,  2));

			bool circleEdgeStable        = (leftPartStable                     && cellCoord.x <= 0                    ) || (                      topPartStable &&                     cellCoord.y <= 0) || (rightPartStable       &&                     cellCoord.x >= 0)                     || (                        bottomPartStable &&                     cellCoord.y >= 0);
			bool circleCornerStable      = (leftTopPartStable                  && cellCoord.x <= 0 && cellCoord.y <= 0) || (rightTopPartStable &&                  cellCoord.x >= 0 && cellCoord.y <= 0) || (rightBottomPartStable &&                     cellCoord.x >= 0 && cellCoord.y >= 0) || (leftBottomPartStable &&                     cellCoord.x <= 0 && cellCoord.y >= 0);
			bool circleEmptyCornerStable = (leftPartStable    && topPartStable && cellCoord.x <= 0 && cellCoord.y <= 0) || (rightPartStable    && topPartStable && cellCoord.x >= 0 && cellCoord.y <= 0) || (rightPartStable       && bottomPartStable && cellCoord.x >= 0 && cellCoord.y >= 0) || (leftPartStable       && bottomPartStable && cellCoord.x <= 0 && cellCoord.y >= 0);

			bool slimEmptyCenterStable = (topPartStable && bottomPartStable && !insideCircleBigL && !insideCircleBigR) || (leftPartStable && rightPartStable && !insideCircleBigT && !insideCircleBigB);
			bool slimEdgeStable        = (top2PartStable && !insideCircleBigLT && !insideCircleBigLB && cellCoord.x <= -0.707f * gCellSize / 2) || (right2PartStable && !insideCircleBigRT && !insideCircleBigRB && cellCoord.x >= 0.707f * gCellSize / 2) || (top2PartStable && !insideCircleBigLT && !insideCircleBigRT && cellCoord.y <= -0.707f * gCellSize / 2) || (bottom2PartStable && !insideCircleBigLB && !insideCircleBigRB && cellCoord.y >= 0.707f * gCellSize / 2);

			if(cellStable)
			{
				if(insideCircle || circleEdgeStable || circleCornerStable || slimEdgeStable)
				{
					result = float4(1.0f, 1.0f, 1.0f, 1.0f) - gColorEnabled;
				}
			}
			else
			{
				if((!insideCircle && circleEmptyCornerStable) || slimEmptyCenterStable)
				{
					result = float4(1.0f, 1.0f, 1.0f, 1.0f) - gColorEnabled;
				}
			}
		}
	}
	else //Edge of the cell
	{
		result = gColorBetween;
	}

	Result[DTid.xy] = result;
}